#include "InteractableSwitch.h"
#include "Components/StaticMeshComponent.h"
#include "Puzzle_LeverCount.h"
#include "NOVAPLAYDebug.h"

AInteractableSwitch::AInteractableSwitch()
{
	PrimaryActorTick.bCanEverTick = false;

	SwitchMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SwitchMesh"));
	RootComponent = SwitchMesh;
	// Bloquea Visibility: es el canal que usa el trace de interaccion del personaje
	SwitchMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SwitchMesh->SetCollisionResponseToAllChannels(ECR_Block);
}

void AInteractableSwitch::Interact_Implementation(APawn* InteractingPawn)
{
	if (bActivated)
	{
		return; // ya esta activada, no se puede volver a usar
	}

	if (!OwningPuzzle)
	{
		NOVAPLAYDebug::ScreenMessage(TEXT("Switch sin OwningPuzzle asignado!"), FColor::Red, 5.0f);
		return;
	}

	bActivated = true;

	if (ActiveMesh)
	{
		SwitchMesh->SetStaticMesh(ActiveMesh);
	}

	OnSwitchActivated();
	OwningPuzzle->NotifySwitchActivated(this);
}
