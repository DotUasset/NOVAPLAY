// NOVAPLAY - Base en C++ de la pantalla de "Te atraparon". La crea y la muestra
// ANOVAPLAYCharacter::ShowCaughtScreen() (llamada desde el Event Graph de BP_Teacher, evento
// OnCaughtSequenceComplete), igual que el menu de pausa. El diseño visual y los botones para
// ir a un nivel especifico se arman en un Blueprint hijo (WBP_CaughtScreen): cada boton de
// nivel usa el nodo nativo "Open Level (by Name)" con el FName del nivel al que quiera ir,
// no hace falta pasar por C++ para eso.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NOVAPLAYCaughtWidget.generated.h"

UCLASS()
class NOVAPLAY_API UNOVAPLAYCaughtWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Reintenta el mismo nivel desde el principio (para un boton tipo "Reintentar").
	// Los botones que van a OTRO nivel especifico no necesitan esto: arman su propio
	// "Open Level (by Name)" directo en el grafo del widget, igual que en WBP_PauseMenu.
	UFUNCTION(BlueprintCallable, Category = "Caught Screen")
	void RestartCurrentLevel();

	// Cierra el widget y despausa, por si agregas un boton "Cancelar" (poco probable, pero
	// por si las moscas). No hace falta llamarlo antes de abrir un nivel nuevo: cargar un
	// nivel ya despausa y descarta este widget solo.
	UFUNCTION(BlueprintCallable, Category = "Caught Screen")
	void Dismiss();

protected:
	// Implementalo en el Blueprint hijo para animar la entrada (fade in, etc.)
	UFUNCTION(BlueprintImplementableEvent, Category = "Caught Screen")
	void OnShown();

	virtual void NativeConstruct() override;
};
