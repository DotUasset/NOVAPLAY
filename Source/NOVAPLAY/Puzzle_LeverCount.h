#pragma once

#include "CoreMinimal.h"
#include "PuzzleBase.h"
#include "Puzzle_LeverCount.generated.h"

class AInteractableSwitch;

// Se dispara cada vez que se activa una palanca (para HUD "1/3", sonido, etc.)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLeverProgress, int32, ActivatedCount, int32, RequiredCount);

// Puzzle: activar N palancas en cualquier orden. Cuando se activan todas -> OnSolved().
UCLASS()
class NOVAPLAY_API APuzzle_LeverCount : public APuzzleBase
{
	GENERATED_BODY()

public:
	// Cuantas palancas distintas hay que activar
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Puzzle")
	int32 LeversRequired = 3;

	UPROPERTY(BlueprintReadOnly, Category = "Puzzle")
	int32 ActivatedCount = 0;

	UPROPERTY(BlueprintAssignable, Category = "Puzzle")
	FOnLeverProgress OnLeverProgress;

	// La llama AInteractableSwitch al activarse
	void NotifySwitchActivated(AInteractableSwitch* Switch);

private:
	UPROPERTY()
	TArray<AInteractableSwitch*> ActivatedSwitches;
};
