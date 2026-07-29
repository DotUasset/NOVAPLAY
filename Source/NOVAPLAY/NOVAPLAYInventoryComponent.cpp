// NOVAPLAYInventoryComponent.cpp
#include "NOVAPLAYInventoryComponent.h"

UNOVAPLAYInventoryComponent::UNOVAPLAYInventoryComponent()
{
	// Este componente no necesita Tick, solo reacciona a llamadas (AddItem, etc.)
	PrimaryComponentTick.bCanEverTick = false;
}

int32 UNOVAPLAYInventoryComponent::FindItemIndex(FName ItemID) const
{
	return Items.IndexOfByPredicate([ItemID](const FInventoryItem& Item)
	{
		return Item.ItemID == ItemID;
	});
}

bool UNOVAPLAYInventoryComponent::AddItem(FName ItemID, FText ItemName, UTexture2D* Icon, FText Description, int32 Amount)
{
	if (ItemID == NAME_None || Amount <= 0)
	{
		return false;
	}

	const int32 ExistingIndex = FindItemIndex(ItemID);

	if (ExistingIndex != INDEX_NONE)
	{
		// Ya existe: sumamos cantidad
		Items[ExistingIndex].Quantity += Amount;
	}
	else
	{
		// No existe: creamos una entrada nueva
		FInventoryItem NewItem;
		NewItem.ItemID = ItemID;
		NewItem.ItemName = ItemName;
		NewItem.Icon = Icon;
		NewItem.Description = Description;
		NewItem.Quantity = Amount;
		Items.Add(NewItem);
	}

	// Avisamos a quien esté escuchando (típicamente la UI) que el inventario cambió
	OnInventoryChanged.Broadcast();
	return true;
}

bool UNOVAPLAYInventoryComponent::RemoveItem(FName ItemID, int32 Amount)
{
	const int32 ExistingIndex = FindItemIndex(ItemID);

	if (ExistingIndex == INDEX_NONE || Amount <= 0)
	{
		return false;
	}

	if (Items[ExistingIndex].Quantity < Amount)
	{
		// No hay suficiente cantidad para sacar
		return false;
	}

	Items[ExistingIndex].Quantity -= Amount;

	if (Items[ExistingIndex].Quantity <= 0)
	{
		Items.RemoveAt(ExistingIndex);
	}

	OnInventoryChanged.Broadcast();
	return true;
}

bool UNOVAPLAYInventoryComponent::HasItem(FName ItemID) const
{
	return FindItemIndex(ItemID) != INDEX_NONE;
}

int32 UNOVAPLAYInventoryComponent::GetItemQuantity(FName ItemID) const
{
	const int32 ExistingIndex = FindItemIndex(ItemID);
	return ExistingIndex != INDEX_NONE ? Items[ExistingIndex].Quantity : 0;
}