// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NOVAPLAYInventoryComponent.h"
#include "NOVAPLAYObjectiveComponent.h"
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

	// NUEVO: llamado una vez desde el personaje en BeginPlay, le pasamos SU componente
	// de objetivos. Nos suscribimos a sus delegates para enterarnos de todo lo que pasa
	// con los objetivos del nivel (objetivo principal, progreso, completados).
	UFUNCTION(BlueprintCallable, Category = "Objectives")
	void InitializeObjectives(class UNOVAPLAYObjectiveComponent* InObjectiveComponent);

protected:
	// Suelta los bindings a los componentes del personaje al destruirse el widget
	virtual void NativeDestruct() override;

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

	// NUEVO: se dispara UNA vez al arrancar el nivel, con el objetivo principal
	// (ej "Escapa de la biblioteca"). Implementalo en el Widget Blueprint: mostrar el
	// texto grande en pantalla y después desvanecerlo (animación de UMG con un Delay).
	UFUNCTION(BlueprintImplementableEvent, Category = "Objectives")
	void OnMainObjectiveShown(const FText& MainObjectiveText);

	// NUEVO: se dispara cada vez que cambia la lista de objetivos (progreso, nuevo
	// objetivo activado, uno completado). Implementalo reconstruyendo la lista visual
	// completa a partir del array recibido (cada entrada trae su texto, si está
	// completado, y el contador si corresponde).
	UFUNCTION(BlueprintImplementableEvent, Category = "Objectives")
	void OnObjectivesListUpdated(const TArray<FObjectiveEntry>& Objectives);

	// NUEVO: se dispara UNA vez, justo cuando un objetivo se completa, con su texto.
	// Implementalo mostrando el cartel de "Objetivo completado" con su propia animación,
	// por separado de la lista (que se actualiza aparte con OnObjectivesListUpdated).
	UFUNCTION(BlueprintImplementableEvent, Category = "Objectives")
	void OnObjectiveCompletedFlash(const FText& CompletedObjectiveText);

private:
	// NUEVO: función "puente" que se engancha al delegate OnInventoryChanged.
	// Tiene que ser UFUNCTION() para poder bindearse a un delegate dinámico.
	UFUNCTION()
	void HandleInventoryChanged();

	// Guardamos la referencia para poder leer GetItems() cuando se dispare el delegate
	UPROPERTY()
	class UNOVAPLAYInventoryComponent* CachedInventoryComponent = nullptr;

	// NUEVO: funciones "puente" enganchadas a los delegates de UNOVAPLAYObjectiveComponent.
	UFUNCTION()
	void HandleObjectivesChanged();

	UFUNCTION()
	void HandleObjectiveCompleted(FText CompletedObjectiveText);

	UFUNCTION()
	void HandleMainObjectiveRequested(FText MainObjectiveText);

	// Guardamos la referencia para poder leer GetObjectives() cuando se dispare el delegate
	UPROPERTY()
	class UNOVAPLAYObjectiveComponent* CachedObjectiveComponent = nullptr;
};
