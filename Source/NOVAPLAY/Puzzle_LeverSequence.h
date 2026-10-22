#pragma once

#include "CoreMinimal.h"
#include "PuzzleBase.h"
#include "Puzzle_LeverSequence.generated.h"

class AInteractableLever;

UCLASS()
class NOVAPLAY_API APuzzle_LeverSequence : public APuzzleBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Puzzle")
	TArray<int32> CorrectSequence;

	UFUNCTION(BlueprintCallable, Category = "Puzzle")
	void ActivateLever(int32 LeverIndex, AInteractableLever* Lever);

	// Cada AInteractableLever se anota acá en su BeginPlay
	UFUNCTION(BlueprintCallable, Category = "Puzzle")
	void RegisterLever(AInteractableLever* Lever);

	virtual void CheckSolution_Implementation() override;

private:
	TArray<int32> CurrentSequence;

	UPROPERTY()
	TArray<AInteractableLever*> RegisteredLevers;

	void ResetAllLevers();
};