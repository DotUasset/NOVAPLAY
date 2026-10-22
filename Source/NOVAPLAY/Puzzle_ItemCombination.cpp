#include "Puzzle_ItemCombination.h"
#include "GameFramework/Pawn.h"
#include "NOVAPLAY.h"
#include "NOVAPLAYDebug.h"
#include "NOVAPLAYInventoryComponent.h"

bool APuzzle_ItemCombination::TryCombine(APawn* InteractingPawn)
{
	if (bIsSolved || !InteractingPawn)
	{
		return false;
	}

	if (RequiredItemID.IsNone())
	{
		UE_LOG(LogNOVAPLAY, Warning, TEXT("%s: RequiredItemID no esta configurado."), *GetName());
		return false;
	}

	UNOVAPLAYInventoryComponent* Inventory = InteractingPawn->FindComponentByClass<UNOVAPLAYInventoryComponent>();
	if (!Inventory)
	{
		NOVAPLAYDebug::ScreenMessage(TEXT("Pawn sin InventoryComponent"), FColor::Red);
		return false;
	}

	if (!Inventory->HasItem(RequiredItemID))
	{
		NOVAPLAYDebug::ScreenMessage(TEXT("No tenes el item necesario"), FColor::Red);
		return false;
	}

	NOVAPLAYDebug::ScreenMessage(TEXT("Item correcto, resolviendo"), FColor::Green);
	OnSolved();
	return true;
}
