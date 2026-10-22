#include "Puzzle_LeverCount.h"
#include "InteractableSwitch.h"
#include "NOVAPLAYDebug.h"

void APuzzle_LeverCount::NotifySwitchActivated(AInteractableSwitch* Switch)
{
	if (bIsSolved || !Switch || ActivatedSwitches.Contains(Switch))
	{
		return;
	}

	ActivatedSwitches.Add(Switch);
	ActivatedCount = ActivatedSwitches.Num();

	NOVAPLAYDebug::ScreenMessage(FString::Printf(TEXT("Palancas: %d/%d"), ActivatedCount, LeversRequired), FColor::Cyan);

	OnLeverProgress.Broadcast(ActivatedCount, LeversRequired);

	if (ActivatedCount >= LeversRequired)
	{
		OnSolved(); // dispara OnPuzzleSolved -> LevelLightingController (y/o la puerta)
	}
}
