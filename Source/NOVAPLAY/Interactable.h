// Interactable.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interactable.generated.h"

// Esta clase no se toca, es la que necesita Unreal para el reflection system.
UINTERFACE(MinimalAPI, Blueprintable)
class UInteractable : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interfaz que deben implementar todos los actores con los que el personaje
 * pueda interactuar apretando E (items, palancas, puertas, piezas de puzzle).
 */
class NOVAPLAY_API IInteractable
{
	GENERATED_BODY()

public:

	// El personaje llama esto cuando apunta al objeto y aprieta E.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void Interact(APawn* InteractingPawn);
};