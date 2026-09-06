// NOVAPLAY - Personaje enemigo (docente) que patrulla, persigue y atrapa al jugador

#include "TeacherCharacter.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NOVAPLAYCharacter.h"
#include "TimerManager.h"

ATeacherCharacter::ATeacherCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	CatchSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CatchSphere"));
	CatchSphere->SetupAttachment(RootComponent);
	CatchSphere->SetSphereRadius(CatchRadius);
	CatchSphere->SetCollisionProfileName(TEXT("Trigger"));
	CatchSphere->SetGenerateOverlapEvents(true);

	// El docente gira suavemente hacia donde se esta moviendo en vez de girar
	// instantaneamente hacia donde apunta el controller (evita el giro "teleport" visual).
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->bOrientRotationToMovement = true;

		// Arranca a velocidad de patrulla
		MoveComp->MaxWalkSpeed = PatrolSpeed;

		// Acelera y frena mas parecido al jugador: si no, arranca/frena "patinando"
		// y da sensacion de lentitud aunque MaxWalkSpeed sea alto.
		MoveComp->MaxAcceleration = 1536.0f;
		MoveComp->BrakingDecelerationWalking = 1536.0f;
		MoveComp->GroundFriction = 8.0f;
	}
}

void ATeacherCharacter::BeginPlay()
{
	Super::BeginPlay();

	CatchSphere->SetSphereRadius(CatchRadius);
	CatchSphere->OnComponentBeginOverlap.AddDynamic(this, &ATeacherCharacter::OnCatchOverlap);

	// Arranca en velocidad/estado de patrulla desde el primer frame
	SetMovementState(false);
	NotifyMovementState(ETeacherMovementState::Patrolling);
}

void ATeacherCharacter::SetMovementState(bool bIsChasingOrInvestigating)
{
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = bIsChasingOrInvestigating ? ChaseSpeed : PatrolSpeed;
	}
}

void ATeacherCharacter::NotifyMovementState(ETeacherMovementState NewState)
{
	if (bHasNotifiedMovementStateOnce && NewState == LastNotifiedMovementState)
	{
		return;
	}

	bHasNotifiedMovementStateOnce = true;
	LastNotifiedMovementState = NewState;

	// El Event Graph de BP_Teacher decide que sonido/print corresponde a cada estado
	OnMovementStateChanged(NewState);
}

AActor* ATeacherCharacter::GetNextPatrolPoint()
{
	if (PatrolPoints.Num() == 0)
	{
		return nullptr;
	}

	if (bPatrolInOrder || PatrolPoints.Num() == 1)
	{
		CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
	}
	else
	{
		// Elige un indice distinto al actual, para no repetir el mismo punto dos veces seguidas
		int32 NewIndex = CurrentPatrolIndex;
		while (NewIndex == CurrentPatrolIndex)
		{
			NewIndex = FMath::RandRange(0, PatrolPoints.Num() - 1);
		}
		CurrentPatrolIndex = NewIndex;
	}

	return PatrolPoints.IsValidIndex(CurrentPatrolIndex) ? PatrolPoints[CurrentPatrolIndex] : nullptr;
}

void ATeacherCharacter::OnCatchOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bHasCaughtPlayer)
	{
		return;
	}

	if (ANOVAPLAYCharacter* Player = Cast<ANOVAPLAYCharacter>(OtherActor))
	{
		bHasCaughtPlayer = true;
		CaughtPlayerRef = Player;

		// Sonido/print de "te atrape" instantaneo, implementado en el Event Graph de BP_Teacher
		OnPlayerCaught(Player);

		if (CatchDelay <= 0.0f)
		{
			HandleCatchDelayFinished();
		}
		else
		{
			GetWorldTimerManager().SetTimer(CatchTimerHandle, this, &ATeacherCharacter::HandleCatchDelayFinished, CatchDelay, false);
		}
	}
}

void ATeacherCharacter::HandleCatchDelayFinished()
{
	// El Event Graph de BP_Teacher castea CaughtPlayerRef al Blueprint del jugador y llama
	// "Show Caught Screen" para bloquear el input y mostrar WBP_CaughtScreen.
	OnCaughtSequenceComplete(CaughtPlayerRef);
}
