#include "Puzzle_ItemCombination.h"
#include "NOVAPLAYInventoryComponent.h"

bool APuzzle_ItemCombination::TryCombine(APawn* InteractingPawn)
{
	if (bIsSolved || !InteractingPawn)
	{
		return false;
	}

	UNOVAPLAYInventoryComponent* Inventory = InteractingPawn->FindComponentByClass<UNOVAPLAYInventoryComponent>();

	if (!Inventory)
	{
		return false;
	}

	if (Inventory->HasItem(RequiredItemID))
	{
		OnSolved();
		return true;
	}

	return false;
}