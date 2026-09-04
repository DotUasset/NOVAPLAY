// NOVAPLAY - BT Service: sincroniza la velocidad/giro del docente (patrulla vs persecucion).
// Esto es lo que faltaba: ATeacherCharacter::SetMovementState() existia pero nunca se llamaba
// desde ningun lado, por eso el docente jamas aceleraba al perseguir.

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
