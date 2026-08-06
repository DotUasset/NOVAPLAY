#include "InteractableLever.h"
#include "Components/StaticMeshComponent.h"
#include "Puzzle_LeverSequence.h"

AInteractableLever::AInteractableLever()
{
	PrimaryActorTick.bCanEverTick = false;
	LeverMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeverMesh"));
	RootComponent = LeverMesh;
	LeverMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	LeverMesh->SetCollisionResponseToAllChannels(ECR_Block);
}

void AInteractableLever::Interact_Implementation(APawn* InteractingPawn)
{
	if (OwningPuzzle)
	{
		OwningPuzzle->ActivateLever(LeverIndex);
	}
}