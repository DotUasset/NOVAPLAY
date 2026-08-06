#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LevelExitDoor.generated.h"

class APuzzleBase;

UCLASS()
class NOVAPLAY_API ALevelExitDoor : public AActor
{
	GENERATED_BODY()

public:
	ALevelExitDoor();

	// Puzzles que hay que resolver para que se abra (podés poner uno o varios)
	UPROPERTY(EditInstanceOnly, Category = "Door")
	TArray<APuzzleBase*> RequiredPuzzles;

	UPROPERTY(EditAnywhere, Category = "Door")
	FName NextLevelName;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandlePuzzleSolved(APuzzleBase* SolvedPuzzle);

	void CheckAllSolved();

	UFUNCTION(BlueprintImplementableEvent, Category = "Door")
	void OnDoorOpened();
};