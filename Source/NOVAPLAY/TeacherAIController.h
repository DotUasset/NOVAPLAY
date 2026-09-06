// NOVAPLAY - AIController del docente: corre el Behavior Tree y detecta al jugador por vision

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
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

	// Nombres de las keys del Blackboard, expuestos para no hardcodear strings en varios lugares.
	// Deben coincidir EXACTAMENTE con los nombres de key que crees en BB_Teacher.
	static const FName BBKey_TargetActor;
	static const FName BBKey_CanSeePlayer;
	static const FName BBKey_LastKnownLocation;
	static const FName BBKey_PatrolPoint;

protected:
	virtual void OnPossess(APawn* InPawn) override;

	UPROPERTY(VisibleAnywhere, Category = "AI", meta = (AllowPrivateAccess = "true"))
	UAIPerceptionComponent* AIPerceptionComp;

	// EditDefaultsOnly + Instanced (no VisibleAnywhere): asi el editor lo muestra como un
	// panel expandible dentro del Class Defaults de BP_TeacherAIController, con SightRadius /
	// LoseSightRadius / PeripheralVisionAngleDegrees editables ahi mismo, sin recompilar C++
	// cada vez que quieras ajustar la vision. (Sin "Instanced" el editor solo muestra un
	// selector de objeto vacio y grisado, que es justo lo que viste antes.)
	UPROPERTY(EditDefaultsOnly, Instanced, Category = "AI|Vision", meta = (AllowPrivateAccess = "true"))
	class UAISenseConfig_Sight* SightConfig;

	UFUNCTION()
	void OnTargetPerceived(AActor* Actor, FAIStimulus Stimulus);

	// Guarda si en el frame anterior lo estaba viendo, para disparar el sonido/debug de
	// "detectado" solo en el instante exacto en que pasa de false a true (no en cada tick).
	bool bWasSeenLastFrame = false;
};
