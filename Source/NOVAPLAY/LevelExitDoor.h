#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LevelExitDoor.generated.h"

class APuzzleBase;
class UBoxComponent;
class UStaticMeshComponent;

UCLASS()
class NOVAPLAY_API ALevelExitDoor : public AActor
{
	GENERATED_BODY()

public:
	ALevelExitDoor();

	// La pared que tapa la salida — se mueve/desaparece al resolver los puzzles
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door")
	UStaticMeshComponent* WallMesh;

	// Volumen en el hueco de la salida; cuando el jugador lo cruza (y la pared ya se abrió), carga el siguiente nivel
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door")
	UBoxComponent* ExitTrigger;

	UPROPERTY(EditInstanceOnly, Category = "Door")
	TArray<APuzzleBase*> RequiredPuzzles;

	UPROPERTY(EditAnywhere, Category = "Door")
	FName NextLevelName;

	UPROPERTY(BlueprintReadOnly, Category = "Door")
	bool bWallOpened = false;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandlePuzzleSolved(APuzzleBase* SolvedPuzzle);

	void CheckAllSolved();

	UFUNCTION()
	void OnExitTriggerOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// El Blueprint hijo implementa acá la animación de la pared (Timeline)
	UFUNCTION(BlueprintImplementableEvent, Category = "Door")
	void OnWallOpened();
};