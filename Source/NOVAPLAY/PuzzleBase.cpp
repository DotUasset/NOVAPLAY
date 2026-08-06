#include "PuzzleBase.h"

APuzzleBase::APuzzleBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

void APuzzleBase::CheckSolution_Implementation()
{
	// Vacío a propósito: cada hijo define su propia condición
}

void APuzzleBase::OnSolved()
{
	if (bIsSolved)
	{
		return; // evita disparar el evento dos veces
	}

	bIsSolved = true;
	OnPuzzleSolved.Broadcast(this);
	OnSolvedBP();
}