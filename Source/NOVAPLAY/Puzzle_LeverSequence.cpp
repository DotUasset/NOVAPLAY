#include "Puzzle_LeverSequence.h"

void APuzzle_LeverSequence::ActivateLever(int32 LeverIndex)
{
	if (bIsSolved)
	{
		return;
	}

	CurrentSequence.Add(LeverIndex);

	// Si ya se pasó de largo la longitud correcta, reinicia
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
		return; // todavía no completó la secuencia
	}

	for (int32 i = 0; i < CorrectSequence.Num(); i++)
	{
		if (CurrentSequence[i] != CorrectSequence[i])
		{
			CurrentSequence.Empty(); // se equivocó, reinicia
			return;
		}
	}

	OnSolved();
}