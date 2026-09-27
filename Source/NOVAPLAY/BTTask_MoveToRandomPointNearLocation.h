// NOVAPLAY - BT Task: se mueve a un punto aleatorio navegable cerca de una ubicacion del
// Blackboard (tipicamente LastKnownLocation). Pensado para armar una "busqueda" mas
// convincente que un simple "Move To LastKnownLocation" y quedarse ahi parado: poniendo
// 2 o 3 de estos en secuencia (con un Wait corto entre cada uno) en la rama de Investigar
// del Behavior Tree, el docente va a mirar en varios puntos alrededor de donde vio/escucho
// al jugador antes de darse por vencido, en vez de plantarse en un solo lugar.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BTTask_MoveToRandomPointNearLocation.generated.h"

UCLASS()
class NOVAPLAY_API UBTTask_MoveToRandomPointNearLocation : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_MoveToRandomPointNearLocation();

	// Key del Blackboard (tipo Vector) alrededor de la cual buscar un punto aleatorio.
	// Normalmente: LastKnownLocation.
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector CenterLocationKey;

	// Radio (cm) alrededor de CenterLocationKey en el que se busca un punto navegable al azar.
	UPROPERTY(EditAnywhere, Category = "AI|Busqueda", meta = (ClampMin = "0.0"))
	float SearchRadius = 450.0f;

	// Que tan cerca del punto elegido alcanza para considerarlo "llegado"
	UPROPERTY(EditAnywhere, Category = "AI|Busqueda", meta = (ClampMin = "0.0"))
	float AcceptanceRadius = 60.0f;

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual FString GetStaticDescription() const override;

protected:
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
};
