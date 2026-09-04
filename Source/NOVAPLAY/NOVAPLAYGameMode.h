// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NOVAPLAYGameMode.generated.h"

UCLASS(minimalapi)
class ANOVAPLAYGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ANOVAPLAYGameMode();

	// La llama el docente (u otro enemigo) cuando atrapa al jugador.
	// Reinicia el nivel actual desde el principio.
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void OnPlayerCaught();
};
