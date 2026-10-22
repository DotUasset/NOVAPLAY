// NOVAPLAY - BT Task generico: limpia una key del Blackboard (equivalente al "Clear Blackboard Value" que no existe nativo en 4.27)

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BTTask_ClearBlackboardValue.generated.h"

UCLASS()
class NOVAPLAY_API UBTTask_ClearBlackboardValue : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_ClearBlackboardValue();

	// Key del blackboard a limpiar (ej: LastKnownLocation)
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector BlackboardKey;

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;

protected:
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
};
