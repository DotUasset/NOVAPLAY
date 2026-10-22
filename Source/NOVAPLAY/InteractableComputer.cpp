#include "InteractableComputer.h"
#include "Components/StaticMeshComponent.h"
#include "Puzzle_BookExchange.h"
#include "NOVAPLAYObjectiveComponent.h"
#include "GameFramework/Pawn.h"

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

		// El canje salió bien (se entregaron los libros y se recibió la llave): esto
		// completa el objetivo activo de "entregar los libros" y activa el siguiente
		// de la cadena (tipicamente "Escapar").
		if (InteractingPawn)
		{
			if (UNOVAPLAYObjectiveComponent* Objectives = InteractingPawn->FindComponentByClass<UNOVAPLAYObjectiveComponent>())
			{
				Objectives->CompleteCurrentObjective();
			}
		}
	}
	else
	{
		OnExchangeFailed();
	}
}
