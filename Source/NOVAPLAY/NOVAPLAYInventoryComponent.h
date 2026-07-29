// NOVAPLAYInventoryComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NOVAPLAYInventoryComponent.generated.h"

// Estructura que representa un ítem dentro del inventario.
// No es un actor del mundo, es solo un "dato": nombre, ID único y cantidad.
USTRUCT(BlueprintType)
struct FInventoryItem
{
	GENERATED_BODY()

	// Identificador único del ítem (ej: "KEY_LIBRARY_01"). Se usa para
	// saber si dos ítems son "el mismo tipo" y así poder apilarlos.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	FName ItemID;

	// Nombre para mostrar en la UI (ej: "Llave de la biblioteca")
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	FText ItemName;

	// NUEVO: ícono para mostrar en el slot de la UI
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	class UTexture2D* Icon;

	// NUEVO: descripción del ítem (ej: tooltip al pasar el mouse)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	FText Description;

	// Cuántos tiene el jugador de este ítem
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	int32 Quantity;

	FInventoryItem()
		: ItemID(NAME_None)
		, ItemName(FText::GetEmpty())
		, Icon(nullptr)
		, Description(FText::GetEmpty())
		, Quantity(0)
	{
	}
};

// Delegate sin parámetros: la UI se suscribe a esto y, cuando se dispara,
// simplemente vuelve a pedir la lista completa de ítems con GetItems().
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class NOVAPLAY_API UNOVAPLAYInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNOVAPLAYInventoryComponent();

	// Se dispara cada vez que el inventario cambia (se agrega o saca algo).
	// La UI (Widget Blueprint) se suscribe a esto en su evento BeginPlay.
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryChanged OnInventoryChanged;

	// Agrega "Amount" unidades del ítem identificado por ItemID.
	// Si ya existe en el inventario, suma la cantidad en vez de duplicar la entrada.
	// Devuelve true si se pudo agregar.
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool AddItem(FName ItemID, FText ItemName, class UTexture2D* Icon, FText Description, int32 Amount = 1);

	// Saca "Amount" unidades del ítem. Si la cantidad llega a 0, se elimina
	// la entrada del array. Devuelve true si se pudo sacar (si había suficiente).
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveItem(FName ItemID, int32 Amount = 1);

	// Devuelve true si el inventario tiene al menos 1 unidad de ese ítem.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	bool HasItem(FName ItemID) const;

	// Devuelve cuántas unidades tiene de ese ítem (0 si no lo tiene).
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	int32 GetItemQuantity(FName ItemID) const;

	// Devuelve todo el inventario (para que la UI arme la lista visual).
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	const TArray<FInventoryItem>& GetItems() const { return Items; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TArray<FInventoryItem> Items;

	// Busca el índice de una entrada por ItemID. Devuelve INDEX_NONE si no está.
	int32 FindItemIndex(FName ItemID) const;
};