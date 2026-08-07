#include "LevelExitDoor.h"
#include "PuzzleBase.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

ALevelExitDoor::ALevelExitDoor()
{
	PrimaryActorTick.bCanEverTick = false;

	WallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WallMesh"));
	RootComponent = WallMesh;

	ExitTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("ExitTrigger"));
	ExitTrigger->SetupAttachment(RootComponent);
	ExitTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ExitTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	ExitTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void ALevelExitDoor::BeginPlay()
{
	Super::BeginPlay();

	for (APuzzleBase* Puzzle : RequiredPuzzles)
	{
		if (Puzzle)
		{
			Puzzle->OnPuzzleSolved.AddDynamic(this, &ALevelExitDoor::HandlePuzzleSolved);
		}
	}

	ExitTrigger->OnComponentBeginOverlap.AddDynamic(this, &ALevelExitDoor::OnExitTriggerOverlap);
}

void ALevelExitDoor::HandlePuzzleSolved(APuzzleBase* SolvedPuzzle)
{
	CheckAllSolved();
}

void ALevelExitDoor::CheckAllSolved()
{
	if (bWallOpened)
	{
		return;
	}

	for (APuzzleBase* Puzzle : RequiredPuzzles)
	{
		if (!Puzzle || !Puzzle->bIsSolved)
		{
			return; // falta al menos uno
		}
	}

	bWallOpened = true;
	OnWallOpened(); // solo anima la pared, todavía NO carga nivel
}

void ALevelExitDoor::OnExitTriggerOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!bWallOpened || !OtherActor || !OtherActor->IsA<ACharacter>())
	{
		return;
	}

	if (!NextLevelName.IsNone())
	{
		UGameplayStatics::OpenLevel(this, NextLevelName);
	}
}