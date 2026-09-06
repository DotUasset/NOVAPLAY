// NOVAPLAY - AIController del docente: corre el Behavior Tree y detecta al jugador por vision

#include "TeacherAIController.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AIPerceptionTypes.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BlackboardData.h"
#include "NOVAPLAYCharacter.h"
#include "TeacherCharacter.h"

const FName ATeacherAIController::BBKey_TargetActor(TEXT("TargetActor"));
const FName ATeacherAIController::BBKey_CanSeePlayer(TEXT("bCanSeePlayer"));
const FName ATeacherAIController::BBKey_LastKnownLocation(TEXT("LastKnownLocation"));
const FName ATeacherAIController::BBKey_PatrolPoint(TEXT("PatrolPoint"));

ATeacherAIController::ATeacherAIController()
{
	AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComp"));

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

	AIPerceptionComp->ConfigureSense(*SightConfig);
	AIPerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());
	AIPerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &ATeacherAIController::OnTargetPerceived);

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
}

void ATeacherAIController::OnTargetPerceived(AActor* Actor, FAIStimulus Stimulus)
{
	if (!Actor || !Actor->IsA(ANOVAPLAYCharacter::StaticClass()) || !GetBlackboardComponent())
	{
		return;
	}

	bool bEffectivelySensed = Stimulus.WasSuccessfullySensed();

	// ----- SIGILO: agacharse reduce el rango real de deteccion -----
	// El trace de vision de UAISense_Sight ya bloquea la deteccion si hay geometria de por medio
	// (por eso esconderse detras de un objeto ya "funciona" solo). Esto suma una capa extra:
	// si el jugador esta agachado y lejos, tampoco lo detecta aunque tenga linea de vista libre.
	if (bEffectivelySensed)
	{
		if (const ANOVAPLAYCharacter* PlayerChar = Cast<ANOVAPLAYCharacter>(Actor))
		{
			if (PlayerChar->bIsCrouched && GetPawn())
			{
				const float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Stimulus.StimulusLocation);
				const float EffectiveSightRadius = SightConfig ? (SightConfig->SightRadius * CrouchSightMultiplier) : 0.0f;

				if (Distance > EffectiveSightRadius)
				{
					bEffectivelySensed = false;
				}
			}
		}
	}

	if (bEffectivelySensed)
	{
		// Lo ve ahora mismo: lo marca como objetivo activo
		GetBlackboardComponent()->SetValueAsObject(BBKey_TargetActor, Actor);
		GetBlackboardComponent()->SetValueAsBool(BBKey_CanSeePlayer, true);
	}
	else
	{
		// Lo perdio de vista (o esta agachado y demasiado lejos): guarda la ultima posicion
		// conocida para que el BT pueda ir a investigar antes de volver a patrullar
		GetBlackboardComponent()->SetValueAsBool(BBKey_CanSeePlayer, false);
		GetBlackboardComponent()->SetValueAsVector(BBKey_LastKnownLocation, Stimulus.StimulusLocation);
	}

	// Evento de "te detecte": solo en el instante exacto en que pasa de no verlo a verlo.
	// El sonido y el Print String de debug se implementan en el Event Graph de BP_Teacher.
	if (bEffectivelySensed && !bWasSeenLastFrame)
	{
		if (ATeacherCharacter* Teacher = Cast<ATeacherCharacter>(GetPawn()))
		{
			Teacher->OnDetectedPlayer();
		}
	}
	bWasSeenLastFrame = bEffectivelySensed;
}
