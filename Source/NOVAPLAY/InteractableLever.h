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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lever")
	class UStaticMeshComponent* LeverMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lever")
	class UStaticMesh* ActiveMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lever")
	class UStaticMesh* InactiveMesh;

	UPROPERTY(EditInstanceOnly, Category = "Lever")
	class APuzzle_LeverSequence* OwningPuzzle;

	UPROPERTY(EditAnywhere, Category = "Lever")
	int32 LeverIndex = 0;

	virtual void Interact_Implementation(APawn* InteractingPawn) override;

	// Público porque lo llama el puzzle, no un hijo de esta clase
	UFUNCTION(BlueprintImplementableEvent, Category = "Lever")
	void OnLeverActivated();

	UFUNCTION(BlueprintImplementableEvent, Category = "Lever")
	void OnLeverReset();

protected:
	virtual void BeginPlay() override;
};