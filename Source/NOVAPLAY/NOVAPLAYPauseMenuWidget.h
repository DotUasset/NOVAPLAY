// Base en C++ del menu de pausa. La UI (botones, sliders, layout) se arma en un
// Blueprint hijo (WBP_PauseMenu); esta clase solo expone las funciones que esos
// controles van a llamar, y sabe como aplicar/guardar cada configuracion.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NOVAPLAYPauseMenuWidget.generated.h"

UCLASS()
class NOVAPLAY_API UNOVAPLAYPauseMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// ----- GENERAL -----

	// Cierra el menu y reanuda el juego (llamalo desde el boton "Reanudar" y desde "Volver")
	UFUNCTION(BlueprintCallable, Category = "Pause Menu")
	void ResumeGame();

	// Cierra el juego (Alt+F4 prolijo). No hace nada distinto en PIE (el editor no cierra).
	UFUNCTION(BlueprintCallable, Category = "Pause Menu")
	void QuitGame();

	// ----- CONTROLES: sensibilidad de mouse -----

	UFUNCTION(BlueprintCallable, Category = "Settings|Controls")
	void SetMouseSensitivity(float NewSensitivity);

	UFUNCTION(BlueprintPure, Category = "Settings|Controls")
	float GetMouseSensitivity() const;

	// ----- VIDEO: usa el UGameUserSettings nativo de Unreal -----

	// 0=Bajo, 1=Medio, 2=Alto, 3=Epico (mismo criterio que el menu de Scalability del editor)
	UFUNCTION(BlueprintCallable, Category = "Settings|Video")
	void SetOverallQuality(int32 QualityLevel);

	UFUNCTION(BlueprintPure, Category = "Settings|Video")
	int32 GetOverallQuality() const;

	UFUNCTION(BlueprintCallable, Category = "Settings|Video")
	void SetScreenResolution(int32 Width, int32 Height);

	UFUNCTION(BlueprintCallable, Category = "Settings|Video")
	void SetFullscreenEnabled(bool bFullscreen);

	UFUNCTION(BlueprintPure, Category = "Settings|Video")
	bool IsFullscreenEnabled() const;

	// Confirma los cambios de video (se llama despues de tocar cualquiera de los de arriba,
	// tipico patron "Apply" para no trabar el juego si el jugador pone una resolucion rara)
	UFUNCTION(BlueprintCallable, Category = "Settings|Video")
	void ApplyVideoSettings();

	// ----- AUDIO: via SoundMix + SoundClass (asignalos en el Class Defaults del WBP hijo) -----

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings|Audio")
	class USoundMix* MasterSoundMix;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings|Audio")
	class USoundClass* MasterSoundClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings|Audio")
	class USoundClass* MusicSoundClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings|Audio")
	class USoundClass* SFXSoundClass;

	UFUNCTION(BlueprintCallable, Category = "Settings|Audio")
	void SetMasterVolume(float NewVolume); // 0.0 - 1.0

	UFUNCTION(BlueprintCallable, Category = "Settings|Audio")
	void SetMusicVolume(float NewVolume);

	UFUNCTION(BlueprintCallable, Category = "Settings|Audio")
	void SetSFXVolume(float NewVolume);

	UFUNCTION(BlueprintPure, Category = "Settings|Audio")
	float GetMasterVolume() const { return CachedMasterVolume; }

	UFUNCTION(BlueprintPure, Category = "Settings|Audio")
	float GetMusicVolume() const { return CachedMusicVolume; }

	UFUNCTION(BlueprintPure, Category = "Settings|Audio")
	float GetSFXVolume() const { return CachedSFXVolume; }

	// ----- GUARDADO -----

	// Llamalo al abrir el menu por primera vez (o en el evento Construct del widget)
	// para que los sliders/dropdowns arranquen mostrando los valores guardados
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void LoadSettings();

	// Llamalo al cerrar el menu (o con un boton "Guardar") para persistir a disco
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void SaveSettings();

protected:
	virtual void NativeConstruct() override;

	float CachedMasterVolume = 1.0f;
	float CachedMusicVolume = 1.0f;
	float CachedSFXVolume = 1.0f;

	class ANOVAPLAYCharacter* GetOwningNOVAPLAYCharacter() const;
};
