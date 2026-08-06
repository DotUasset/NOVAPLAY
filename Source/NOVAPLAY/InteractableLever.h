#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "InteractableLever.generated.h"

UCLASS()
class NOVAPLAY_API AInteractableLever : public AActor, public IInteractable
{
	GENERATED_BODY()

public:
	AInteractableLever();

	UPROPERTY(VisibleAnywhere, Category = "Lever")
	class UStaticMeshComponent* LeverMesh;

	// Se setea a mano en el editor: a qué puzzle pertenece esta palanca
	UPROPERTY(EditInstanceOnly, Category = "Lever")
	class APuzzle_LeverSequence* OwningPuzzle;

	// Posición de esta palanca dentro de la secuencia (0, 1, 2...)
	UPROPERTY(EditAnywhere, Category = "Lever")
	int32 LeverIndex = 0;

	virtual void Interact_Implementation(APawn* InteractingPawn) override;
};