#include "Puzzle_LeverCount.h"
#include "InteractableSwitch.h"

void APuzzle_LeverCount::NotifySwitchActivated(AInteractableSwitch* Switch)
{
	if (bIsSolved || !Switch || ActivatedSwitches.Contains(Switch))
	{
		return;
	}

	ActivatedSwitches.Add(Switch);
	ActivatedCount = ActivatedSwitches.Num();

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan,
			FString::Printf(TEXT("Palancas: %d/%d"), ActivatedCount, LeversRequired));
	}

	OnLeverProgress.Broadcast(ActivatedCount, LeversRequired);

	if (ActivatedCount >= LeversRequired)
	{
		OnSolved(); // dispara OnPuzzleSolved -> LevelLightingController (y/o la puerta)
	}
}
