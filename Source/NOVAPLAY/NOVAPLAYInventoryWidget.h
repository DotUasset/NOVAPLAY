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

protected:
	// Implementalo en WBP_Inventory: reconstruir la lista visual
	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory")
	void OnInventoryUpdated(const TArray<FInventoryItem>& Items);

private:
	UFUNCTION()
	void HandleInventoryChanged();

	UPROPERTY()
	class UNOVAPLAYInventoryComponent* CachedInventoryComponent = nullptr;
};