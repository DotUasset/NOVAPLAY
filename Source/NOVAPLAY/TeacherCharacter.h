// NOVAPLAY - Personaje enemigo (docente) que patrulla, persigue y atrapa al jugador

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TeacherCharacter.generated.h"

class ATargetPoint;

UCLASS()
class NOVAPLAY_API ATeacherCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ATeacherCharacter();

	virtual void Tick(float DeltaTime) override;

	// ----- VELOCIDAD -----

	// Velocidad de movimiento mientras patrulla
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Movimiento")
	float PatrolSpeed = 260.0f;

	// Velocidad de movimiento mientras persigue al jugador
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Movimiento")
	float ChaseSpeed = 650.0f;

	// Que tan rapido gira (grados/seg) mientras patrulla. Mas bajo = giro mas suave/lento.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Movimiento")
	float PatrolRotationRate = 220.0f;

	// Que tan rapido gira (grados/seg) mientras persigue. Mas alto = reacciona mas rapido al jugador.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Movimiento")
	float ChaseRotationRate = 420.0f;

	// Cambia velocidad Y velocidad de giro segun el estado. La llama el BTService_UpdateMovementState
	// cada vez que el Behavior Tree reevalua si esta persiguiendo o patrullando.
	UFUNCTION(BlueprintCallable, Category = "AI|Movimiento")
	void SetMovementState(bool bIsChasing);

	// ----- SALTO AUTOMATICO -----

	// Si esta activo, el docente salta solo cuando queda "trabado" contra un obstaculo bajo
	// (cordones, escalones, props chicos) en vez de quedarse empujando la pared para siempre.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Salto")
	bool bCanAutoJump = true;

	// Velocidad horizontal (cm/s) por debajo de la cual se considera "trabado" si esta intentando moverse
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Salto")
	float StuckSpeedThreshold = 30.0f;

	// Cuanto tiempo (seg) tiene que estar trabado antes de saltar
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Salto")
	float StuckTimeToJump = 0.35f;

	// ----- PATRULLA -----

	// Puntos de patrulla del docente. Arrastra actores "Target Point" (o cualquier Actor) desde
	// el nivel a este array, en el orden en que queres que los recorra. Si lo dejas vacio,
	// el docente elige puntos aleatorios navegables alrededor de su posicion inicial.
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "AI|Patrulla")
	TArray<AActor*> PatrolPoints;

	// true = recorre los puntos en orden (1,2,3,1,2,3...). false = elige uno al azar cada vez.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Patrulla")
	bool bPatrolInOrder = true;

	// Devuelve el siguiente punto de patrulla segun PatrolPoints/bPatrolInOrder.
	// La llama UBTService_FindPatrolPoint. Devuelve nullptr si PatrolPoints esta vacio
	// (en ese caso el Service recurre a un punto aleatorio del NavMesh).
	UFUNCTION(BlueprintCallable, Category = "AI|Patrulla")
	AActor* GetNextPatrolPoint();

	// Esfera de deteccion de contacto: si el jugador entra aca, se lo considera atrapado
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI", meta = (AllowPrivateAccess = "true"))
	class USphereComponent* CatchSphere;

	// Radio de la esfera de captura (cm). Ajustalo segun la escala del mesh del docente.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	float CatchRadius = 80.0f;

	// Tiempo (seg) entre que el docente toca al jugador y se dispara el reinicio del nivel.
	// Da margen para reproducir un sonido/animacion de "atrapado" antes de reiniciar.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	float CatchDelay = 0.5f;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnCatchOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	// Llamada tras CatchDelay: avisa al GameMode que el jugador fue atrapado
	void NotifyPlayerCaught();

	// Revisa cada frame si el docente esta "empujando" un obstaculo sin avanzar y, de ser asi,
	// lo hace saltar.
	void UpdateAutoJump(float DeltaTime);

	FTimerHandle CatchTimerHandle;

	// Evita disparar el overlap varias veces mientras el timer de captura esta corriendo
	bool bHasCaughtPlayer = false;

	// Acumulador de tiempo "trabado", usado por UpdateAutoJump
	float StuckTimer = 0.0f;

	// Indice del ultimo punto de patrulla devuelto por GetNextPatrolPoint
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "AI|Patrulla", Transient)
	int32 CurrentPatrolIndex = -1;
};
