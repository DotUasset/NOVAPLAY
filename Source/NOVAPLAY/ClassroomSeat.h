// NOVAPLAY - Asiento del salon (nivel 3). Uno de todos los asientos es el "asignado" al jugador
// (lo elige AClassroomSeatManager al azar en cada partida). Si el jugador interactua con el
// asiento correcto: se sienta, la pantalla se funde a negro, suena la campana y vuelve al menu.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "ClassroomSeat.generated.h"

class USoundBase;
class UStaticMeshComponent;
class USceneComponent;

UCLASS()
class NOVAPLAY_API AClassroomSeat : public AActor, public IInteractable
{
	GENERATED_BODY()

public:
	AClassroomSeat();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Seat")
	UStaticMeshComponent* SeatMesh;

	// Donde queda el CENTRO de la capsula del jugador al sentarse (mueve este punto en el
	// editor/Blueprint hasta que la camara quede a la altura de los ojos sobre la silla).
	// La X del componente (flecha azul) es hacia donde mira el jugador sentado.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Seat")
	USceneComponent* SitPoint;

	// true si este es EL asiento del jugador en esta partida (lo setea el SeatManager)
	UPROPERTY(BlueprintReadOnly, Category = "Seat")
	bool bIsAssignedSeat = false;

	// Sonido de campana (asignalo en el Blueprint hijo o en la instancia del nivel)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ending")
	USoundBase* BellSound = nullptr;

	// Segundos que tarda la pantalla en ponerse completamente negra
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ending")
	float FadeDuration = 2.0f;

	// Segundos entre que empieza a sonar la campana y que se carga el menu.
	// Ponele lo que dure tu sonido, si no se corta al cambiar de nivel.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ending")
	float PostBellDelay = 4.0f;

	// Nombre del nivel del menu principal (el mapa, sin ruta ni extension)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ending")
	FName MenuLevelName = TEXT("MainMenu");

	// Texto para mostrar cuando el jugador intenta sentarse en un asiento que no es el suyo
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Seat")
	FText WrongSeatMessage = NSLOCTEXT("NOVAPLAY", "WrongSeat", "Este no es tu asiento");

	// Lo llama el SeatManager
	void SetAssigned(bool bAssigned);

	virtual void Interact_Implementation(APawn* InteractingPawn) override;

protected:
	// Para marcar visualmente el asiento asignado (brillo, tarjeta con nombre, etc.) en el Blueprint
	UFUNCTION(BlueprintImplementableEvent, Category = "Seat")
	void OnAssignedChanged(bool bAssigned);

	// Para mostrar un mensaje en tu HUD / sonido de "no es aqui"
	UFUNCTION(BlueprintImplementableEvent, Category = "Seat")
	void OnWrongSeat();

	// Se dispara al sentarse en el asiento correcto (animacion, sonido de silla, etc.)
	UFUNCTION(BlueprintImplementableEvent, Category = "Seat")
	void OnPlayerSat();

private:
	bool bSequenceStarted = false;
	FTimerHandle BellTimerHandle;
	FTimerHandle MenuTimerHandle;

	void PlayBell();
	void GoToMenu();
};
