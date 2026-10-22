// NOVAPLAY - AIController del docente: corre el Behavior Tree y detecta al jugador por
// vision Y por oido, con deteccion gradual (no instantanea) en vez de un simple bool.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "TeacherCharacter.h" // ETeacherAwarenessState
#include "TeacherAIController.generated.h"

UCLASS()
class NOVAPLAY_API ATeacherAIController : public AAIController
{
	GENERATED_BODY()

public:
	ATeacherAIController();

	// Asset del Behavior Tree (BT_Teacher). Se asigna en el editor, en el Blueprint del docente
	// o en el default de esta clase.
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	class UBehaviorTree* BehaviorTreeAsset;

	// Asset del Blackboard (BB_Teacher)
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	class UBlackboardData* BlackboardAsset;

	// Si el jugador esta agachado, multiplicamos el radio de vision por este valor para decidir
	// si realmente lo detecta. Ej: 0.6 significa que agachado hay que estar a menos del 60%
	// de la distancia normal para ser visto -> el jugador se puede "esconder" alejandose y
	// agachandose, ademas de taparse detras de geometria (eso ya lo bloquea el trace de vision solo).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Sigilo")
	float CrouchSightMultiplier = 0.6f;

	// ----- DETECCION GRADUAL (VISION) -----
	// En vez de alertarse apenas hay UN frame de linea de vista, el docente necesita verlo
	// de forma continua durante este tiempo (segundos) antes de confirmarlo y perseguirlo
	// de verdad. Mientras "junta sospecha" el estado es Suspicious (ver ETeacherAwarenessState),
	// no Alerted: da tiempo a reaccionar si el jugador se cruza un instante en su campo de vision.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Deteccion", meta = (ClampMin = "0.0"))
	float TimeToDetectSight = 0.8f;

	// Si el jugador esta MUY cerca (dentro de este porcentaje del SightRadius), la deteccion
	// es mas rapida (se multiplica TimeToDetectSight por CloseRangeDetectionSpeedup). Simula que
	// es mucho mas dificil pasar desapercibido a corta distancia.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Deteccion", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CloseRangeThresholdPct = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Deteccion", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CloseRangeDetectionSpeedup = 0.4f;

	// Que tan rapido se "olvida" la sospecha acumulada (unidades de TimeToDetectSight por
	// segundo) cuando el docente deja de verlo. Mas alto = mas facil despistarlo escondiendose
	// un instante; con 0 nunca decae y cualquier suma de vistazos cortos termina alertandolo.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Deteccion", meta = (ClampMin = "0.0"))
	float SightSuspicionDecayRate = 1.2f;

	// Cada cuanto (seg) se re-evalua el acumulador de sospecha/decaimiento. No hace falta
	// que sea muy chico: 0.1-0.15 alcanza para que se sienta responsivo.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Deteccion", meta = (ClampMin = "0.02"))
	float DetectionTickInterval = 0.1f;

	// ----- OIDO -----
	// Radio (cm) dentro del cual el docente puede escuchar un ruido AUNQUE haya paredes de por
	// medio (atenuado). Los ruidos del jugador se emiten con NOVAPLAYCharacter::EmitAINoise.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Oido", meta = (ClampMin = "0.0"))
	float HearingRange = 1600.0f;

	// Radio (cm) con linea de sonido libre (sin geometria de por medio): mas generoso que
	// HearingRange porque no hay nada bloqueando.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Oido", meta = (ClampMin = "0.0"))
	float LineOfSightHearingRange = 3000.0f;

	// Un ruido escuchado (sin haber visto al jugador) no lo persigue de una: lo manda a
	// investigar el punto donde sono. Este es cuanto "empuja" ese unico ruido al acumulador
	// de sospecha (0 = solo investiga, nunca alerta por sonido solo; valores altos permiten
	// que un ruido MUY fuerte, ej. sprint pegado al docente, lo alerte directamente).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Oido", meta = (ClampMin = "0.0"))
	float NoiseSuspicionContribution = 0.35f;

	// ----- OLVIDO / VUELTA A PATRULLA -----
	// Si el docente esta investigando (perdio de vista o escucho algo, pero no confirma nada
	// nuevo) durante mas de este tiempo (segundos), se rinde y vuelve a patrullar solo:
	// limpia el Blackboard el mismo, sin depender de que el Behavior Tree lo haga con un
	// Wait + Clear Blackboard Value ajustado "a ojo". Es una red de seguridad para que el
	// docente nunca se quede persiguiendo un fantasma para siempre.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Deteccion", meta = (ClampMin = "0.0"))
	float ForgetInvestigationAfterSeconds = 10.0f;

	// ----- DEBUG -----
	// Dibuja en el viewport (solo en editor/PIE, nunca en Shipping) el radio de vision, el cono
	// de vision y el radio de oido del docente, con un color segun su estado actual
	// (verde = Unaware, amarillo = Suspicious, rojo = Alerted). Muy util para calibrar los
	// valores de arriba sin adivinar.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Debug")
	bool bDrawDebugPerception = false;

