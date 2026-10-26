#include "ClassroomSeat.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"
#include "NOVAPLAYCharacter.h"
#include "NOVAPLAYObjectiveComponent.h"
#include "NOVAPLAYDebug.h"

AClassroomSeat::AClassroomSeat()
{
	PrimaryActorTick.bCanEverTick = false;

	SeatMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SeatMesh"));
	RootComponent = SeatMesh;
	// Bloquea Visibility: es el canal que usa el trace de interaccion del personaje
	SeatMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SeatMesh->SetCollisionResponseToAllChannels(ECR_Block);

	SitPoint = CreateDefaultSubobject<USceneComponent>(TEXT("SitPoint"));
	SitPoint->SetupAttachment(SeatMesh);
	SitPoint->SetRelativeLocation(FVector(0.0f, 0.0f, 60.0f));
}

void AClassroomSeat::SetAssigned(bool bAssigned)
{
	bIsAssignedSeat = bAssigned;
	OnAssignedChanged(bAssigned);
}

void AClassroomSeat::Interact_Implementation(APawn* InteractingPawn)
{
	if (bSequenceStarted)
	{
		return;
	}

	if (!bIsAssignedSeat)
	{
		NOVAPLAYDebug::ScreenMessage(WrongSeatMessage.ToString(), FColor::Orange, 2.0f);
		OnWrongSeat();
		return;
	}

	ANOVAPLAYCharacter* Player = Cast<ANOVAPLAYCharacter>(InteractingPawn);
	if (!Player)
	{
		return;
	}

	bSequenceStarted = true;

	// Completa el ultimo objetivo de la cadena (ej: "Sientate en tu asiento"), igual que LevelExitBase
	if (UNOVAPLAYObjectiveComponent* Objectives = Player->FindComponentByClass<UNOVAPLAYObjectiveComponent>())
	{
		Objectives->CompleteCurrentObjective();
	}

	// Mueve al jugador al asiento y le bloquea el control
	Player->SitDown(SitPoint->GetComponentTransform());
	OnPlayerSat();

	// Fundido a negro. bHoldWhenFinished = true: se queda negro hasta que cambie el nivel.
	if (APlayerController* PC = Cast<APlayerController>(Player->GetController()))
	{
		if (PC->PlayerCameraManager)
		{
			PC->PlayerCameraManager->StartCameraFade(0.0f, 1.0f, FadeDuration, FLinearColor::Black, false, true);
		}
	}

	// Rate <= 0 en SetTimer CANCELA el timer, por eso el Max
	GetWorldTimerManager().SetTimer(BellTimerHandle, this, &AClassroomSeat::PlayBell, FMath::Max(FadeDuration, 0.01f), false);
}

void AClassroomSeat::PlayBell()
{
	if (BellSound)
	{
		UGameplayStatics::PlaySound2D(this, BellSound);
	}

	GetWorldTimerManager().SetTimer(MenuTimerHandle, this, &AClassroomSeat::GoToMenu, FMath::Max(PostBellDelay, 0.01f), false);
}

void AClassroomSeat::GoToMenu()
{
	UGameplayStatics::OpenLevel(this, MenuLevelName);
}
