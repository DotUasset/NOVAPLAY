#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "InteractableComputer.generated.h"

UCLASS()
class NOVAPLAY_API AInteractableComputer : public AActor, public IInteractable
{
	GENERATED_BODY()

public:
	AInteractableComputer();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Computer")
	class UStaticMeshComponent* ComputerMesh;

	// Se asigna a mano en el nivel: a que puzzle de canje pertenece esta computadora
	UPROPERTY(EditInstanceOnly, Category = "Computer")
	class APuzzle_BookExchange* OwningPuzzle;

	virtual void Interact_Implementation(APawn* InteractingPawn) override;

protected:
	// El Blueprint hijo implementa el feedback (sonido, animacion de pantalla, texto, etc.)
	UFUNCTION(BlueprintImplementableEvent, Category = "Computer")
	void OnExchangeSuccess();

	UFUNCTION(BlueprintImplementableEvent, Category = "Computer")
	void OnExchangeFailed();
};
