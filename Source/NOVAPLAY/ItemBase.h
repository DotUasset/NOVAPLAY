// ItemBase.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "ItemBase.generated.h"

/**
 * Clase base para cualquier ítem agarrable del piso.
 * Implementa IInteractable para poder ser interactuado con la tecla E.
 */
UCLASS()
class NOVAPLAY_API AItemBase : public AActor, public IInteractable
{
	GENERATED_BODY()

public:
	AItemBase();

	// Mesh visible del ítem en el mundo
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item", meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* ItemMesh;

	// Nombre a mostrar (lo vamos a usar más adelante en el inventario/HUD)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FText ItemName = FText::FromString(TEXT("Item"));

	// Identificador único de este tipo de ítem (ej: "KEY_LIBRARY_01").
	// Se usa para que el inventario sepa si dos ítems son "el mismo tipo" y apilarlos.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FName ItemID;

	// Ícono a mostrar en el inventario
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	class UTexture2D* ItemIcon = nullptr;

	// Descripción del ítem
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FText ItemDescription;

	// Implementación de la interfaz: qué pasa cuando el personaje aprieta E sobre este ítem
	virtual void Interact_Implementation(APawn* InteractingPawn) override;

protected:
	virtual void BeginPlay() override;
};