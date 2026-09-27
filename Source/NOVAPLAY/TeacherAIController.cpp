// NOVAPLAY - AIController del docente: corre el Behavior Tree y detecta al jugador por
// vision Y por oido, con deteccion gradual (no instantanea) en vez de un simple bool.

#include "TeacherAIController.h"
#include "Perception/AISense.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AIPerceptionTypes.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BlackboardData.h"
#include "NOVAPLAYCharacter.h"
#include "TeacherCharacter.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"

const FName ATeacherAIController::BBKey_TargetActor(TEXT("TargetActor"));
const FName ATeacherAIController::BBKey_CanSeePlayer(TEXT("bCanSeePlayer"));
const FName ATeacherAIController::BBKey_LastKnownLocation(TEXT("LastKnownLocation"));
const FName ATeacherAIController::BBKey_PatrolPoint(TEXT("PatrolPoint"));

ATeacherAIController::ATeacherAIController()
{
	// Necesitamos Tick solo para el dibujo de debug (bDrawDebugPerception). Toda la logica
	// real de deteccion corre por timer (DetectionTimerHandle), no en Tick.
	PrimaryActorTick.bCanEverTick = true;

	AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComp"));

	// ----- VISION -----
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));

	// Deteccion mas generosa y mas estable que antes: radio mas grande, "colchon" mas amplio
	// entre SightRadius y LoseSightRadius (evita que parpadee entre ver/no ver justo en el
	// borde) y mas memoria de la ultima posicion vista antes de que el docente "se olvide".
	// Ajustalos directo en el Class Defaults de BP_TeacherAIController sin recompilar.
	SightConfig->SightRadius = 3000.0f;              // distancia maxima a la que puede VER
	SightConfig->LoseSightRadius = 3800.0f;          // distancia a la que deja de verlo (evita parpadeo)

	// AutoSuccessRangeFromLastSeenLocation: si el jugador se mueve pero sigue estando cerca
	// de donde el docente lo vio por ultima vez, lo vuelve a detectar automaticamente sin
	// necesitar linea de vista directa por un instante. Esto evita que lo "pierda" apenas
	// dobla una esquina cerca del docente.
	SightConfig->AutoSuccessRangeFromLastSeenLocation = 900.0f;

	// El cuerpo del docente solo gira mientras camina (para que el giro sea suave, ver
	// TeacherCharacter). Cuando esta parado esperando en un punto de patrulla, su cono de
	// vision queda "congelado" mirando hacia donde venia. Un cono ancho compensa eso:
	// 100 grados de cada lado = ~200 de campo de vision total.
	SightConfig->PeripheralVisionAngleDegrees = 100.0f;
	SightConfig->SetMaxAge(8.0f);                    // cuanto "recuerda" haberlo visto tras perderlo
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;

	// ----- OIDO (nuevo) -----
	// Antes el docente era completamente sordo: correr o saltar pegado a el, fuera de su cono
	// de vision, nunca lo alertaba. Ahora NOVAPLAYCharacter emite ruido (EmitAINoise) al
	// correr/aterrizar y el docente lo escucha y va a investigar el origen del sonido.
	HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
	HearingConfig->HearingRange = HearingRange;             // puede escuchar a traves de paredes, atenuado, hasta aca
	HearingConfig->LoSHearingRange = LineOfSightHearingRange; // con linea de sonido libre, escucha mas lejos
	HearingConfig->bUseLoSHearing = true;
	HearingConfig->SetMaxAge(5.0f);
	HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
	HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
	HearingConfig->DetectionByAffiliation.bDetectEnemies = true;

	AIPerceptionComp->ConfigureSense(*SightConfig);
	AIPerceptionComp->ConfigureSense(*HearingConfig);
	AIPerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());
	AIPerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &ATeacherAIController::OnTargetPerceived);
	// NOTA: UAIPerceptionComponent no tiene un delegate "OnTargetPerceptionForgotten" en 4.27
	// (se agrego en una version posterior del motor). No hace falta: ForgetInvestigationAfterSeconds
	// en UpdateDetectionState ya se encarga de dar por perdido al jugador sin depender de eso.

	SetPerceptionComponent(*AIPerceptionComp);
}

void ATeacherAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (BlackboardAsset && Blackboard)
	{
		Blackboard->InitializeBlackboard(*BlackboardAsset);
	}

	if (BehaviorTreeAsset)
	{
		RunBehaviorTree(BehaviorTreeAsset);
	}

	// Reinicia todo el estado de deteccion: importante si el nivel se reinicia y este mismo
	// controller vuelve a poseer un pawn, para no arrancar con sospecha de la partida anterior.
	bRawSightThisFrame = false;
	bIsAlerted = false;
	SightSuspicionTime = 0.0f;
	TimeSinceLastEvidence = 0.0f;
	CurrentAwareness = ETeacherAwarenessState::Unaware;

	GetWorldTimerManager().SetTimer(DetectionTimerHandle, this, &ATeacherAIController::UpdateDetectionState, FMath::Max(DetectionTickInterval, 0.02f), true);
}