	// Nombres de las keys del Blackboard, expuestos para no hardcodear strings en varios lugares.
	// Deben coincidir EXACTAMENTE con los nombres de key que crees en BB_Teacher.
	// NOTA: seguimos usando exactamente las mismas 4 keys de siempre. La deteccion gradual y el
	// oido viven como estado interno de esta clase, no requieren agregar keys nuevas al Blackboard.
	static const FName BBKey_TargetActor;
	static const FName BBKey_CanSeePlayer;
	static const FName BBKey_LastKnownLocation;
	static const FName BBKey_PatrolPoint;

	// Nivel de sospecha actual, de 0 (nada) a 1 (a punto de confirmar), pensado para un
	// medidor en el HUD/debug. No es la fuente de verdad del Behavior Tree (esa sigue siendo
	// bCanSeePlayer/LastKnownLocation en el Blackboard); es solo para UI y ajuste fino.
	UFUNCTION(BlueprintPure, Category = "AI|Deteccion")
	float GetSuspicionRatio() const { return CurrentTimeToDetect > 0.0f ? FMath::Clamp(SightSuspicionTime / CurrentTimeToDetect, 0.0f, 1.0f) : (SightSuspicionTime > 0.0f ? 1.0f : 0.0f); }

	UFUNCTION(BlueprintPure, Category = "AI|Deteccion")
	ETeacherAwarenessState GetAwarenessState() const { return CurrentAwareness; }

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, Category = "AI", meta = (AllowPrivateAccess = "true"))
	UAIPerceptionComponent* AIPerceptionComp;

	// EditDefaultsOnly + Instanced (no VisibleAnywhere): asi el editor lo muestra como un
	// panel expandible dentro del Class Defaults de BP_TeacherAIController, con SightRadius /
	// LoseSightRadius / PeripheralVisionAngleDegrees editables ahi mismo, sin recompilar C++
	// cada vez que quieras ajustar la vision.
	UPROPERTY(EditDefaultsOnly, Instanced, Category = "AI|Vision", meta = (AllowPrivateAccess = "true"))
	class UAISenseConfig_Sight* SightConfig;

	// Mismo patron para el oido: editable en Class Defaults sin recompilar.
	UPROPERTY(EditDefaultsOnly, Instanced, Category = "AI|Oido", meta = (AllowPrivateAccess = "true"))
	class UAISenseConfig_Hearing* HearingConfig;

	UFUNCTION()
	void OnTargetPerceived(AActor* Actor, FAIStimulus Stimulus);

	// Aplica la logica de un estimulo de VISION (separado de OnTargetPerceived para que este
	// no sea un bloque gigante con ifs anidados). Devuelve true si el jugador esta efectivamente
	// visible este frame (ya despues de aplicar el multiplicador de agachado).
	bool ProcessSightStimulus(const class ANOVAPLAYCharacter* PlayerChar, const FAIStimulus& Stimulus);

	// Aplica la logica de un estimulo de OIDO: manda a investigar el punto del ruido y opcionalmente
	// suma sospecha si el ruido fue fuerte.
	void ProcessHearingStimulus(AActor* Actor, const FAIStimulus& Stimulus);

	// Se llama cada DetectionTickInterval segundos: hace crecer/decaer SightSuspicionTime,
	// decide si hay que pasar a Alerted, y aplica el "olvido" de investigacion tras
	// ForgetInvestigationAfterSeconds sin novedades.
	void UpdateDetectionState();
	FTimerHandle DetectionTimerHandle;

	// true si el trace de AISense_Sight esta viendo al jugador ESTE frame (antes de aplicar
	// el tiempo de confirmacion). Lo actualiza OnTargetPerceived, lo consume UpdateDetectionState.
	bool bRawSightThisFrame = false;
	FVector LastRawSightLocation = FVector::ZeroVector;
	TWeakObjectPtr<AActor> LastSensedPlayer;

	// Segundos acumulados de vision continua (con decaimiento cuando no lo ve). Cuando llega
	// a TimeToDetectSight, se confirma la deteccion (Alerted).
	float SightSuspicionTime = 0.0f;

	// Tiempo de confirmacion vigente (TimeToDetectSight, ya reducido si el jugador esta muy cerca).
	// Se guarda para que GetSuspicionRatio llegue a 1.0 tambien en la deteccion a corta distancia.
	float CurrentTimeToDetect = 0.0f;

	// Segundos transcurridos desde la ultima vez que hubo novedades (vision o sonido) mientras
	// se esta investigando/persiguiendo. Al superar ForgetInvestigationAfterSeconds, se limpia
	// todo y se vuelve a patrullar.
	float TimeSinceLastEvidence = 0.0f;

	// true mientras el docente esta Alerted (persiguiendo de verdad, ya confirmado)
	bool bIsAlerted = false;

	ETeacherAwarenessState CurrentAwareness = ETeacherAwarenessState::Unaware;

	// Cambia CurrentAwareness, notifica al Pawn (para el icono sobre la cabeza) y actualiza
	// el Blackboard segun corresponda. Centraliza todas las transiciones de estado en un
	// solo lugar en vez de repetir la logica en cada evento de percepcion.
	void SetAwarenessState(ETeacherAwarenessState NewState);
};
