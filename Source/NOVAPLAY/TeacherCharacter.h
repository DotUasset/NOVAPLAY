// NOVAPLAY - Personaje enemigo (docente) que patrulla, persigue y atrapa al jugador.
//
// Esta clase se encarga SOLO de la logica dura (velocidad, patrulla, deteccion de contacto).
// Sonido, mensajes de debug y la pantalla de "atrapado" NO se manejan aca: se disparan como
// eventos de Blueprint (ver la seccion "EVENTOS PARA BLUEPRINT" mas abajo) para que los
// implementes en el Event Graph de BP_Teacher, con total libertad de diseno sin recompilar C++.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TeacherCharacter.generated.h"

class USphereComponent;
class ANOVAPLAYCharacter;

// Estado de movimiento actual del docente. Se lo pasamos a Blueprint (OnMovementStateChanged)
// para que decida que sonido loopear y que mensaje de debug imprimir en cada caso.
UENUM(BlueprintType)
enum class ETeacherMovementState : uint8
{
	Patrolling    UMETA(DisplayName = "Patrullando"),
	Investigating UMETA(DisplayName = "Investigando (ultima posicion vista)"),
	Chasing       UMETA(DisplayName = "Persiguiendo")
};

// Nivel de conciencia del docente sobre el jugador. Independiente de ETeacherMovementState:
// esto es sobre CUANTO sabe/sospecha el docente, no sobre a que velocidad se mueve.
// Pensado para mostrar un icono "?" (Suspicious) o "!" (Alerted) sobre la cabeza del docente
// desde el Event Graph de BP_Teacher (ver OnAwarenessChanged).
UENUM(BlueprintType)
enum class ETeacherAwarenessState : uint8
{
	// No sospecha nada: no ve ni escucho nada relevante ultimamente.
	Unaware    UMETA(DisplayName = "Sin sospechas"),
	// Escucho un ruido o tiene al jugador a la vista pero todavia no el tiempo suficiente
	// como para estar seguro (ver TimeToDetectSight en ATeacherAIController). Va a investigar.
	Suspicious UMETA(DisplayName = "Sospechando"),
	// Confirmo al jugador: lo esta persiguiendo activamente.
	Alerted    UMETA(DisplayName = "Alertado")
};

