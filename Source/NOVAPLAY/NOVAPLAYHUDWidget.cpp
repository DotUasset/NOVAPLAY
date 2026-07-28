// Copyright Epic Games, Inc. All Rights Reserved.

#include "NOVAPLAYHUDWidget.h"

void UNOVAPLAYHUDWidget::UpdateStaminaBar(float StaminaPercent)
{
	// Dispara el evento que vas a "cablear" en el Widget Blueprint
	OnStaminaUpdated(StaminaPercent);
}

void UNOVAPLAYHUDWidget::SetInteractionPrompt(bool bVisible, const FText& PromptText)
{
	OnInteractionPromptChanged(bVisible, PromptText);
}
