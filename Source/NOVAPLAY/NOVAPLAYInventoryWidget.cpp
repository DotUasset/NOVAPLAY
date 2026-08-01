#include "NOVAPLAYInventoryWidget.h"

void UNOVAPLAYInventoryWidget::InitializeInventory(UNOVAPLAYInventoryComponent* InInventoryComponent)
{
	if (!InInventoryComponent)
	{
		return;
	}

	CachedInventoryComponent = InInventoryComponent;
	CachedInventoryComponent->OnInventoryChanged.AddDynamic(this, &UNOVAPLAYInventoryWidget::HandleInventoryChanged);

	// Mostramos el estado inicial por si ya había ítems agregados antes de crear el widget
	HandleInventoryChanged();
}

bool UNOVAPLAYInventoryWidget::UseItem(FName ItemID)
{
	if (!CachedInventoryComponent)
	{
		return false;
	}

	return CachedInventoryComponent->UseItem(ItemID);
}

void UNOVAPLAYInventoryWidget::HandleInventoryChanged()
{
	if (CachedInventoryComponent)
	{
		OnInventoryUpdated(CachedInventoryComponent->GetItems());
	}
}
