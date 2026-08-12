#include "InteractableComputer.h"
#include "Components/StaticMeshComponent.h"
#include "Puzzle_BookExchange.h"

AInteractableComputer::AInteractableComputer()
{
	PrimaryActorTick.bCanEverTick = false;

	ComputerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ComputerMesh"));
	RootComponent = ComputerMesh;

	ComputerMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ComputerMesh->SetCollisionResponseToAllChannels(ECR_Block);
}

void AInteractableComputer::Interact_Implementation(APawn* InteractingPawn)
{
	if (!OwningPuzzle)
	{
		return;
	}

	const bool bSuccess = OwningPuzzle->TryExchange(InteractingPawn);

	if (bSuccess)
	{
		OnExchangeSuccess();
	}
	else
	{
		OnExchangeFailed();
	}
}
