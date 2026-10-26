// NOVAPLAY - Coloca UNO de estos en el nivel 3. Al empezar la partida elige al azar cuál de los
// AClassroomSeat del nivel es el asiento del jugador.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ClassroomSeatManager.generated.h"

class AClassroomSeat;

UCLASS()
class NOVAPLAY_API AClassroomSeatManager : public AActor
{
	GENERATED_BODY()

public:
	AClassroomSeatManager();

	// El asiento elegido en esta partida (util para Blueprint: apuntar una flecha, poner un objetivo, etc.)
	UPROPERTY(BlueprintReadOnly, Category = "Seat")
	AClassroomSeat* AssignedSeat = nullptr;

	// Elige otro asiento al azar (por si quieres re-sortear desde Blueprint)
	UFUNCTION(BlueprintCallable, Category = "Seat")
	void PickRandomSeat();

protected:
	virtual void BeginPlay() override;
};
