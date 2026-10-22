#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NOVAPLAYInventoryComponent.h"
#include "NOVAPLAYInventoryWidget.generated.h"

UCLASS()
class NOVAPLAY_API UNOVAPLAYInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Llamado una vez desde el personaje en BeginPlay
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void InitializeInventory(class UNOVAPLAYInventoryComponent* InInventoryComponent);

	// Función puente: la llamás desde el grafo de WBP_Inventory cuando el
	// jugador clickea un slot. Reenvía el pedido al InventoryComponent real
	// sin que el Blueprint necesite guardar su propia referencia al componente.
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool UseItem(FName ItemID);

protected:
	virtual void NativeDestruct() override;

	// Implementalo en WBP_Inventory: reconstruir la lista visual
	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory")
	void OnInventoryUpdated(const TArray<FInventoryItem>& Items);

private:
	UFUNCTION()
	void HandleInventoryChanged();

	UPROPERTY()
	class UNOVAPLAYInventoryComponent* CachedInventoryComponent = nullptr;
};
