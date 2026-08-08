// Guarda en disco las configuraciones que no maneja UGameUserSettings (video ya se
// guarda solo con Epic). Sensibilidad de mouse y volumenes de audio van aca.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "NOVAPLAYSettingsSave.generated.h"

UCLASS()
class NOVAPLAY_API UNOVAPLAYSettingsSave : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "Settings")
	float MouseSensitivity = 1.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Settings")
	float MasterVolume = 1.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Settings")
	float MusicVolume = 1.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Settings")
	float SFXVolume = 1.0f;

	static FString GetSlotName() { return TEXT("NOVAPLAYSettings"); }
	static int32 GetUserIndex() { return 0; }
};
