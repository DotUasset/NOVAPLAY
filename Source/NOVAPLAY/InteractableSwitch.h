#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "InteractableSwitch.generated.h"

class APuzzle_LeverCount;

// Palanca de "un solo uso": al interactuar se activa y le avisa a su puzzle contador.
// A diferencia de AInteractableLever (que es parte de una secuencia ordenada), aca el
// orden no importa: solo cuenta cuantas palancas distintas se activaron.
UCLASS()
class NOVAPLAY_API AInteractableSwitch : public AActor, public IInteractable
{
	GENERATED_BODY()

public:
	AInteractableSwitch();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Switch")
	class UStaticMeshComponent* SwitchMesh;

	// Mesh opcionales para el estado "activada" (si no se asigna, anima el Blueprint con OnSwitchActivated)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Switch")
	class UStaticMesh* ActiveMesh;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Switch")
	APuzzle_LeverCount* OwningPuzzle;

	UPROPERTY(BlueprintReadOnly, Category = "Switch")
	bool bActivated = false;

	virtual void Interact_Implementation(APawn* InteractingPawn) override;

	// Para animacion / sonido / luz de la palanca en el Blueprint hijo
	UFUNCTION(BlueprintImplementableEvent, Category = "Switch")
	void OnSwitchActivated();
};
