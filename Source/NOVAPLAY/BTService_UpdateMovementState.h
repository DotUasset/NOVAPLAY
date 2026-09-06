// NOVAPLAY - BT Service: sincroniza la velocidad del docente (patrulla vs persecucion) y
// notifica el estado de movimiento a Blueprint (ver ATeacherCharacter::NotifyMovementState).

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_UpdateMovementState.generated.h"

UCLASS()
class NOVAPLAY_API UBTService_UpdateMovementState : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_UpdateMovementState();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
