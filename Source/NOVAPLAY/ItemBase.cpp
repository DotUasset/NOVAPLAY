// ItemBase.cpp
#include "ItemBase.h"
#include "Components/StaticMeshComponent.h"

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
	UE_LOG(LogTemp, Warning, TEXT("Interact_Implementation llamado en item: %s"), *ItemName.ToString());

	// Por ahora solo lo destruimos, para confirmar que el flujo entero funciona.
	// Cuando armemos el InventoryComponent (próximo paso), acá vamos a
	// llamar a algo como InteractingPawn->FindComponentByClass<UInventoryComponent>()->AddItem(this)
	// antes de destruirlo.
	Destroy();
}