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

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandlePuzzleSolved(APuzzleBase* SolvedPuzzle);

	void CheckAllSolved();

	UFUNCTION()
	void OnExitTriggerOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// Cada subclase implementa en su propio Blueprint como se ve "abrirse"
	// (mover una pared, girar una puerta, lo que sea)
	UFUNCTION(BlueprintImplementableEvent, Category = "Exit")
	void OnExitOpened();
};