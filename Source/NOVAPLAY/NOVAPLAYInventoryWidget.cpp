#include "NOVAPLAYInventoryWidget.h"

void UNOVAPLAYInventoryWidget::InitializeInventory(UNOVAPLAYInventoryComponent* InInventoryComponent)
{
	if (!InInventoryComponent)
	{
		return;
	}

	CachedInventoryComponent = InInventoryComponent;
	CachedInventoryComponent->OnInventoryChanged.AddUniqueDynamic(this, &UNOVAPLAYInventoryWidget::HandleInventoryChanged);

	// Mostramos el estado inicial por si ya había ítems agregados antes de crear el widget
	HandleInventoryChanged();
}

void UNOVAPLAYInventoryWidget::NativeDestruct()
{
	if (CachedInventoryComponent)
	{
		CachedInventoryComponent->OnInventoryChanged.RemoveDynamic(this, &UNOVAPLAYInventoryWidget::HandleInventoryChanged);
	}

	Super::NativeDestruct();
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
