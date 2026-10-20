#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LevelLightingController.generated.h"

class APuzzleBase;
class ULightComponentBase;
class ULightComponent;
class USoundBase;

// Maneja un nivel "a oscuras": al empezar apaga las luces listadas, le da la linterna al
// jugador y esconde ciertos actores (ej: la llave). Cuando el puzzle asignado se resuelve,
// prende las luces con un fade, revela los actores y completa el objetivo actual.
UCLASS()
class NOVAPLAY_API ALevelLightingController : public AActor
{
	GENERATED_BODY()

public:
	ALevelLightingController();

	// Puzzle que, al resolverse, reactiva la iluminacion (ej: APuzzle_LeverCount)
	UPROPERTY(EditInstanceOnly, Category = "Lighting")
	APuzzleBase* TriggerPuzzle;

	// Actores con luces (PointLight, SpotLight, RectLight, DirectionalLight, SkyLight...).
	// Se apagan en BeginPlay y se encienden al resolver el puzzle. Deben ser Movable.
	UPROPERTY(EditInstanceOnly, Category = "Lighting")
	TArray<AActor*> LightActors;

	// Se ocultan y sin colision hasta que vuelve la luz (ej: la llave de salida)
	UPROPERTY(EditInstanceOnly, Category = "Lighting")
	TArray<AActor*> ActorsToRevealOnRestore;

	// Segundos que tarda la luz en subir de 0 a su intensidad original
	UPROPERTY(EditAnywhere, Category = "Lighting")
	float FadeInDuration = 2.0f;

	UPROPERTY(EditAnywhere, Category = "Lighting")
	USoundBase* RestoreSound = nullptr;

	// Habilita la linterna del jugador al empezar el nivel
	UPROPERTY(EditAnywhere, Category = "Lighting")
	bool bGivePlayerFlashlight = true;

	// Al volver la luz, completa el objetivo actual del jugador (ej: "Activa las 3 palancas")
	UPROPERTY(EditAnywhere, Category = "Lighting")
	bool bCompleteObjectiveOnRestore = true;

	// Hook para el Blueprint (sonido extra, mensaje, cambiar post process, etc.)
	UFUNCTION(BlueprintImplementableEvent, Category = "Lighting")
	void OnLightsRestored();

	UPROPERTY(BlueprintReadOnly, Category = "Lighting")
	bool bLightsOn = false;

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandlePuzzleSolved(APuzzleBase* SolvedPuzzle);

private:
	struct FManagedLight
	{
		ULightComponentBase* Component = nullptr;
		float TargetIntensity = 0.0f;
		bool bHasIntensity = false; // true si es ULightComponent (se puede hacer fade)
	};

	TArray<FManagedLight> ManagedLights;
	float FadeElapsed = 0.0f;
	bool bFading = false;

	void GiveFlashlightToPlayer();
	void RestoreLights();
};
