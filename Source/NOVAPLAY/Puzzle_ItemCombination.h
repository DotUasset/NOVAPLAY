#pragma once

#include "CoreMinimal.h"
#include "PuzzleBase.h"
#include "Puzzle_ItemCombination.generated.h"

UCLASS()
class NOVAPLAY_API APuzzle_ItemCombination : public APuzzleBase
{
	GENERATED_BODY()

public:
	// ItemID que hace falta tener en el inventario, ej "KEY_LIBRARY_01"
	UPROPERTY(EditAnywhere, Category = "Puzzle")
	FName RequiredItemID;

	UFUNCTION(BlueprintCallable, Category = "Puzzle")
	void TryCombine(APawn* InteractingPawn);
};