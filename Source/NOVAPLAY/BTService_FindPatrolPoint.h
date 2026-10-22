// NOVAPLAY - BT Service: elige el siguiente punto de patrulla y lo escribe en el Blackboard.
// Esto es lo que faltaba: la key "PatrolPoint" nunca se seteaba desde ningun lado, por eso
// la rama "Patrullar" (Move To: PatrolPoint) fallaba siempre y el docente no patrullaba.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_FindPatrolPoint.generated.h"

UCLASS()
class NOVAPLAY_API UBTService_FindPatrolPoint : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_FindPatrolPoint();

	// Radio (cm) para buscar un punto aleatorio navegable si el docente no tiene
	// PatrolPoints asignados en el nivel.
	UPROPERTY(EditAnywhere, Category = "AI|Patrulla")
	float FallbackSearchRadius = 1200.0f;

protected:
	// Se dispara cada vez que la rama "Patrullar" se vuelve activa (al llegar al punto anterior
	// y terminar el Wait, o al volver de perseguir/investigar). Elige el proximo punto ahi,
	// no en cada tick, para no reasignar el destino mientras el docente todavia va camino a el.
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
