#include "LevelExitBase.h"
#include "PuzzleBase.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "NOVAPLAYObjectiveComponent.h"
#include "TimerManager.h"

ALevelExitBase::ALevelExitBase()
{
	PrimaryActorTick.bCanEverTick = false;

	ExitTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("ExitTrigger"));
	// OJO: a proposito NO se attachea aca. Cada subclase lo cuelga de su propio
	// mesh raiz en su constructor, una vez que ese root ya existe.
	ExitTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ExitTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	ExitTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void ALevelExitBase::BeginPlay()
{
	Super::BeginPlay();

	for (APuzzleBase* Puzzle : RequiredPuzzles)
	{
		if (Puzzle)
		{
			Puzzle->OnPuzzleSolved.AddDynamic(this, &ALevelExitBase::HandlePuzzleSolved);
		}
	}

	ExitTrigger->OnComponentBeginOverlap.AddDynamic(this, &ALevelExitBase::OnExitTriggerOverlap);
}

void ALevelExitBase::HandlePuzzleSolved(APuzzleBase* SolvedPuzzle)
{
	CheckAllSolved();
}

void ALevelExitBase::CheckAllSolved()
{
	if (bIsOpened)
	{
		return;
	}

	for (APuzzleBase* Puzzle : RequiredPuzzles)
	{
		if (!Puzzle || !Puzzle->bIsSolved)
		{
			return;
		}
	}

	bIsOpened = true;
	OnExitOpened();
}

void ALevelExitBase::OnExitTriggerOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!bIsOpened || !OtherActor || !OtherActor->IsA<ACharacter>() || bExitSequenceStarted)
	{
		return;
	}

	bExitSequenceStarted = true;

	// Esto completa el ultimo objetivo de la cadena del nivel (tipicamente "Escapar").
	if (ACharacter* EscapingCharacter = Cast<ACharacter>(OtherActor))
	{
		if (UNOVAPLAYObjectiveComponent* Objectives = EscapingCharacter->FindComponentByClass<UNOVAPLAYObjectiveComponent>())
		{
			Objectives->CompleteCurrentObjective();
		}
	}

	if (!NextLevelName.IsNone())
	{
		GetWorldTimerManager().SetTimer(ExitDelayTimerHandle, this, &ALevelExitBase::OpenNextLevel, ExitDelay, false);
	}
}

void ALevelExitBase::OpenNextLevel()
{
	UGameplayStatics::OpenLevel(this, NextLevelName);
}