void ATeacherAIController::OnUnPossess()
{
	GetWorldTimerManager().ClearTimer(DetectionTimerHandle);
	Super::OnUnPossess();
}

void ATeacherAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

#if !UE_BUILD_SHIPPING
	// Ayuda visual para calibrar SightRadius/HearingRange sin adivinar. Nunca se compila en
	// builds de Shipping (no hace falta apagarlo a mano antes de empaquetar).
	if (bDrawDebugPerception && GetPawn() && GetWorld())
	{
		const FVector Origin = GetPawn()->GetActorLocation();

		FColor StateColor = FColor::Green;
		switch (CurrentAwareness)
		{
			case ETeacherAwarenessState::Suspicious: StateColor = FColor::Yellow; break;
			case ETeacherAwarenessState::Alerted:    StateColor = FColor::Red;    break;
			default: break;
		}

		if (SightConfig)
		{
			DrawDebugCone(GetWorld(), Origin, GetPawn()->GetActorForwardVector(), SightConfig->SightRadius,
				FMath::DegreesToRadians(SightConfig->PeripheralVisionAngleDegrees),
				FMath::DegreesToRadians(SightConfig->PeripheralVisionAngleDegrees),
				24, StateColor, false, -1.0f, 0, 1.5f);
		}

		if (HearingConfig)
		{
			DrawDebugSphere(GetWorld(), Origin, HearingConfig->HearingRange, 16, FColor::Cyan, false, -1.0f, 0, 1.0f);
		}
	}
#endif
}

bool ATeacherAIController::ProcessSightStimulus(const ANOVAPLAYCharacter* PlayerChar, const FAIStimulus& Stimulus)
{
	if (!Stimulus.WasSuccessfullySensed())
	{
		return false;
	}

	// ----- SIGILO: agacharse reduce el rango real de deteccion -----
	// El trace de vision de UAISense_Sight ya bloquea la deteccion si hay geometria de por
	// medio (por eso esconderse detras de un objeto ya "funciona" solo). Esto suma una capa
	// extra: si el jugador esta agachado y lejos, tampoco lo detecta aunque tenga linea de
	// vista libre.
	if (PlayerChar && PlayerChar->bIsCrouched && GetPawn())
	{
		const float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Stimulus.StimulusLocation);
		const float EffectiveSightRadius = SightConfig ? (SightConfig->SightRadius * CrouchSightMultiplier) : 0.0f;

		if (Distance > EffectiveSightRadius)
		{
			return false;
		}
	}

	return true;
}

void ATeacherAIController::ProcessHearingStimulus(AActor* Actor, const FAIStimulus& Stimulus)
{
	UBlackboardComponent* BB = GetBlackboardComponent();
	if (!BB || !Stimulus.WasSuccessfullySensed())
	{
		return;
	}

	// Si ya lo tiene confirmado (Alerted, viendolo de verdad), un ruido no aporta nada nuevo.
	if (bIsAlerted)
	{
		return;
	}

	// No lo vio, pero escucho algo: lo manda a investigar el punto exacto del ruido. Esto
	// reutiliza la MISMA key que ya usaba la rama de "perdi de vista al jugador", asi que
	// el Behavior Tree existente (Move To LastKnownLocation) no necesita cambios.
	BB->SetValueAsVector(BBKey_LastKnownLocation, Stimulus.StimulusLocation);
	TimeSinceLastEvidence = 0.0f;

	// Stimulus.Strength trae la loudness reportada por EmitAINoise, ya escalada por la
	// distancia/atenuacion del sentido de oido. Un ruido fuerte "empuja" el acumulador de
	// sospecha en vez de solo mandarlo a mirar: sprintar demasiado cerca del docente puede
	// llegar a alertarlo directamente sin necesidad de que tambien te vea.
	SightSuspicionTime = FMath::Min(SightSuspicionTime + Stimulus.Strength * NoiseSuspicionContribution, TimeToDetectSight);

	SetAwarenessState(ETeacherAwarenessState::Suspicious);

	if (ATeacherCharacter* Teacher = Cast<ATeacherCharacter>(GetPawn()))
	{
		Teacher->OnHeardNoise(Stimulus.StimulusLocation, Stimulus.Strength);
	}
}

