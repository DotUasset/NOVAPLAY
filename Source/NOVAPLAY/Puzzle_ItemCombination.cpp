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
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("Pawn sin InventoryComponent"));
		}
		return false;
	}

	if (Inventory->HasItem(RequiredItemID))
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("Item correcto, resolviendo"));
		}
		OnSolved();
		return true;
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("No tenes el item necesario"));
		}
	}
	return false;
}