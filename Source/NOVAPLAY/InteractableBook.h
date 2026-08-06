#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "InteractableBook.generated.h"

UCLASS()
class NOVAPLAY_API AInteractableBook : public AActor, public IInteractable
{
	GENERATED_BODY()

public:
	AInteractableBook();

	UPROPERTY(VisibleAnywhere, Category = "Book")
	class UStaticMeshComponent* BookMesh;

	UPROPERTY(EditInstanceOnly, Category = "Book")
	class APuzzle_BookOrder* OwningPuzzle;

	// Identidad de este libro (a qué "título" corresponde)
	UPROPERTY(EditAnywhere, Category = "Book")
	int32 BookID = 0;

	virtual void Interact_Implementation(APawn* InteractingPawn) override;
};