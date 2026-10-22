#include "InteractableBook.h"
#include "Components/StaticMeshComponent.h"
#include "Puzzle_BookOrder.h"
#include "NOVAPLAYDebug.h"

AInteractableBook::AInteractableBook()
{
	PrimaryActorTick.bCanEverTick = false;
	BookMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BookMesh"));
	RootComponent = BookMesh;
	BookMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BookMesh->SetCollisionResponseToAllChannels(ECR_Block);
}

void AInteractableBook::Interact_Implementation(APawn* InteractingPawn)
{
	NOVAPLAYDebug::ScreenMessage(FString::Printf(TEXT("Book %d interactuado"), BookID));

	if (OwningPuzzle)
	{
		OwningPuzzle->PlaceNextBook(BookID);
	}
}