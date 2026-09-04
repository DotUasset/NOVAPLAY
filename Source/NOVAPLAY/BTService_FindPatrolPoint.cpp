// NOVAPLAY - BT Service: elige el siguiente punto de patrulla y lo escribe en el Blackboard

#include "BTService_FindPatrolPoint.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "TeacherCharacter.h"
#include "TeacherAIController.h"

UBTService_FindPatrolPoint::UBTService_FindPatrolPoint()
{
	NodeName = TEXT("Elegir Punto de Patrulla");

	// Solo necesita evaluar al entrar a la rama, no en cada tick
	bNotifyBecomeRelevant = true;
	Interval = 1.0f;
}

void UBTService_FindPatrolPoint::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);

	AAIController* AICon = OwnerComp.GetAIOwner();
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!AICon || !BB)
	{
		return;
	}

	ATeacherCharacter* Teacher = Cast<ATeacherCharacter>(AICon->GetPawn());
	if (!Teacher)
	{
		return;
	}

	if (AActor* NextPoint = Teacher->GetNextPatrolPoint())
	{
		// Tiene puntos asignados en el nivel: usa el siguiente en la secuencia/al azar
		BB->SetValueAsVector(ATeacherAIController::BBKey_PatrolPoint, NextPoint->GetActorLocation());
		return;
	}

	// Respaldo: no hay PatrolPoints configurados en el Blueprint del docente.
	// Elige un punto aleatorio navegable alrededor de su posicion actual para que
	// al menos se mueva en vez de quedarse quieto.
	if (UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(Teacher->GetWorld()))
	{
		FNavLocation RandomLoc;
		if (NavSys->GetRandomReachablePointInRadius(Teacher->GetActorLocation(), FallbackSearchRadius, RandomLoc))
		{
			BB->SetValueAsVector(ATeacherAIController::BBKey_PatrolPoint, RandomLoc.Location);
		}
	}
}
