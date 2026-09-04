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
	// si realmente lo detecta. Ej: 0.45 significa que agachado hay que estar a menos de la mitad
	// de distancia para ser visto -> el jugador se puede "esconder" alejandose y agachandose,
	// ademas de taparse detras de geometria (eso ya lo bloquea el trace de vision solo).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Sigilo")
	float CrouchSightMultiplier = 0.45f;

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

	// EditDefaultsOnly (no VisibleAnywhere) a proposito: asi se puede expandir y tocar
	// SightRadius / LoseSightRadius / PeripheralVisionAngleDegrees directo en el Class
	// Defaults de BP_TeacherAIController, sin tener que recompilar C++ cada vez que
	// quieras ajustar la vision.
	UPROPERTY(EditDefaultsOnly, Category = "AI|Vision", meta = (AllowPrivateAccess = "true"))
	class UAISenseConfig_Sight* SightConfig;

	UFUNCTION()
	void OnTargetPerceived(AActor* Actor, FAIStimulus Stimulus);
};
