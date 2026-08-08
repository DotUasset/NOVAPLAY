#include "NOVAPLAYPauseMenuWidget.h"
#include "NOVAPLAYCharacter.h"
#include "NOVAPLAYSettingsSave.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/GameUserSettings.h"
#include "Sound/SoundMix.h"
#include "Sound/SoundClass.h"

ANOVAPLAYCharacter* UNOVAPLAYPauseMenuWidget::GetOwningNOVAPLAYCharacter() const
{
	return Cast<ANOVAPLAYCharacter>(GetOwningPlayerPawn());
}

void UNOVAPLAYPauseMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Activa el sound mix una sola vez al crear el widget, asi los SetXVolume de mas
	// abajo tienen algo sobre lo que aplicar el override
	if (MasterSoundMix)
	{
		UGameplayStatics::PushSoundMixModifier(GetWorld(), MasterSoundMix);
	}

	LoadSettings();
}

//////////////////////////////////////////////////////////////////////////
// General

void UNOVAPLAYPauseMenuWidget::ResumeGame()
{
	SaveSettings();

	UGameplayStatics::SetGamePaused(GetWorld(), false);

	APlayerController* PC = GetOwningPlayer();
	if (ANOVAPLAYCharacter* Character = GetOwningNOVAPLAYCharacter())
	{
		// El personaje maneja su propio flag bPauseMenuOpen y el input mode;
		// esto evita que el estado se desincronice si el jugador cierra desde un
		// boton de la UI en vez de apretando Esc de vuelta
		Character->ClosePauseMenu();
	}

	RemoveFromParent();
}

void UNOVAPLAYPauseMenuWidget::QuitGame()
{
	SaveSettings();

	APlayerController* PC = GetOwningPlayer();
	UKismetSystemLibrary::QuitGame(GetWorld(), PC, EQuitPreference::Quit, false);
}

//////////////////////////////////////////////////////////////////////////
// Controles: sensibilidad

void UNOVAPLAYPauseMenuWidget::SetMouseSensitivity(float NewSensitivity)
{
	if (ANOVAPLAYCharacter* Character = GetOwningNOVAPLAYCharacter())
	{
		Character->SetMouseSensitivity(NewSensitivity);
	}
}

float UNOVAPLAYPauseMenuWidget::GetMouseSensitivity() const
{
	if (ANOVAPLAYCharacter* Character = GetOwningNOVAPLAYCharacter())
	{
		return Character->GetMouseSensitivity();
	}

	return 1.0f;
}

//////////////////////////////////////////////////////////////////////////
// Video

void UNOVAPLAYPauseMenuWidget::SetOverallQuality(int32 QualityLevel)
{
	if (UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		Settings->SetOverallScalabilityLevel(QualityLevel);
	}
}

int32 UNOVAPLAYPauseMenuWidget::GetOverallQuality() const
{
	if (UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		return Settings->GetOverallScalabilityLevel();
	}

	return 2; // Alto por default si algo falla
}

void UNOVAPLAYPauseMenuWidget::SetScreenResolution(int32 Width, int32 Height)
{
	if (UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		Settings->SetScreenResolution(FIntPoint(Width, Height));
	}
}

void UNOVAPLAYPauseMenuWidget::SetFullscreenEnabled(bool bFullscreen)
{
	if (UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		Settings->SetFullscreenMode(bFullscreen ? EWindowMode::Fullscreen : EWindowMode::Windowed);
	}
}

bool UNOVAPLAYPauseMenuWidget::IsFullscreenEnabled() const
{
	if (UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		return Settings->GetFullscreenMode() == EWindowMode::Fullscreen;
	}

	return true;
}

void UNOVAPLAYPauseMenuWidget::ApplyVideoSettings()
{
	if (UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		Settings->ApplySettings(false);
		Settings->SaveSettings(); // esto Epic ya lo guarda en su propio archivo, aparte de nuestro SaveGame
	}
}

//////////////////////////////////////////////////////////////////////////
// Audio

void UNOVAPLAYPauseMenuWidget::SetMasterVolume(float NewVolume)
{
	CachedMasterVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);

	if (MasterSoundMix && MasterSoundClass)
	{
		UGameplayStatics::SetSoundMixClassOverride(GetWorld(), MasterSoundMix, MasterSoundClass, CachedMasterVolume, 1.0f, 0.0f, true);
	}
}

void UNOVAPLAYPauseMenuWidget::SetMusicVolume(float NewVolume)
{
	CachedMusicVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);

	if (MasterSoundMix && MusicSoundClass)
	{
		UGameplayStatics::SetSoundMixClassOverride(GetWorld(), MasterSoundMix, MusicSoundClass, CachedMusicVolume, 1.0f, 0.0f, true);
	}
}

void UNOVAPLAYPauseMenuWidget::SetSFXVolume(float NewVolume)
{
	CachedSFXVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);

	if (MasterSoundMix && SFXSoundClass)
	{
		UGameplayStatics::SetSoundMixClassOverride(GetWorld(), MasterSoundMix, SFXSoundClass, CachedSFXVolume, 1.0f, 0.0f, true);
	}
}

//////////////////////////////////////////////////////////////////////////
// Guardado

void UNOVAPLAYPauseMenuWidget::LoadSettings()
{
	UNOVAPLAYSettingsSave* Save = nullptr;

	if (UGameplayStatics::DoesSaveGameExist(UNOVAPLAYSettingsSave::GetSlotName(), UNOVAPLAYSettingsSave::GetUserIndex()))
	{
		Save = Cast<UNOVAPLAYSettingsSave>(UGameplayStatics::LoadGameFromSlot(UNOVAPLAYSettingsSave::GetSlotName(), UNOVAPLAYSettingsSave::GetUserIndex()));
	}

	if (!Save)
	{
		Save = Cast<UNOVAPLAYSettingsSave>(UGameplayStatics::CreateSaveGameObject(UNOVAPLAYSettingsSave::StaticClass()));
	}

	if (!Save)
	{
		return;
	}

	SetMouseSensitivity(Save->MouseSensitivity);
	SetMasterVolume(Save->MasterVolume);
	SetMusicVolume(Save->MusicVolume);
	SetSFXVolume(Save->SFXVolume);
}

void UNOVAPLAYPauseMenuWidget::SaveSettings()
{
	UNOVAPLAYSettingsSave* Save = Cast<UNOVAPLAYSettingsSave>(UGameplayStatics::CreateSaveGameObject(UNOVAPLAYSettingsSave::StaticClass()));

	if (!Save)
	{
		return;
	}

	Save->MouseSensitivity = GetMouseSensitivity();
	Save->MasterVolume = CachedMasterVolume;
	Save->MusicVolume = CachedMusicVolume;
	Save->SFXVolume = CachedSFXVolume;

	UGameplayStatics::SaveGameToSlot(Save, UNOVAPLAYSettingsSave::GetSlotName(), UNOVAPLAYSettingsSave::GetUserIndex());
}