UCLASS()
class NOVAPLAY_API ATeacherCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ATeacherCharacter();

	// ----- VELOCIDAD -----

	// Velocidad de movimiento mientras patrulla
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Movimiento")
	float PatrolSpeed = 260.0f;

	// Velocidad de movimiento mientras persigue/investiga al jugador
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Movimiento")
	float ChaseSpeed = 650.0f;

	// Cambia la velocidad segun el estado. La llama BTService_UpdateMovementState cada vez
	// que el Behavior Tree reevalua si esta persiguiendo/investigando o patrullando.
	UFUNCTION(BlueprintCallable, Category = "AI|Movimiento")
	void SetMovementState(bool bIsChasingOrInvestigating);

	// Dispara OnMovementStateChanged solo si el estado cambio desde la ultima vez (evita
	// spamear el evento en cada tick del Service). La llama BTService_UpdateMovementState
	// junto con SetMovementState, pasandole el estado especifico (Patrol/Investigate/Chase).
	UFUNCTION(BlueprintCallable, Category = "AI|Movimiento")
	void NotifyMovementState(ETeacherMovementState NewState);

	// Igual que NotifyMovementState pero para el nivel de sospecha (Unaware/Suspicious/Alerted).
	// La llama ATeacherAIController cada vez que cambia su estado interno de deteccion.
	UFUNCTION(BlueprintCallable, Category = "AI|Movimiento")
	void NotifyAwarenessState(ETeacherAwarenessState NewState);

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

	// ----- CAPTURA -----

	// Esfera de deteccion de contacto: si el jugador entra aca, se lo considera atrapado
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI", meta = (AllowPrivateAccess = "true"))
	USphereComponent* CatchSphere;

	// Radio de la esfera de captura (cm). Ajustalo segun la escala del mesh del docente.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	float CatchRadius = 80.0f;

	// Tiempo (seg) entre que el docente toca al jugador y se dispara OnCaughtSequenceComplete.
	// Da margen para reproducir el sonido/animacion de "atrapado" (OnPlayerCaught) antes de
	// bloquear el input y mostrar la pantalla de derrota.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	float CatchDelay = 0.5f;

	// ----- EVENTOS PARA BLUEPRINT -----
	// Implementa estos eventos en el Event Graph de BP_Teacher. Ahi es donde va toda la logica
	// de sonido y de "Print String" de debug: esta clase en C++ solo avisa CUANDO pasa cada cosa.

	// Se dispara cada vez que cambia el estado de movimiento (Patrullando/Investigando/
	// Persiguiendo). Enganchalo a un Switch on ETeacherMovementState en el Event Graph para
	// imprimir el mensaje de debug correspondiente y arrancar/parar el loop de sonido que
	// corresponda (podes agregar tu propio Audio Component en los Components de BP_Teacher).
	UFUNCTION(BlueprintImplementableEvent, Category = "AI|Eventos")
	void OnMovementStateChanged(ETeacherMovementState NewState);

	// Se dispara cada vez que cambia el nivel de sospecha (Unaware/Suspicious/Alerted).
	// Ideal para mostrar/ocultar un icono "?" o "!" sobre la cabeza del docente.
	UFUNCTION(BlueprintImplementableEvent, Category = "AI|Eventos")
	void OnAwarenessChanged(ETeacherAwarenessState NewState);

	// Se dispara en el instante exacto en que el docente CONFIRMA al jugador (pasa a Alerted,
	// tras el tiempo de deteccion). Ideal para el sonido/print de "te vi" y animaciones.
	UFUNCTION(BlueprintImplementableEvent, Category = "AI|Eventos")
	void OnDetectedPlayer();

	// Se dispara cuando el docente escucha un ruido del jugador (paso corriendo, aterrizaje,
	// etc.) sin haberlo visto todavia. Util para un sonido corto de "eh?" y para que Blueprint
	// haga que el docente gire la cabeza hacia RUMBO, sin necesidad de tocar el Behavior Tree.
	UFUNCTION(BlueprintImplementableEvent, Category = "AI|Eventos")
	void OnHeardNoise(FVector NoiseLocation, float Loudness);

	// Se dispara apenas el docente toca al jugador (antes del delay). Usalo para el sonido/
	// print de "te atrape" y para reproducir una animacion de captura.
	UFUNCTION(BlueprintImplementableEvent, Category = "AI|Eventos")
	void OnPlayerCaught(ANOVAPLAYCharacter* CaughtPlayer);

	// Se dispara CatchDelay segundos despues de OnPlayerCaught. Es el momento de bloquear el
	// input y mostrar la pantalla de derrota: castea CaughtPlayer al Blueprint del jugador
	// (ThirdPersonCharacter) y llama a su funcion "Show Caught Screen".
	UFUNCTION(BlueprintImplementableEvent, Category = "AI|Eventos")
	void OnCaughtSequenceComplete(ANOVAPLAYCharacter* CaughtPlayer);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void OnCatchOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	// Llamada tras CatchDelay: dispara OnCaughtSequenceComplete
	void HandleCatchDelayFinished();

	FTimerHandle CatchTimerHandle;

	// Evita disparar el overlap varias veces mientras el timer de captura esta corriendo
	bool bHasCaughtPlayer = false;

	// Jugador atrapado, guardado para pasarlo a OnCaughtSequenceComplete despues del delay
	UPROPERTY()
	ANOVAPLAYCharacter* CaughtPlayerRef = nullptr;

	// Indice del ultimo punto de patrulla devuelto por GetNextPatrolPoint
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "AI|Patrulla", Transient)
	int32 CurrentPatrolIndex = -1;

	// Ultimo estado de movimiento notificado a Blueprint, para no repetir el mismo evento
	// en cada tick del BTService (dedupe interno, no es un sistema de debug).
	ETeacherMovementState LastNotifiedMovementState = ETeacherMovementState::Patrolling;

	// true la primera vez que se notifica el estado (fuerza el primer OnMovementStateChanged
	// aunque el valor por default coincida con Patrolling)
	bool bHasNotifiedMovementStateOnce = false;

	// Mismo patron que arriba pero para el nivel de sospecha
	ETeacherAwarenessState LastNotifiedAwarenessState = ETeacherAwarenessState::Unaware;
	bool bHasNotifiedAwarenessStateOnce = false;
};
