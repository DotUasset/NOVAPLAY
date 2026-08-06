#pragma once

#include "CoreMinimal.h"
#include "PuzzleBase.h"
#include "Puzzle_BookOrder.generated.h"

UCLASS()
class NOVAPLAY_API APuzzle_BookOrder : public APuzzleBase
{
	GENERATED_BODY()

public:
	// Orden correcto de IDs, ej: [3, 1, 2, 0]
	UPROPERTY(EditAnywhere, Category = "Puzzle")
	TArray<int32> CorrectOrder;

	// El jugador va "colocando" libros en este orden al interactuar con ellos
	UFUNCTION(BlueprintCallable, Category = "Puzzle")
	void PlaceNextBook(int32 BookID);

	virtual void CheckSolution_Implementation() override;

private:
	TArray<int32> CurrentOrder;
};