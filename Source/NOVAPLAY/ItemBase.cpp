// ItemBase.cpp
#include "ItemBase.h"
#include "Components/StaticMeshComponent.h"
#include "NOVAPLAYInventoryComponent.h"

AItemBase::AItemBase()
{
	PrimaryActorTick.bCanEverTick = false;

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	RootComponent = ItemMesh;

	// Nos aseguramos de que el mesh bloquee todos los canales,
	// incluido Visibility (el que usa el trace de interacción).
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ItemMesh->SetCollisionResponseToAllChannels(ECR_Block);
}

void AItemBase::BeginPlay()
{
	Super::BeginPlay();
}

void AItemBase::Interact_Implementation(APawn* InteractingPawn)
{
	if (!InteractingPawn)
	{
		return;
	}

	UNOVAPLAYInventoryComponent* Inventory = InteractingPawn->FindComponentByClass<UNOVAPLAYInventoryComponent>();

	if (Inventory)
	{
		const bool bAdded = Inventory->AddItem(ItemID, ItemName, ItemIcon, ItemDescription, 1);

		if (bAdded)
		{
			UE_LOG(LogTemp, Log, TEXT("Item %s agregado al inventario"), *ItemID.ToString());

			if (bAdded)
			{
				UE_LOG(LogTemp, Log, TEXT("Item %s agregado al inventario"), *ItemID.ToString());

				if (GEngine)
				{
					const FString Msg = FString::Printf(TEXT("Agregado: %s"), *ItemName.ToString());
					GEngine->AddOnScreenDebugMessage(4, 3.0f, FColor::Green, Msg);
				}

				Destroy();
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("No se pudo agregar %s al inventario"), *ItemID.ToString());
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("El pawn que interactuó no tiene InventoryComponent"));
	}
}