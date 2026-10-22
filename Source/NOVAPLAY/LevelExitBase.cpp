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
			Puzzle->OnPuzzleSolved.AddUniqueDynamic(this, &ALevelExitBase::HandlePuzzleSolved);
		}
	}

	ExitTrigger->OnComponentBeginOverlap.AddUniqueDynamic(this, &ALevelExitBase::OnExitTriggerOverlap);

	// Por si los puzzles ya estaban resueltos antes de este BeginPlay (orden de BeginPlay no
	// garantizado entre actores): sin esto la salida quedaria cerrada para siempre.
	if (RequiredPuzzles.Num() > 0)
	{
		CheckAllSolved();
	}
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

	// BeginOverlap no se dispara si el jugador ya estaba parado dentro del trigger cuando se abrio
	TArray<AActor*> AlreadyInside;
	ExitTrigger->GetOverlappingActors(AlreadyInside, ACharacter::StaticClass());
	for (AActor* Actor : AlreadyInside)
	{
		TryBeginExitSequence(Actor);
	}
}

void ALevelExitBase::OnExitTriggerOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	TryBeginExitSequence(OtherActor);
}

void ALevelExitBase::TryBeginExitSequence(AActor* OtherActor)
{
	// Solo el jugador puede salir: el docente tambien es un ACharacter y, si lo persigue hasta
	// la salida abierta, antes cargaba el siguiente nivel por su cuenta.
	const APawn* Pawn = Cast<APawn>(OtherActor);
	if (!bIsOpened || bExitSequenceStarted || !Pawn || !Pawn->IsPlayerControlled())
	{
		return;
	}

	bExitSequenceStarted = true;

	// Esto completa el ultimo objetivo de la cadena del nivel (tipicamente "Escapar").
	if (UNOVAPLAYObjectiveComponent* Objectives = Pawn->FindComponentByClass<UNOVAPLAYObjectiveComponent>())
	{
		Objectives->CompleteCurrentObjective();
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