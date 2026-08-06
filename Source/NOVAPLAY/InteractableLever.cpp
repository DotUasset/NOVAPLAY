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
	if (GEngine)
	{
		const FString Msg = FString::Printf(TEXT("Lever %d interactuada"), LeverIndex);
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, Msg);
	}

	if (!OwningPuzzle)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("OwningPuzzle es NULL!"));
		}
		return;
	}

	OwningPuzzle->ActivateLever(LeverIndex);
}