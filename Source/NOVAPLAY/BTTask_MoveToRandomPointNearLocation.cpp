// NOVAPLAY - BT Task: se mueve a un punto aleatorio navegable cerca de una ubicacion del
// Blackboard, para armar una busqueda mas convincente en la rama de Investigar.

#include "BTTask_MoveToRandomPointNearLocation.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "Navigation/PathFollowingComponent.h"
#include "AIController.h"
#include "NavigationSystem.h"

UBTTask_MoveToRandomPointNearLocation::UBTTask_MoveToRandomPointNearLocation()
{
	NodeName = TEXT("Buscar Cerca de Ubicacion");

	// Necesitamos que el motor nos llame TickTask para poder chequear cuando termina el
	// movimiento (MoveToLocation es asincronico: arranca el path y devuelve enseguida).
	bNotifyTick = true;

	// Restringe el selector de key en el editor a solo keys de tipo Vector (ej. LastKnownLocation)
	CenterLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_MoveToRandomPointNearLocation, CenterLocationKey));
}

EBTNodeResult::Type UBTTask_MoveToRandomPointNearLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!AICon || !BB)
	{
		return EBTNodeResult::Failed;
	}

	const FVector Center = BB->GetValueAsVector(CenterLocationKey.SelectedKeyName);

	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(AICon->GetWorld());
	if (!NavSys)
	{
		return EBTNodeResult::Failed;
	}

	FNavLocation RandomLoc;
	if (!NavSys->GetRandomReachablePointInRadius(Center, SearchRadius, RandomLoc))
	{
		return EBTNodeResult::Failed;
	}

	const EPathFollowingRequestResult::Type MoveResult = AICon->MoveToLocation(RandomLoc.Location, AcceptanceRadius);

	if (MoveResult == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		return EBTNodeResult::Succeeded;
	}

	return (MoveResult == EPathFollowingRequestResult::RequestSuccessful) ? EBTNodeResult::InProgress : EBTNodeResult::Failed;
}

void UBTTask_MoveToRandomPointNearLocation::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// MoveToLocation es asincronico: cuando el path following vuelve a Idle, o bien llego
	// o bien el path fallo en el camino. En ambos casos terminamos la tarea aca para que la
	// secuencia de busqueda pueda seguir con el proximo punto sin quedar trabada.
	if (AICon->GetMoveStatus() == EPathFollowingStatus::Idle)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

FString UBTTask_MoveToRandomPointNearLocation::GetStaticDescription() const
{
	return FString::Printf(TEXT("Buscar cerca de %s (radio %.0f)"), *CenterLocationKey.SelectedKeyName.ToString(), SearchRadius);
}

void UBTTask_MoveToRandomPointNearLocation::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	if (UBlackboardData* BBAsset = GetBlackboardAsset())
	{
		CenterLocationKey.ResolveSelectedKey(*BBAsset);
	}
}
