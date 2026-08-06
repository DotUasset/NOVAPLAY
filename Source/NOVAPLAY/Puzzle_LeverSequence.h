#pragma once

#include "CoreMinimal.h"
#include "PuzzleBase.h"
#include "Puzzle_LeverSequence.generated.h"

UCLASS()
class NOVAPLAY_API APuzzle_LeverSequence : public APuzzleBase
{
	GENERATED_BODY()

public:
	// Orden correcto, ej: [2, 0, 1]
	UPROPERTY(EditAnywhere, Category = "Puzzle")
	TArray<int32> CorrectSequence;

	// Le pega cada palanca cuando la interactúan
	UFUNCTION(BlueprintCallable, Category = "Puzzle")
	void ActivateLever(int32 LeverIndex);

	virtual void CheckSolution_Implementation() override;

private:
	TArray<int32> CurrentSequence;
};