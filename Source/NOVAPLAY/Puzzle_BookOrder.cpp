#include "Puzzle_BookOrder.h"
#include "NOVAPLAYDebug.h"

void APuzzle_BookOrder::PlaceNextBook(int32 BookID)
{
	if (bIsSolved)
	{
		return;
	}

	CurrentOrder.Add(BookID);

	if (CurrentOrder.Num() > CorrectOrder.Num())
	{
		CurrentOrder.Reset();
		CurrentOrder.Add(BookID);
	}

#if !UE_BUILD_SHIPPING
	FString Debug = TEXT("Orden actual: ");
	for (const int32 Val : CurrentOrder)
	{
		Debug += FString::Printf(TEXT("%d "), Val);
	}
	NOVAPLAYDebug::ScreenMessage(Debug, FColor::Cyan);
#endif

	CheckSolution();
}

void APuzzle_BookOrder::CheckSolution_Implementation()
{
	if (CurrentOrder.Num() != CorrectOrder.Num())
	{
		return;
	}

	if (CurrentOrder != CorrectOrder)
	{
		CurrentOrder.Reset();
		return;
	}

	OnSolved();
}
