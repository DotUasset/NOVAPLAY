// NOVAPLAY - AIController del docente: corre el Behavior Tree y detecta al jugador por vision

#include "TeacherAIController.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AIPerceptionTypes.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BlackboardData.h"
#include "NOVAPLAYCharacter.h"

const FName ATeacherAIController::BBKey_TargetActor(TEXT("TargetActor"));
const FName ATeacherAIController::BBKey_CanSeePlayer(TEXT("bCanSeePlayer"));
const FName ATeacherAIController::BBKey_LastKnownLocation(TEXT("LastKnownLocation"));
const FName ATeacherAIController::BBKey_PatrolPoint(TEXT("PatrolPoint"));

ATeacherAIController::ATeacherAIController()
{
	AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComp"));

	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = 1200.0f;            // distancia maxima a la que puede VER
	SightConfig->LoseSightRadius = 1500.0f;         // distancia a la que deja de verlo (evita parpadeo)

	// El cuerpo del docente solo gira mientras camina (para que el giro sea suave, ver
	// TeacherCharacter). Cuando esta parado esperando en un punto de patrulla, su cono de
	// vision queda "congelado" mirando hacia donde venia. Un cono mas ancho compensa eso:
	// 85 grados de cada lado = ~170 de campo de vision total.
	SightConfig->PeripheralVisionAngleDegrees = 85.0f;
	SightConfig->SetMaxAge(5.0f);                   // cuanto "recuerda" haberlo visto tras perderlo
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
}
