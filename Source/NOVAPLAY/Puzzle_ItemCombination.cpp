#include "Puzzle_ItemCombination.h"
#include "NOVAPLAYInventoryComponent.h"

void APuzzle_ItemCombination::TryCombine(APawn* InteractingPawn)
{
	if (bIsSolved || !InteractingPawn)
	{
		return;
	}

	UNOVAPLAYInventoryComponent* Inventory = InteractingPawn->FindComponentByClass<UNOVAPLAYInventoryComponent>();

	if (!Inventory)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("Pawn sin InventoryComponent"));
		}
		return;
	}

	if (Inventory->HasItem(RequiredItemID))
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("Item correcto, resolviendo"));
		}
		OnSolved();
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("No tenes el item necesario"));
		}
	}
}