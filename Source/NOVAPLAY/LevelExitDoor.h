#pragma once

#include "CoreMinimal.h"
#include "LevelExitBase.h"
#include "LevelExitDoor.generated.h"

UCLASS()
class NOVAPLAY_API ALevelExitDoor : public ALevelExitBase
{
	GENERATED_BODY()

public:
	ALevelExitDoor();

	// El marco: no se mueve, es el root del actor (tu mesh "Pared" de BP_Puerta_Central_A)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door")
	class UStaticMeshComponent* FrameMesh;

	// Punto de la bisagra: posicionalo en el Viewport justo en el borde donde
	// gira la puerta -- asi no importa donde tenga el pivot el mesh original
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door")
	class USceneComponent* DoorPivot;

	// La hoja de la puerta (tu mesh "Puerta"): cuelga del DoorPivot, no del marco directo
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door")
	class UStaticMeshComponent* DoorMesh;

	// Cuantos grados gira al abrirse (probá con numero negativo si abre para el lado que no queres)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door")
	float DoorOpenAngle = -90.0f;
};