void ATeacherAIController::OnTargetPerceived(AActor* Actor, FAIStimulus Stimulus)
{
	const ANOVAPLAYCharacter* PlayerChar = Cast<ANOVAPLAYCharacter>(Actor);
	if (!PlayerChar || !GetBlackboardComponent())
	{
		return;
	}

	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
	{
		const bool bSeenNow = ProcessSightStimulus(PlayerChar, Stimulus);
		bRawSightThisFrame = bSeenNow;

		if (bSeenNow)
		{
			LastSensedPlayer = Actor;
			LastRawSightLocation = Stimulus.StimulusLocation;
		}
		else
		{
			// Lo perdio de vista (o esta agachado y demasiado lejos): guarda la ultima
			// posicion conocida para que UpdateDetectionState lo mande a investigar.
			GetBlackboardComponent()->SetValueAsVector(BBKey_LastKnownLocation, Stimulus.StimulusLocation);
		}
	}
	else if (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
	{
		ProcessHearingStimulus(Actor, Stimulus);
	}
}

void ATeacherAIController::UpdateDetectionState()
{
	APawn* MyPawn = GetPawn();
	UBlackboardComponent* BB = GetBlackboardComponent();
	if (!MyPawn || !BB)
	{
		return;
	}

	const float DeltaTime = FMath::Max(DetectionTickInterval, 0.02f);

	if (bRawSightThisFrame && LastSensedPlayer.IsValid())
	{
		// Lo esta viendo ahora mismo: acumula sospecha (mas rapido si esta muy cerca, para
		// que sea casi imposible pasar desapercibido a corta distancia).
		float Speedup = 1.0f;
		if (SightConfig && SightConfig->SightRadius > 0.0f)
		{
			const float Distance = FVector::Dist(MyPawn->GetActorLocation(), LastRawSightLocation);
			if (Distance <= SightConfig->SightRadius * CloseRangeThresholdPct)
			{
				Speedup = FMath::Max(CloseRangeDetectionSpeedup, 0.05f);
			}
		}

		const float EffectiveTimeToDetect = FMath::Max(TimeToDetectSight * Speedup, 0.05f);
		SightSuspicionTime = FMath::Min(SightSuspicionTime + DeltaTime, EffectiveTimeToDetect);
		TimeSinceLastEvidence = 0.0f;

		if (!bIsAlerted && SightSuspicionTime >= EffectiveTimeToDetect)
		{
			// Confirmado: recien ACA pasa a Alerted/persecucion de verdad.
			bIsAlerted = true;
			BB->SetValueAsObject(BBKey_TargetActor, LastSensedPlayer.Get());
			BB->SetValueAsBool(BBKey_CanSeePlayer, true);
			BB->SetValueAsVector(BBKey_LastKnownLocation, LastRawSightLocation);
			SetAwarenessState(ETeacherAwarenessState::Alerted);

			if (ATeacherCharacter* Teacher = Cast<ATeacherCharacter>(MyPawn))
			{
				Teacher->OnDetectedPlayer();
			}
		}
		else if (bIsAlerted)
		{
			// Ya confirmado y lo sigue viendo: mantiene la ultima posicion al dia
			BB->SetValueAsVector(BBKey_LastKnownLocation, LastRawSightLocation);
		}
		else
		{
			// Todavia juntando sospecha, no lo suficiente como para confirmar
			SetAwarenessState(ETeacherAwarenessState::Suspicious);
		}
	}
	else
	{
		// No lo ve en este instante: la sospecha decae con el tiempo (un vistazo corto y
		// aislado se termina olvidando en vez de sumar para siempre).
		SightSuspicionTime = FMath::Max(SightSuspicionTime - SightSuspicionDecayRate * DeltaTime, 0.0f);

		if (bIsAlerted)
		{
			// Estaba persiguiendolo de verdad y lo perdio de vista: baja a "Suspicious"
			// (sigue investigando la ultima posicion conocida, no vuelve a Unaware de golpe).
			bIsAlerted = false;
			BB->SetValueAsBool(BBKey_CanSeePlayer, false);
			SetAwarenessState(ETeacherAwarenessState::Suspicious);
		}

		const bool bHasInvestigationLead = BB->IsVectorValueSet(BBKey_LastKnownLocation);
		if (bHasInvestigationLead)
		{
			// Sigue teniendo una pista (ultima posicion vista o ultimo ruido escuchado)
			// pero no hay novedades nuevas: cuenta cuanto hace que no pasa nada.
			TimeSinceLastEvidence += DeltaTime;

			if (TimeSinceLastEvidence >= ForgetInvestigationAfterSeconds)
			{
				// Se rindio: limpia todo el rastro y vuelve a patrullar tranquilo. Esto
				// evita que el docente quede persiguiendo un fantasma para siempre si el
				// Behavior Tree no tiene su propio Clear Blackboard Value bien puesto.
				BB->ClearValue(BBKey_LastKnownLocation);
				BB->ClearValue(BBKey_TargetActor);
				BB->SetValueAsBool(BBKey_CanSeePlayer, false);
				SightSuspicionTime = 0.0f;
				TimeSinceLastEvidence = 0.0f;
				SetAwarenessState(ETeacherAwarenessState::Unaware);
			}
		}
		else if (SightSuspicionTime <= 0.0f)
		{
			SetAwarenessState(ETeacherAwarenessState::Unaware);
		}
	}
}

void ATeacherAIController::SetAwarenessState(ETeacherAwarenessState NewState)
{
	if (CurrentAwareness == NewState)
	{
		return;
	}

	CurrentAwareness = NewState;

	if (ATeacherCharacter* Teacher = Cast<ATeacherCharacter>(GetPawn()))
	{
		Teacher->NotifyAwarenessState(NewState);
	}
}
