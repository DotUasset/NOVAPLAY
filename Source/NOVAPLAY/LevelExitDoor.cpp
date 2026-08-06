#include "LevelExitDoor.h"
#include "PuzzleBase.h"
#include "Kismet/GameplayStatics.h"

ALevelExitDoor::ALevelExitDoor()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ALevelExitDoor::BeginPlay()
{
	Super::BeginPlay();

	for (APuzzleBase* Puzzle : RequiredPuzzles)
	{
		if (Puzzle)
		{
			Puzzle->OnPuzzleSolved.AddDynamic(this, &ALevelExitDoor::HandlePuzzleSolved);
		}
	}
}

void ALevelExitDoor::HandlePuzzleSolved(APuzzleBase* SolvedPuzzle)
{
	CheckAllSolved();
}

void ALevelExitDoor::CheckAllSolved()
{
	for (APuzzleBase* Puzzle : RequiredPuzzles)
	{
		if (!Puzzle || !Puzzle->bIsSolved)
		{
			return; // falta al menos uno
		}
	}

	OnDoorOpened(); // animación de la puerta en BP

	if (!NextLevelName.IsNone())
	{
		UGameplayStatics::OpenLevel(this, NextLevelName);
	}
}