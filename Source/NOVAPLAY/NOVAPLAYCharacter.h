// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NOVAPLAYInventoryComponent.h"
#include "NOVAPLAYCharacter.generated.h"

UCLASS(config=Game)
class ANOVAPLAYCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camara de primera persona, montada directo en la capsula */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
public:
	ANOVAPLAYCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	// Getter para que la UI (Blueprint) pueda leer el % de stamina
	UFUNCTION(BlueprintCallable, Category = "Stamina")
	float GetStaminaPercent() const { return CurrentStamina / MaxStamina; }

	// Se llama al apretar E: interactúa con lo que el personaje tenga enfocado
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void Interact();

	// Abre/cierra el panel de inventario y bloquea/desbloquea el movimiento mientras está abierto
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void ToggleInventory();

	// override necesario para procesar UpdateStamina cada frame
	virtual void Tick(float DeltaTime) override;

protected:

	// se llama una vez al arrancar el juego: acá creamos el widget del HUD
	virtual void BeginPlay() override;

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	// ----- STAMINA -----
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stamina")
	float MaxStamina = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Stamina")
	float CurrentStamina;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stamina")
	float StaminaDrainRate = 20.0f; // por segundo, mientras corre

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stamina")
	float StaminaRegenRate = 5.0f; // por segundo, en reposo

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stamina")
	float MinStaminaToSprint = 10.0f; // no te deja arrancar a correr si tenés menos que esto

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stamina")
	float JumpStaminaCost = 15.0f; // costo extra si saltás mientras corrés

	// ----- SALTO: gravedad asimetrica (sube mas lento, cae mas rapido) -----
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Jump")
	float RisingGravityScale = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Jump")
	float FallingGravityScale = 2.5f;

	// ----- SALTO: coyote time + jump buffer -----
	// Cuanto tiempo (seg) despues de dejar el piso (sin haber saltado) todavia te deja saltar
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Jump")
	float CoyoteTime = 0.15f;

	// Cuanto tiempo (seg) se "recuerda" una pulsacion de salto hecha justo antes de aterrizar
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Jump")
	float JumpBufferTime = 0.15f;

	float TimeSinceLeftGround = 0.0f;
	float TimeSinceJumpPressed = -1.0f; // -1 = no hay ningun salto pendiente en el buffer

	void UpdateJumpGravity();
	void UpdateJumpAssist(float DeltaTime);
	void TryConsumeJump();

	// ----- SPRINT: transicion de velocidad suave (interpolada, no instantanea) -----
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Sprint")
	float WalkSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Sprint")
	float SprintSpeed = 600.0f;

	// Que tan rapido interpola entre WalkSpeed y SprintSpeed (mas alto = transicion mas brusca)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Sprint")
	float SpeedInterpSpeed = 6.0f;

	void UpdateMovementSpeed(float DeltaTime);

	// ----- CAMARA: head bob, dip de aterrizaje, FOV al sprintar -----
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Camera Feel")
	float BobFrequency = 10.0f; // que tan rapido oscila la camara al caminar/correr

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Camera Feel")
	float BobAmplitudeWalk = 1.2f; // oscilacion vertical caminando (cm)

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Camera Feel")
	float BobAmplitudeSprint = 2.2f; // oscilacion vertical corriendo (cm)

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Camera Feel")
	float LandingDipStrength = 8.0f; // cuanto se hunde la camara al aterrizar (cm)

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Camera Feel")
	float LandingRecoverySpeed = 8.0f; // que tan rapido vuelve a su posicion normal

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Camera Feel")
	float BaseFOV = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Camera Feel")
	float SprintFOV = 98.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Camera Feel")
	float FOVInterpSpeed = 8.0f;

	float BobTimeAccumulator = 0.0f;
	float CurrentLandingOffset = 0.0f;

	void UpdateCameraFeel(float DeltaTime);

	virtual void Landed(const FHitResult& Hit) override;

	bool bIsSprinting = false;

	// ----- FUNCIONES DE SPRINT / CROUCH -----
	void StartSprint();
	void HandleJumpInput();
	void StopSprint();
	void ToggleCrouch();

	// ----- INTERACCIÓN -----
	// Distancia máxima del trace de interacción (cm)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	float InteractionDistance = 600.0f;

	// Radio del sphere trace de interacción (cm). Más alto = más tolerante al apuntar.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	float InteractionTraceRadius = 20.0f;

	// Actor que el personaje tiene actualmente enfocado (o nullptr si no hay ninguno)
	UPROPERTY()
	AActor* FocusedInteractable = nullptr;

	// Hace el trace desde la cámara cada frame y actualiza FocusedInteractable + el prompt del HUD
	void PerformInteractionTrace();

	// se llama todos los frames para manejar el drain/regen de stamina
	void UpdateStamina(float DeltaTime);

	// ----- HUD -----
	// Clase del widget a usar. Se asigna desde el Blueprint del personaje (WBP_HUD)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HUD")
	TSubclassOf<class UNOVAPLAYHUDWidget> HUDWidgetClass;

	// Instancia real creada en BeginPlay
	UPROPERTY()
	class UNOVAPLAYHUDWidget* HUDWidgetInstance;

	// Componente de inventario del personaje
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	class UNOVAPLAYInventoryComponent* NOVAPLAYInventoryComponent;

	// true mientras el panel de inventario está abierto en pantalla
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	bool bInventoryOpen = false;

	// Clase del widget de inventario (asignás WBP_Inventory desde el Character Blueprint)
	UPROPERTY(EditAnywhere, Category = "Inventory")
	TSubclassOf<class UNOVAPLAYInventoryWidget> InventoryWidgetClass;

	UPROPERTY()
	class UNOVAPLAYInventoryWidget* InventoryWidgetInstance;

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

public:
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};
