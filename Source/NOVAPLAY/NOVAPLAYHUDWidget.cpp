// Copyright Epic Games, Inc. All Rights Reserved.

#include "NOVAPLAYHUDWidget.h"

void UNOVAPLAYHUDWidget::UpdateStaminaBar(float StaminaPercent)
{
	// Dispara el evento que vas a "cablear" en el Widget Blueprint
	OnStaminaUpdated(StaminaPercent);
}

void UNOVAPLAYHUDWidget::SetInteractionPrompt(bool bVisible, const FText& PromptText, AActor* TargetActor)
{
	OnInteractionPromptChanged(bVisible, PromptText, TargetActor);
}

void UNOVAPLAYHUDWidget::InitializeInventory(UNOVAPLAYInventoryComponent* InInventoryComponent)
{
	if (!InInventoryComponent)
	{
		return;
	}

	CachedInventoryComponent = InInventoryComponent;

	// Nos suscribimos: a partir de ahora, cada AddItem/RemoveItem dispara HandleInventoryChanged
	CachedInventoryComponent->OnInventoryChanged.AddDynamic(this, &UNOVAPLAYHUDWidget::HandleInventoryChanged);

	// Mostramos el estado inicial por si ya había ítems agregados antes de crear el widget
	HandleInventoryChanged();
}

void UNOVAPLAYHUDWidget::HandleInventoryChanged()
{
	if (CachedInventoryComponent)
	{
		OnInventoryUpdated(CachedInventoryComponent->GetItems());
	}
}

void UNOVAPLAYHUDWidget::SetInventoryVisible(bool bVisible)
{
	OnInventoryVisibilityChanged(bVisible);
}