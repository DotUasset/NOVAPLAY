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

void APuzzle_LeverSequence::ActivateLever(int32 LeverIndex, AInteractableLever* Lever)
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

	if (GEngine)
	{
		FString Debug = TEXT("Secuencia actual: ");
		for (int32 Val : CurrentSequence)
		{
			Debug += FString::Printf(TEXT("%d "), Val);
		}
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, Debug);
	}

	const bool bReachedFullLength = (CurrentSequence.Num() == CorrectSequence.Num());

	if (!bReachedFullLength)
	{
		if (Lever)
		{
			Lever->OnLeverActivated();
		}
		return;
	}

	bool bCorrect = true;
	for (int32 i = 0; i < CorrectSequence.Num(); i++)
	{
		if (CurrentSequence[i] != CorrectSequence[i])
		{
			bCorrect = false;
			break;
		}
	}

	if (bCorrect)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("PUZZLE RESUELTO"));
		}

		if (Lever)
		{
			Lever->OnLeverActivated();
		}
		OnSolved();
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("Secuencia incorrecta, reinicia"));
		}

		CurrentSequence.Empty();
		ResetAllLevers();
	}
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
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("Secuencia incorrecta, reinicia"));
			}
			CurrentSequence.Empty();
			ResetAllLevers();
			return;
		}
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("PUZZLE RESUELTO"));
	}

	OnSolved();
}