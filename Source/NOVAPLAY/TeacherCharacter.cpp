// NOVAPLAY - Personaje enemigo (docente) que patrulla, persigue y atrapa al jugador

#include "TeacherCharacter.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/TargetPoint.h"
#include "NOVAPLAYCharacter.h"
#include "NOVAPLAYGameMode.h"
#include "TimerManager.h"

ATeacherCharacter::ATeacherCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	CatchSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CatchSphere"));
	CatchSphere->SetupAttachment(RootComponent);
	CatchSphere->SetSphereRadius(CatchRadius);
	CatchSphere->SetCollisionProfileName(TEXT("Trigger"));
	CatchSphere->SetGenerateOverlapEvents(true);

	// El docente NUNCA gira instantaneamente hacia donde apunta el controller: gira
	// suavemente hacia donde se esta moviendo. Esto evita el giro "brusco"/teleport visual.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->bOrientRotationToMovement = true;
		MoveComp->RotationRate = FRotator(0.0f, PatrolRotationRate, 0.0f);

		// Arranca a velocidad de patrulla
		MoveComp->MaxWalkSpeed = PatrolSpeed;

		// Acelera y frena mas parecido al jugador: si no, arranca/frena "patinando"
		// y da sensacion de lentitud aunque MaxWalkSpeed sea alto.
		MoveComp->MaxAcceleration = 1536.0f;
		MoveComp->BrakingDecelerationWalking = 1536.0f;
		MoveComp->GroundFriction = 8.0f;

		// Necesario para que bCanAutoJump tenga suficiente impulso al saltar obstaculos bajos
		MoveComp->JumpZVelocity = 500.0f;
		MoveComp->AirControl = 0.3f;
	}
}

void ATeacherCharacter::BeginPlay()
{
	Super::BeginPlay();

	CatchSphere->SetSphereRadius(CatchRadius);
	CatchSphere->OnComponentBeginOverlap.AddDynamic(this, &ATeacherCharacter::OnCatchOverlap);
}

void ATeacherCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bCanAutoJump)
	{
		UpdateAutoJump(DeltaTime);
	}
}

void ATeacherCharacter::SetMovementState(bool bIsChasing)
{
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = bIsChasing ? ChaseSpeed : PatrolSpeed;
		MoveComp->RotationRate = FRotator(0.0f, bIsChasing ? ChaseRotationRate : PatrolRotationRate, 0.0f);
	}
}

void ATeacherCharacter::UpdateAutoJump(float DeltaTime)
{
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (!MoveComp || !MoveComp->IsMovingOnGround())
	{
		// En el aire o cayendo: no acumules tiempo "trabado"
		StuckTimer = 0.0f;
		return;
	}

	// Esta tratando de moverse (tiene aceleracion pedida por el MoveTo del Behavior Tree)...
	const bool bTryingToMove = MoveComp->GetCurrentAcceleration().SizeSquared() > FMath::Square(50.0f);

	// ...pero casi no se mueve en el plano horizontal: probablemente esta contra un obstaculo.
	const bool bBarelyMoving = GetVelocity().SizeSquared2D() < FMath::Square(StuckSpeedThreshold);

	if (bTryingToMove && bBarelyMoving)
	{
		StuckTimer += DeltaTime;
		if (StuckTimer >= StuckTimeToJump)
		{
			Jump();
			StuckTimer = 0.0f;
		}
	}
	else
	{
		StuckTimer = 0.0f;
	}
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

		if (CatchDelay <= 0.0f)
		{
			NotifyPlayerCaught();
		}
		else
		{
			GetWorldTimerManager().SetTimer(CatchTimerHandle, this, &ATeacherCharacter::NotifyPlayerCaught, CatchDelay, false);
		}
	}
}

void ATeacherCharacter::NotifyPlayerCaught()
{
	if (ANOVAPLAYGameMode* GM = GetWorld()->GetAuthGameMode<ANOVAPLAYGameMode>())
	{
		GM->OnPlayerCaught();
	}
}
