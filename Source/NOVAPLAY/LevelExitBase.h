#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LevelExitBase.generated.h"

class APuzzleBase;
class UBoxComponent;

// Base abstracta: escucha puzzles y maneja el trigger de salida. No tiene mesh propio --
// cada subclase (Wall, Door) crea el suyo y decide COMO se ve "abrirse".
UCLASS(Abstract)
class NOVAPLAY_API ALevelExitBase : public AActor
{
	GENERATED_BODY()

public:
	ALevelExitBase();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Exit")
	UBoxComponent* ExitTrigger;

	UPROPERTY(EditInstanceOnly, Category = "Exit")
	TArray<APuzzleBase*> RequiredPuzzles;

	UPROPERTY(EditAnywhere, Category = "Exit")
	FName NextLevelName;

	UPROPERTY(BlueprintReadOnly, Category = "Exit")
	bool bIsOpened = false;

	// Tiempo (seg) entre que el jugador pisa el trigger de salida (ya abierto) y que
	// efectivamente se carga el siguiente nivel. Da lugar a que el HUD alcance a mostrar
	// el "Objetivo completado: Escapar" antes de que cambie el nivel.
	UPROPERTY(EditAnywhere, Category = "Exit")
	float ExitDelay = 1.2f;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandlePuzzleSolved(APuzzleBase* SolvedPuzzle);

	void CheckAllSolved();

	/** Arranca la secuencia de salida si OtherActor es el jugador y la salida esta abierta. */
	void TryBeginExitSequence(AActor* OtherActor);

	UFUNCTION()
	void OnExitTriggerOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// Cada subclase implementa en su propio Blueprint como se ve "abrirse"
	// (mover una pared, girar una puerta, lo que sea)
	UFUNCTION(BlueprintImplementableEvent, Category = "Exit")
	void OnExitOpened();

private:
	// Evita que un segundo overlap reinicie el timer o dispare el objetivo dos veces.
	bool bExitSequenceStarted = false;

	FTimerHandle ExitDelayTimerHandle;

	void OpenNextLevel();
};