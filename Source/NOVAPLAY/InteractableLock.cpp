#include "InteractableLock.h"
#include "Components/StaticMeshComponent.h"
#include "Puzzle_ItemCombination.h"

AInteractableLock::AInteractableLock()
{
	PrimaryActorTick.bCanEverTick = false;
	LockMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LockMesh"));
	RootComponent = LockMesh;
	LockMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	LockMesh->SetCollisionResponseToAllChannels(ECR_Block);
}

void AInteractableLock::Interact_Implementation(APawn* InteractingPawn)
{
	if (OwningPuzzle)
	{
		OwningPuzzle->TryCombine(InteractingPawn);
	}
}