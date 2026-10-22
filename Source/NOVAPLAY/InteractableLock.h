#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "InteractableLock.generated.h"

UCLASS()
class NOVAPLAY_API AInteractableLock : public AActor, public IInteractable
{
	GENERATED_BODY()

public:
	AInteractableLock();

	UPROPERTY(VisibleAnywhere, Category = "Lock")
	class UStaticMeshComponent* LockMesh;

	UPROPERTY(EditInstanceOnly, Category = "Lock")
	class APuzzle_ItemCombination* OwningPuzzle;

	virtual void Interact_Implementation(APawn* InteractingPawn) override;

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Lock")
	void OnUnlocked();
};