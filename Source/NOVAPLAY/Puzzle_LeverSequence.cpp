#include "Puzzle_LeverSequence.h"
#include "InteractableLever.h"

void APuzzle_LeverSequence::RegisterLever(AInteractableLever* Lever)
{
	if (Lever)
	{
		RegisteredLevers.AddUnique(Lever);
	}
}

void APuzzle_LeverSequence::ResetAllLevers()
{
	for (AInteractableLever* Lever : RegisteredLevers)
	{
		if (Lever)
		{
			Lever->OnLeverReset();
		}
	}
}

void APuzzle_LeverSequence::ActivateLever(int32 LeverIndex)
{
	if (bIsSolved)
	{
		return;
	}

	CurrentSequence.Add(LeverIndex);

	if (CurrentSequence.Num() > CorrectSequence.Num())
	{
		CurrentSequence.Empty();
		CurrentSequence.Add(LeverIndex);
	}

	CheckSolution();
}

void APuzzle_LeverSequence::CheckSolution_Implementation()
{
	if (CurrentSequence.Num() != CorrectSequence.Num())
	{
		return;
	}

	for (int32 i = 0; i < CorrectSequence.Num(); i++)
	{
		if (CurrentSequence[i] != CorrectSequence[i])
		{
			CurrentSequence.Empty();
			ResetAllLevers();
			return;
		}
	}

	OnSolved();
}