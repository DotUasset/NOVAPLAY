#pragma once

#include "CoreMinimal.h"
#include "PuzzleBase.h"
#include "Puzzle_BookExchange.generated.h"

UCLASS()
class NOVAPLAY_API APuzzle_BookExchange : public APuzzleBase
{
	GENERATED_BODY()

public:
	// Los ItemID de los libros que hacen falta juntar antes de poder canjear
	UPROPERTY(EditAnywhere, Category = "Puzzle")
	TArray<FName> RequiredBookIDs;

	// ----- Datos de la llave que se entrega a cambio -----
	UPROPERTY(EditAnywhere, Category = "Puzzle")
	FName KeyItemID;

	UPROPERTY(EditAnywhere, Category = "Puzzle")
	FText KeyItemName = FText::FromString(TEXT("Llave"));

	UPROPERTY(EditAnywhere, Category = "Puzzle")
	class UTexture2D* KeyIcon = nullptr;

	UPROPERTY(EditAnywhere, Category = "Puzzle")
	FText KeyDescription;

	// Intenta el canje: si el jugador tiene TODOS los libros requeridos, se los
	// saca del inventario y le entrega la llave. Devuelve true si se pudo hacer.
	UFUNCTION(BlueprintCallable, Category = "Puzzle")
	bool TryExchange(APawn* InteractingPawn);
};
