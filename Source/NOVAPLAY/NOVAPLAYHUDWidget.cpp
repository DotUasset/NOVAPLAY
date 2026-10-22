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
	CachedInventoryComponent->OnInventoryChanged.AddUniqueDynamic(this, &UNOVAPLAYHUDWidget::HandleInventoryChanged);

	// Mostramos el estado inicial por si ya había ítems agregados antes de crear el widget
	HandleInventoryChanged();
}

void UNOVAPLAYHUDWidget::NativeDestruct()
{
	if (CachedInventoryComponent)
	{
		CachedInventoryComponent->OnInventoryChanged.RemoveDynamic(this, &UNOVAPLAYHUDWidget::HandleInventoryChanged);
	}

	if (CachedObjectiveComponent)
	{
		CachedObjectiveComponent->OnObjectivesChanged.RemoveDynamic(this, &UNOVAPLAYHUDWidget::HandleObjectivesChanged);
		CachedObjectiveComponent->OnObjectiveCompleted.RemoveDynamic(this, &UNOVAPLAYHUDWidget::HandleObjectiveCompleted);
		CachedObjectiveComponent->OnMainObjectiveRequested.RemoveDynamic(this, &UNOVAPLAYHUDWidget::HandleMainObjectiveRequested);
	}

	Super::NativeDestruct();
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

void UNOVAPLAYHUDWidget::InitializeObjectives(UNOVAPLAYObjectiveComponent* InObjectiveComponent)
{
	if (!InObjectiveComponent)
	{
		return;
	}

	CachedObjectiveComponent = InObjectiveComponent;

	CachedObjectiveComponent->OnObjectivesChanged.AddUniqueDynamic(this, &UNOVAPLAYHUDWidget::HandleObjectivesChanged);
	CachedObjectiveComponent->OnObjectiveCompleted.AddUniqueDynamic(this, &UNOVAPLAYHUDWidget::HandleObjectiveCompleted);
	CachedObjectiveComponent->OnMainObjectiveRequested.AddUniqueDynamic(this, &UNOVAPLAYHUDWidget::HandleMainObjectiveRequested);

	// Mostramos el estado inicial por si la cadena de objetivos ya habÃ­a arrancado
	// antes de que este widget se creara.
	HandleObjectivesChanged();

	// Mismo problema que arriba, pero para el objetivo principal: OnMainObjectiveRequested
	// se dispara UNA sola vez dentro de StartObjectiveChain, asÃ­ que si este widget se
	// suscribe despuÃ©s de ese momento, se pierde para siempre. Lo resincronizamos a mano.
	const FText CachedMainObjective = CachedObjectiveComponent->GetMainObjectiveTitle();
	if (!CachedMainObjective.IsEmpty())
	{
		HandleMainObjectiveRequested(CachedMainObjective);
	}
}

void UNOVAPLAYHUDWidget::HandleObjectivesChanged()
{
	if (CachedObjectiveComponent)
	{
		OnObjectivesListUpdated(CachedObjectiveComponent->GetObjectives());
	}
}

void UNOVAPLAYHUDWidget::HandleObjectiveCompleted(FText CompletedObjectiveText)
{
	OnObjectiveCompletedFlash(CompletedObjectiveText);
}

void UNOVAPLAYHUDWidget::HandleMainObjectiveRequested(FText MainObjectiveText)
{
	OnMainObjectiveShown(MainObjectiveText);
}