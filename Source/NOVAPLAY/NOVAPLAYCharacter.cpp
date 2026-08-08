// Copyright Epic Games, Inc. All Rights Reserved.

#include "NOVAPLAYCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "NOVAPLAYHUDWidget.h"
#include "Blueprint/UserWidget.h"
#include "Interactable.h"
#include "NOVAPLAYInventoryComponent.h"
#include "NOVAPLAYInventoryWidget.h"
#include "NOVAPLAYPauseMenuWidget.h"
#include "NOVAPLAYSettingsSave.h"
#include "Kismet/GameplayStatics.h"

//////////////////////////////////////////////////////////////////////////
// ANOVAPLAYCharacter

ANOVAPLAYCharacter::ANOVAPLAYCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	StandingCapsuleHalfHeight = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Primera persona: el personaje gira en yaw junto con el mouse (el pitch lo maneja
	// la camara sola, sin afectar la capsula, para no inclinar el cuerpo al mirar arriba/abajo)
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false; // ya no gira hacia donde camina: gira hacia donde mira
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 550.f;
	GetCharacterMovement()->AirControl = 0.35f; // un poco mas de maniobra en el aire que el default (0.2)

	// Aceleracion/frenado mas altos que el default: arranca y frena rapido en vez de "patinar" en el piso
	GetCharacterMovement()->MaxAcceleration = 2048.0f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2048.0f;
	GetCharacterMovement()->GroundFriction = 8.0f;

	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	// Permite que este character pueda agacharse (Crouch()/UnCrouch())
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	// Velocidad al caminar agachado (menor a la velocidad normal de caminar)
	GetCharacterMovement()->MaxWalkSpeedCrouched = 150.0f; // ajustá el número a gusto

	// Camara de primera persona: colgada directo de la capsula, a la altura de los ojos.
	// Sin spring arm de por medio -> nada de distancia ni colision de camara que resolver.
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(GetCapsuleComponent());
	FollowCamera->SetRelativeLocation(FVector(0.0f, 0.0f, BaseEyeHeight));
	FollowCamera->bUsePawnControlRotation = true; // ahora la camara rota con el control directamente
	FollowCamera->FieldOfView = BaseFOV;

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

	// Primera persona: el dueno del pawn no ve su propio mesh de cuerpo entero
	// (SetOwnerNoSee lo oculta solo para la vista del dueno; en multiplayer otros
	// jugadores lo siguen viendo con normalidad)
	if (GetMesh())
	{
		GetMesh()->SetOwnerNoSee(true);
	}

	// Cargamos la sensibilidad guardada (si el jugador nunca abrio el menu de
	// settings todavia, no existe el slot y se queda con el default de 1.0)
	if (UGameplayStatics::DoesSaveGameExist(UNOVAPLAYSettingsSave::GetSlotName(), UNOVAPLAYSettingsSave::GetUserIndex()))
	{
		if (UNOVAPLAYSettingsSave* Save = Cast<UNOVAPLAYSettingsSave>(UGameplayStatics::LoadGameFromSlot(UNOVAPLAYSettingsSave::GetSlotName(), UNOVAPLAYSettingsSave::GetUserIndex())))
		{
			MouseSensitivity = Save->MouseSensitivity;
		}
	}

	if (HUDWidgetClass)
	{
		HUDWidgetInstance = CreateWidget<UNOVAPLAYHUDWidget>(GetWorld(), HUDWidgetClass);
		if (HUDWidgetInstance)
		{
			HUDWidgetInstance->AddToViewport();
		}
	}

	if (InventoryWidgetClass)
	{
		InventoryWidgetInstance = CreateWidget<UNOVAPLAYInventoryWidget>(GetWorld(), InventoryWidgetClass);
		if (InventoryWidgetInstance)
		{
			InventoryWidgetInstance->InitializeInventory(NOVAPLAYInventoryComponent);
			// Ojo: NO llamamos AddToViewport() acá — el panel arranca oculto,
			// recién se agrega a pantalla cuando el jugador lo abre con "I"
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Tick / Stamina

void ANOVAPLAYCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// El sprint solo "cuenta" yendo hacia adelante (o adelante+costado). Yendo en
	// reversa o solo de costado, aunque tengas Shift apretado, es como caminar normal
	// -- esto se calcula una sola vez acá porque stamina, velocidad, bob y FOV lo usan.
	bIsEffectivelySprinting = bIsSprinting && (LastForwardInputValue > 0.0f);

	UpdateStamina(DeltaTime);
	UpdateJumpGravity();
	UpdateJumpAssist(DeltaTime);
	UpdateMovementSpeed(DeltaTime);
	UpdateCrouchEyeHeight(DeltaTime);
	UpdateCameraFeel(DeltaTime);

	if (!bInventoryOpen)
	{
		PerformInteractionTrace();
	}
}

void ANOVAPLAYCharacter::UpdateStamina(float DeltaTime)
{
	const bool bIsGrounded = GetCharacterMovement()->IsMovingOnGround();

	if (bIsEffectivelySprinting && bIsGrounded && GetCharacterMovement()->Velocity.SizeSquared() > 0.0f)
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
// Salto: gravedad asimetrica + coyote time + jump buffer

void ANOVAPLAYCharacter::UpdateJumpGravity()
{
	// Mientras sube usa una gravedad, mientras cae otra mas fuerte -> el salto
	// se siente ágil arriba y "seco" al aterrizar, en vez de flotante y lento.
	if (GetCharacterMovement()->Velocity.Z < 0.0f)
	{
		GetCharacterMovement()->GravityScale = FallingGravityScale;
	}
	else
	{
		GetCharacterMovement()->GravityScale = RisingGravityScale;
	}
}

void ANOVAPLAYCharacter::UpdateJumpAssist(float DeltaTime)
{
	const bool bGroundedNow = GetCharacterMovement()->IsMovingOnGround();

	if (bGroundedNow)
	{
		TimeSinceLeftGround = 0.0f;
	}
	else
	{
		TimeSinceLeftGround += DeltaTime;
	}

	// Jump buffer: si el jugador apreto salto un instante antes de tocar el piso,
	// reintentamos consumirlo cada frame hasta que se cumpla o venza la ventana
	if (TimeSinceJumpPressed >= 0.0f)
	{
		TimeSinceJumpPressed += DeltaTime;

		if (TimeSinceJumpPressed > JumpBufferTime)
		{
			TimeSinceJumpPressed = -1.0f; // se vencio la ventana, se descarta
		}
		else
		{
			TryConsumeJump();
		}
	}
}

void ANOVAPLAYCharacter::TryConsumeJump()
{
	const bool bGrounded = GetCharacterMovement()->IsMovingOnGround();
	const bool bWithinCoyoteWindow = !bGrounded && (TimeSinceLeftGround <= CoyoteTime);

	if (!bGrounded && !bWithinCoyoteWindow)
	{
		return; // ni parado en el piso ni dentro de la ventana de coyote time: queda pendiente en el buffer
	}

	// Costo de stamina si saltas mientras corres (misma regla que antes)
	const bool bIsActuallyMoving = GetCharacterMovement()->Velocity.SizeSquared() > 0.0f;
	if (bIsEffectivelySprinting && bIsActuallyMoving)
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

	if (bGrounded)
	{
		Jump();
	}
	else
	{
		// Coyote jump: el motor ya te considera "en el aire", asi que el impulso
		// se aplica a mano en vez de con Jump() (que solo funciona parado en el piso)
		FVector NewVelocity = GetCharacterMovement()->Velocity;
		NewVelocity.Z = GetCharacterMovement()->JumpZVelocity;
		GetCharacterMovement()->Velocity = NewVelocity;
	}

	TimeSinceJumpPressed = -1.0f; // consumido: ya no queda pendiente
}

//////////////////////////////////////////////////////////////////////////
// Sprint: velocidad interpolada (transicion suave en vez de instantanea)

void ANOVAPLAYCharacter::UpdateMovementSpeed(float DeltaTime)
{
	const float TargetSpeed = bIsEffectivelySprinting ? SprintSpeed : WalkSpeed;
	GetCharacterMovement()->MaxWalkSpeed = FMath::FInterpTo(GetCharacterMovement()->MaxWalkSpeed, TargetSpeed, DeltaTime, SpeedInterpSpeed);
}

//////////////////////////////////////////////////////////////////////////
// Camara: head bob al caminar/correr + dip al aterrizar + FOV al sprintar

void ANOVAPLAYCharacter::UpdateCrouchEyeHeight(float DeltaTime)
{
	// La capsula ya salto a su altura final ESTE MISMO FRAME (el motor no la interpola,
	// es instantanea). Por eso la parte que suaviza tiene que ser el DROP NETO DESEADO
	// (0 -> CrouchCameraDrop), no el offset de compensacion -- ese siempre tiene que
	// matchear exactamente lo que la capsula ya hizo, sin ningun frame de demora,
	// o aparece el rebote (se ve la caida completa un instante y despues "sube" de vuelta).
	const float TargetNetDrop = bIsCrouched ? CrouchCameraDrop : 0.0f;
	CrouchNetDropCurrent = FMath::FInterpTo(CrouchNetDropCurrent, TargetNetDrop, DeltaTime, CrouchTransitionSpeed);

	// Cuanto salto la capsula, calculado en vivo cada frame (0 si esta parado, el valor
	// completo si esta agachado -- nunca a medio camino, porque la capsula no interpola)
	const float CrouchedHalfHeight = GetCharacterMovement()->CrouchedHalfHeight;
	const float AutomaticCapsuleDrop = bIsCrouched ? (StandingCapsuleHalfHeight - CrouchedHalfHeight) : 0.0f;

	// Cancela la caida automatica por completo, y le resta el drop neto (que si es suave)
	CrouchEyeHeightOffset = AutomaticCapsuleDrop - CrouchNetDropCurrent;
}

void ANOVAPLAYCharacter::UpdateCameraFeel(float DeltaTime)
{
	if (!FollowCamera)
	{
		return;
	}

	// --- Head bob: solo si esta en el piso y realmente moviendose ---
	const bool bGrounded = GetCharacterMovement()->IsMovingOnGround();
	const float Speed = GetCharacterMovement()->Velocity.Size2D();
	const bool bIsMoving = bGrounded && Speed > 10.0f;

	float BobOffset = 0.0f;
	if (bIsMoving)
	{
		// La frecuencia escala con la velocidad: pasos mas rapidos al correr que al caminar
		BobTimeAccumulator += DeltaTime * BobFrequency * FMath::Max(Speed / WalkSpeed, 0.5f);
		const float Amplitude = bIsEffectivelySprinting ? BobAmplitudeSprint : BobAmplitudeWalk;
		BobOffset = FMath::Sin(BobTimeAccumulator) * Amplitude;
	}
	else
	{
		BobTimeAccumulator = 0.0f; // parado o en el aire: sin balanceo
	}

	// --- Dip de aterrizaje: se recupera solo, cada frame un poco mas cerca de 0 ---
	CurrentLandingOffset = FMath::FInterpTo(CurrentLandingOffset, 0.0f, DeltaTime, LandingRecoverySpeed);

	const FVector TargetLocation(0.0f, 0.0f, BaseEyeHeight + BobOffset + CurrentLandingOffset + CrouchEyeHeightOffset);
	FollowCamera->SetRelativeLocation(TargetLocation);

	// --- FOV kick al sprintar ---
	const float TargetFOV = bIsEffectivelySprinting ? SprintFOV : BaseFOV;
	FollowCamera->FieldOfView = FMath::FInterpTo(FollowCamera->FieldOfView, TargetFOV, DeltaTime, FOVInterpSpeed);
}

void ANOVAPLAYCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	// La camara se hunde de golpe y UpdateCameraFeel la recupera sola, frame a frame
	CurrentLandingOffset = -LandingDipStrength;
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
	}
}
void ANOVAPLAYCharacter::StopSprint()
{
	bIsSprinting = false;
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

	// Guardamos el momento de la pulsacion; si no se puede saltar ya mismo (ni con
	// coyote time), UpdateJumpAssist va a seguir reintentando hasta JumpBufferTime
	TimeSinceJumpPressed = 0.0f;
	TryConsumeJump();
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

	AActor* NewFocus = nullptr;

	if (bHit && Hit.GetActor() && Hit.GetActor()->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
	{
		NewFocus = Hit.GetActor();
	}

	if (NewFocus != FocusedInteractable)
	{
		FocusedInteractable = NewFocus;

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

		if (InventoryWidgetInstance && !InventoryWidgetInstance->IsInViewport())
		{
			InventoryWidgetInstance->AddToViewport();
		}
	}
	else
	{
		PC->SetIgnoreMoveInput(false);
		PC->SetIgnoreLookInput(false);

		PC->SetInputMode(FInputModeGameOnly());
		PC->bShowMouseCursor = false;

		if (InventoryWidgetInstance && InventoryWidgetInstance->IsInViewport())
		{
			InventoryWidgetInstance->RemoveFromParent();
		}
	}

	if (HUDWidgetInstance)
	{
		HUDWidgetInstance->SetInventoryVisible(bInventoryOpen);
	}
}

//////////////////////////////////////////////////////////////////////////
// Pause Menu

void ANOVAPLAYCharacter::TogglePauseMenu()
{
	// Si el inventario esta abierto, Esc lo cierra a ese primero en vez de
	// abrir la pausa encima -- evita que se pisen los dos bloqueos de input
	if (bInventoryOpen && !bPauseMenuOpen)
	{
		ToggleInventory();
		return;
	}

	bPauseMenuOpen = !bPauseMenuOpen;

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		return;
	}

	if (bPauseMenuOpen)
	{
		PC->SetIgnoreMoveInput(true);
		PC->SetIgnoreLookInput(true);

		PC->SetInputMode(FInputModeGameAndUI());
		PC->bShowMouseCursor = true;

		UGameplayStatics::SetGamePaused(GetWorld(), true);

		if (PauseMenuWidgetClass && !PauseMenuWidgetInstance)
		{
			PauseMenuWidgetInstance = CreateWidget<UNOVAPLAYPauseMenuWidget>(GetWorld(), PauseMenuWidgetClass);
		}

		if (PauseMenuWidgetInstance && !PauseMenuWidgetInstance->IsInViewport())
		{
			PauseMenuWidgetInstance->AddToViewport(10); // Z-order alto: siempre por encima del HUD/inventario
		}
	}
	else
	{
		ClosePauseMenu();
	}
}

void ANOVAPLAYCharacter::ClosePauseMenu()
{
	bPauseMenuOpen = false;

	UGameplayStatics::SetGamePaused(GetWorld(), false);

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC)
	{
		PC->SetIgnoreMoveInput(false);
		PC->SetIgnoreLookInput(false);

		PC->SetInputMode(FInputModeGameOnly());
		PC->bShowMouseCursor = false;
	}

	if (PauseMenuWidgetInstance && PauseMenuWidgetInstance->IsInViewport())
	{
		PauseMenuWidgetInstance->RemoveFromParent();
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
	PlayerInputComponent->BindAxis("Turn", this, &ANOVAPLAYCharacter::Turn);
	PlayerInputComponent->BindAxis("TurnRate", this, &ANOVAPLAYCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &ANOVAPLAYCharacter::LookUp);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ANOVAPLAYCharacter::LookUpAtRate);

	// Sprint y Crouch
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ANOVAPLAYCharacter::StartSprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ANOVAPLAYCharacter::StopSprint);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ANOVAPLAYCharacter::ToggleCrouch);
	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &ANOVAPLAYCharacter::Interact);

	// Abrir inventario
	PlayerInputComponent->BindAction("ToggleInventory", IE_Pressed, this, &ANOVAPLAYCharacter::ToggleInventory);

	// Menu de pausa (Esc)
	PlayerInputComponent->BindAction("TogglePause", IE_Pressed, this, &ANOVAPLAYCharacter::TogglePauseMenu);
}

void ANOVAPLAYCharacter::Turn(float Value)
{
	AddControllerYawInput(Value * MouseSensitivity);
}

void ANOVAPLAYCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value * MouseSensitivity);
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
	LastForwardInputValue = Value;

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
