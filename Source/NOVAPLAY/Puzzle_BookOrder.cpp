#include "Puzzle_BookOrder.h"

void APuzzle_BookOrder::PlaceNextBook(int32 BookID)
{
	if (bIsSolved)
	{
		return;
	}

	CurrentOrder.Add(BookID);

	if (CurrentOrder.Num() > CorrectOrder.Num())
	{
		CurrentOrder.Empty();
		CurrentOrder.Add(BookID);
	}

	if (GEngine)
	{
		FString Debug = TEXT("Orden actual: ");
		for (int32 Val : CurrentOrder)
		{
			Debug += FString::Printf(TEXT("%d "), Val);
		}
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, Debug);
	}

	CheckSolution();
}

void APuzzle_BookOrder::CheckSolution_Implementation()
{
	if (CurrentOrder.Num() != CorrectOrder.Num())
	{
		return;
	}

	for (int32 i = 0; i < CorrectOrder.Num(); i++)
	{
		if (CurrentOrder[i] != CorrectOrder[i])
		{
			CurrentOrder.Empty();
			return;
		}
	}

	OnSolved();
}