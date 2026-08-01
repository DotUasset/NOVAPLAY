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

void UNOVAPLAYInventoryWidget::HandleInventoryChanged()
{
	if (CachedInventoryComponent)
	{
		OnInventoryUpdated(CachedInventoryComponent->GetItems());
	}
}