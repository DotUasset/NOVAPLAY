#include "InteractableLever.h"
#include "Components/StaticMeshComponent.h"
#include "Puzzle_LeverSequence.h"
#include "NOVAPLAYDebug.h"

AInteractableLever::AInteractableLever()
{
	PrimaryActorTick.bCanEverTick = false;
	LeverMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeverMesh"));
	RootComponent = LeverMesh;
	LeverMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	LeverMesh->SetCollisionResponseToAllChannels(ECR_Block);
}

void AInteractableLever::BeginPlay()
{
	Super::BeginPlay();

	if (OwningPuzzle)
	{
		OwningPuzzle->RegisterLever(this);
	}
}

void AInteractableLever::Interact_Implementation(APawn* InteractingPawn)
{
	NOVAPLAYDebug::ScreenMessage(FString::Printf(TEXT("Lever %d interactuada"), LeverIndex));

	if (!OwningPuzzle)
	{
		NOVAPLAYDebug::ScreenMessage(TEXT("OwningPuzzle es NULL!"), FColor::Red, 5.0f);
		return;
	}

	OwningPuzzle->ActivateLever(LeverIndex, this);
}