#include "Puzzle_LeverSequence.h"
#include "InteractableLever.h"
#include "NOVAPLAYDebug.h"

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

void APuzzle_LeverSequence::ActivateLever(int32 LeverIndex, AInteractableLever* Lever)
{
	if (bIsSolved)
	{
		return;
	}

	CurrentSequence.Add(LeverIndex);

	if (CurrentSequence.Num() > CorrectSequence.Num())
	{
		CurrentSequence.Reset();
		CurrentSequence.Add(LeverIndex);
	}

#if !UE_BUILD_SHIPPING
	FString Debug = TEXT("Secuencia actual: ");
	for (const int32 Val : CurrentSequence)
	{
		Debug += FString::Printf(TEXT("%d "), Val);
	}
	NOVAPLAYDebug::ScreenMessage(Debug, FColor::Cyan);
#endif

	// Si la secuencia todavia no esta completa, la palanca solo se anima como activada.
	// Si esta completa, la palanca se anima SOLO cuando era la correcta (si no, se resetea todo).
	const bool bReachedFullLength = (CurrentSequence.Num() == CorrectSequence.Num());
	if (!bReachedFullLength || CurrentSequence == CorrectSequence)
	{
		if (Lever)
		{
			Lever->OnLeverActivated();
		}
	}

	if (bReachedFullLength)
	{
		// Misma logica de validacion que usa CheckSolution (puede sobrescribirse en Blueprint)
		CheckSolution();
	}
}

void APuzzle_LeverSequence::CheckSolution_Implementation()
{
	if (CurrentSequence.Num() != CorrectSequence.Num())
	{
		return;
	}

	if (CurrentSequence != CorrectSequence)
	{
		NOVAPLAYDebug::ScreenMessage(TEXT("Secuencia incorrecta, reinicia"), FColor::Red);
		CurrentSequence.Reset();
		ResetAllLevers();
		return;
	}

	NOVAPLAYDebug::ScreenMessage(TEXT("PUZZLE RESUELTO"), FColor::Green, 5.0f);
	OnSolved();
}
