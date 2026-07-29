// Copyright Epic Games, Inc. All Rights Reserved.

#include "NOVAPLAYCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "NOVAPLAYHUDWidget.h"
#include "Blueprint/UserWidget.h"
#include "Interactable.h"
#include "NOVAPLAYInventoryComponent.h"

//////////////////////////////////////////////////////////////////////////
// ANOVAPLAYCharacter

ANOVAPLAYCharacter::ANOVAPLAYCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Permite que este character pueda agacharse (Crouch()/UnCrouch())
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	// Velocidad al caminar agachado (menor a la velocidad normal de caminar)
	GetCharacterMovement()->MaxWalkSpeedCrouched = 150.0f; // ajustá el número a gusto

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)

	// Stamina arranca llena
	CurrentStamina = MaxStamina;

	// Componente de inventario del personaje
	NOVAPLAYInventoryComponent = CreateDefaultSubobject<UNOVAPLAYInventoryComponent>(TEXT("InventoryComponent"));
}

//////////////////////////////////////////////////////////////////////////
// BeginPlay / HUD

void ANOVAPLAYCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HUDWidgetClass)
	{
		HUDWidgetInstance = CreateWidget<UNOVAPLAYHUDWidget>(GetWorld(), HUDWidgetClass);
		if (HUDWidgetInstance)
		{
			HUDWidgetInstance->AddToViewport();

			// NUEVO: conectamos el widget con el inventario de este personaje
			HUDWidgetInstance->InitializeInventory(NOVAPLAYInventoryComponent);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Tick / Stamina

void ANOVAPLAYCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateStamina(DeltaTime);

	if (!bInventoryOpen)
	{
		PerformInteractionTrace();
	}
}

void ANOVAPLAYCharacter::UpdateStamina(float DeltaTime)
{
	const bool bIsGrounded = GetCharacterMovement()->IsMovingOnGround();

	if (bIsSprinting && bIsGrounded && GetCharacterMovement()->Velocity.SizeSquared() > 0.0f)
	{
		CurrentStamina -= StaminaDrainRate * DeltaTime;
		CurrentStamina = FMath::Max(CurrentStamina, 0.0f);

		if (CurrentStamina <= 0.0f)
		{
			StopSprint();
		}
	}
	else
	{
		CurrentStamina += StaminaRegenRate * DeltaTime;
		CurrentStamina = FMath::Min(CurrentStamina, MaxStamina);
	}

	if (HUDWidgetInstance)
	{
		HUDWidgetInstance->UpdateStaminaBar(GetStaminaPercent());
	}
}

//////////////////////////////////////////////////////////////////////////
// Sprint / Crouch

void ANOVAPLAYCharacter::StartSprint()
{
	if (bInventoryOpen)
	{
		return;
	}

	if (CurrentStamina > MinStaminaToSprint && !GetCharacterMovement()->IsCrouching())
	{
		bIsSprinting = true;
		GetCharacterMovement()->MaxWalkSpeed = 600.0f;
	}
}
void ANOVAPLAYCharacter::StopSprint()
{
	bIsSprinting = false;
	GetCharacterMovement()->MaxWalkSpeed = 300.0f; // velocidad normal del template
}

void ANOVAPLAYCharacter::ToggleCrouch()
{
	if (bInventoryOpen)
	{
		return;
	}

	// No permitimos agacharse/pararse mientras el personaje está en el aire
	if (!GetCharacterMovement()->IsMovingOnGround())
	{
		return;
	}

	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void ANOVAPLAYCharacter::HandleJumpInput()
{
	if (bInventoryOpen)
	{
		return;
	}

	if (bIsCrouched)
	{
		return;
	}

	// El costo extra de stamina solo se cobra si el salto arranca desde el piso,
	// mientras estás sprinteando, Y realmente te estás moviendo
	// (evita que cobre stamina si estás parado con Shift apretado sin caminar)
	const bool bIsActuallyMoving = GetCharacterMovement()->Velocity.SizeSquared() > 0.0f;

	if (bIsSprinting && GetCharacterMovement()->IsMovingOnGround() && bIsActuallyMoving)
	{
		CurrentStamina -= JumpStaminaCost;
		CurrentStamina = FMath::Max(CurrentStamina, 0.0f);

		if (HUDWidgetInstance)
		{
			HUDWidgetInstance->UpdateStaminaBar(GetStaminaPercent());
		}

		if (CurrentStamina <= 0.0f)
		{
			StopSprint();
		}
	}

	Jump();
}

//////////////////////////////////////////////////////////////////////////
// Interacción

void ANOVAPLAYCharacter::PerformInteractionTrace()
{
	if (!FollowCamera)
	{
		return;
	}

	const FVector Start = FollowCamera->GetComponentLocation();
	const FVector End = Start + (FollowCamera->GetComponentRotation().Vector() * InteractionDistance);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	// Sphere trace en vez de line trace: da un margen de tolerancia
	// (InteractionTraceRadius) para no tener que apuntar pixel-perfect,
	// útil para objetos bajos como items en el piso.
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(InteractionTraceRadius);

	FHitResult Hit;
	const bool bHit = GetWorld()->SweepSingleByChannel(Hit, Start, End, FQuat::Identity, ECC_Visibility, SphereShape, Params);

	if (GEngine)
	{
		const FString Msg = bHit ? FString::Printf(TEXT("HIT: %s a distancia %.0f"), *Hit.GetActor()->GetName(), Hit.Distance) : TEXT("SIN HIT");
		GEngine->AddOnScreenDebugMessage(3, 0.0f, FColor::Cyan, Msg);
	}

	AActor* NewFocus = nullptr;

	if (bHit && Hit.GetActor() && Hit.GetActor()->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
	{
		NewFocus = Hit.GetActor();
	}

	if (NewFocus != FocusedInteractable)
	{
		FocusedInteractable = NewFocus;

		if (GEngine)
		{
			const FString Msg = FocusedInteractable ? FString::Printf(TEXT("Enfocando: %s"), *FocusedInteractable->GetName()) : TEXT("Sin foco");
			GEngine->AddOnScreenDebugMessage(1, 2.0f, FColor::Yellow, Msg);
		}

		if (HUDWidgetInstance)
		{
			const bool bShowPrompt = (FocusedInteractable != nullptr);
			HUDWidgetInstance->SetInteractionPrompt(bShowPrompt, FText::FromString(TEXT("Presiona E para interactuar")), FocusedInteractable);
		}
	}
}

void ANOVAPLAYCharacter::Interact()
{
	if (bInventoryOpen)
	{
		return;
	}

	if (GEngine)
	{
		const FString Msg = FocusedInteractable ? FString::Printf(TEXT("Interact() -> %s"), *FocusedInteractable->GetName()) : TEXT("Interact() -> nada enfocado");
		GEngine->AddOnScreenDebugMessage(2, 2.0f, FColor::Green, Msg);
	}

	if (FocusedInteractable && FocusedInteractable->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
	{
		IInteractable::Execute_Interact(FocusedInteractable, this);
	}
}

void ANOVAPLAYCharacter::ToggleInventory()
{
	bInventoryOpen = !bInventoryOpen;

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		return;
	}

	if (bInventoryOpen)
	{
		// Bloqueamos movimiento y cámara mientras el inventario está abierto
		PC->SetIgnoreMoveInput(true);
		PC->SetIgnoreLookInput(true);

		// Liberamos el mouse para poder clickear/arrastrar la UI
		PC->SetInputMode(FInputModeGameAndUI());
		PC->bShowMouseCursor = true;
	}
	else
	{
		PC->SetIgnoreMoveInput(false);
		PC->SetIgnoreLookInput(false);

		PC->SetInputMode(FInputModeGameOnly());
		PC->bShowMouseCursor = false;
	}

	if (HUDWidgetInstance)
	{
		HUDWidgetInstance->SetInventoryVisible(bInventoryOpen);
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void ANOVAPLAYCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ANOVAPLAYCharacter::HandleJumpInput);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &ANOVAPLAYCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ANOVAPLAYCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ANOVAPLAYCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ANOVAPLAYCharacter::LookUpAtRate);

	// Sprint y Crouch
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ANOVAPLAYCharacter::StartSprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ANOVAPLAYCharacter::StopSprint);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ANOVAPLAYCharacter::ToggleCrouch);
	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &ANOVAPLAYCharacter::Interact);

	// Abrir inventario
	PlayerInputComponent->BindAction("ToggleInventory", IE_Pressed, this, &ANOVAPLAYCharacter::ToggleInventory);
}

void ANOVAPLAYCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ANOVAPLAYCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ANOVAPLAYCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ANOVAPLAYCharacter::MoveRight(float Value)
{
	if ( (Controller != nullptr) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}
