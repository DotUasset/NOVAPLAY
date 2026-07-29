// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NOVAPLAYInventoryComponent.h"
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

	// Muestra/oculta el prompt de interacción ("Presiona E para agarrar").
		// TargetActor es el objeto sobre el que hay que dibujar el prompt en pantalla.
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void SetInteractionPrompt(bool bVisible, const FText& PromptText, AActor* TargetActor);

	// NUEVO: llamado una vez desde el personaje en BeginPlay, le pasamos SU inventario.
// Acá nos suscribimos al delegate para enterarnos cada vez que cambia.
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void InitializeInventory(class UNOVAPLAYInventoryComponent* InInventoryComponent);

	// Llamado desde el personaje al abrir/cerrar el inventario
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetInventoryVisible(bool bVisible);

protected:

	// BlueprintImplementableEvent: NO tiene cuerpo en C++.
	// Vos lo "implementás" armando el nodo en el grafo del Widget Blueprint
	// (ahí conectás esto a la barra de progreso visual).
	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void OnStaminaUpdated(float StaminaPercent);

	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void OnInteractionPromptChanged(bool bVisible, const FText& PromptText, AActor* TargetActor);

	// NUEVO: se dispara cada vez que el inventario cambia. En el Widget Blueprint
// implementás este evento para reconstruir la lista visual de ítems.
	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory")
	void OnInventoryUpdated(const TArray<FInventoryItem>& Items);

	// Implementalo en el Widget Blueprint: mostrar/ocultar el panel del inventario
	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory")
	void OnInventoryVisibilityChanged(bool bVisible);

private:
	// NUEVO: función "puente" que se engancha al delegate OnInventoryChanged.
	// Tiene que ser UFUNCTION() para poder bindearse a un delegate dinámico.
	UFUNCTION()
	void HandleInventoryChanged();

	// Guardamos la referencia para poder leer GetItems() cuando se dispare el delegate
	UPROPERTY()
	class UNOVAPLAYInventoryComponent* CachedInventoryComponent = nullptr;
};
