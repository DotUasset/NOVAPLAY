// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NOVAPLAYHUDWidget.generated.h"

/**
 * Clase base del HUD. La lógica de qué mostrar vive acá (C++).
 * El diseño visual (colores, forma de la barra, íconos) se hace
 * en el Widget Blueprint que va a heredar de esta clase.
 */
UCLASS()
class NOVAPLAY_API UNOVAPLAYHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	// Llamado desde el personaje cada vez que cambia la stamina.
	// BlueprintCallable: se puede invocar desde C++ o Blueprint.
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void UpdateStaminaBar(float StaminaPercent);

	// Muestra/oculta el prompt de interacción ("Presiona E para agarrar")
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void SetInteractionPrompt(bool bVisible, const FText& PromptText);

protected:

	// BlueprintImplementableEvent: NO tiene cuerpo en C++.
	// Vos lo "implementás" armando el nodo en el grafo del Widget Blueprint
	// (ahí conectás esto a la barra de progreso visual).
	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void OnStaminaUpdated(float StaminaPercent);

	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void OnInteractionPromptChanged(bool bVisible, const FText& PromptText);
};
