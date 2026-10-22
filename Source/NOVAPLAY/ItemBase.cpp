// ItemBase.cpp

#include "ItemBase.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "NOVAPLAY.h"
#include "NOVAPLAYDebug.h"
#include "NOVAPLAYInventoryComponent.h"

AItemBase::AItemBase()
{
	PrimaryActorTick.bCanEverTick = false;

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	RootComponent = ItemMesh;

	// Nos aseguramos de que el mesh bloquee todos los canales,
	// incluido Visibility (el que usa el trace de interaccion).
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
	if (!Inventory)
	{
		UE_LOG(LogNOVAPLAY, Warning, TEXT("%s: el pawn que interactuo no tiene InventoryComponent"), *GetName());
		return;
	}

	if (!Inventory->AddItem(ItemID, ItemName, ItemIcon, ItemDescription, 1))
	{
		UE_LOG(LogNOVAPLAY, Warning, TEXT("%s: no se pudo agregar '%s' al inventario (ItemID vacio?)"), *GetName(), *ItemID.ToString());
		return;
	}

	UE_LOG(LogNOVAPLAY, Log, TEXT("Item %s agregado al inventario"), *ItemID.ToString());
	NOVAPLAYDebug::ScreenMessage(FString::Printf(TEXT("Agregado: %s"), *ItemName.ToString()), FColor::Green, 3.0f, 4);

	Destroy();
}
