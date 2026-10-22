#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PuzzleBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPuzzleSolved, APuzzleBase*, SolvedPuzzle);

UCLASS(Abstract)
class NOVAPLAY_API APuzzleBase : public AActor
{
	GENERATED_BODY()

public:
	APuzzleBase();

	UPROPERTY(BlueprintReadOnly, Category = "Puzzle")
	bool bIsSolved = false;

	// Otros actores (una puerta, un trigger) se suscriben a esto sin conocer el puzzle concreto
	UPROPERTY(BlueprintAssignable, Category = "Puzzle")
	FOnPuzzleSolved OnPuzzleSolved;

	// Cada puzzle concreto sobrescribe esto (en C++ o en Blueprint)
	UFUNCTION(BlueprintNativeEvent, Category = "Puzzle")
	void CheckSolution();
	virtual void CheckSolution_Implementation();

	// Llamable desde C++ o BP cuando se cumple la condición de victoria
	UFUNCTION(BlueprintCallable, Category = "Puzzle")
	void OnSolved();

protected:
	// Para feedback visual/sonido en el Blueprint (abrir cofre, prender luz, etc.)
	UFUNCTION(BlueprintImplementableEvent, Category = "Puzzle")
	void OnSolvedBP();
};