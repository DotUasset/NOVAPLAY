// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NOVAPLAYGameMode.generated.h"

// La logica de "fin de partida al ser atrapado" vive en ANOVAPLAYCharacter::ShowCaughtScreen()
// (mismo patron que el menu de pausa/inventario) y se dispara desde el Event Graph de BP_Teacher
// (evento OnCaughtSequenceComplete). Este GameMode solo se encarga de elegir el pawn del jugador.
UCLASS(minimalapi)
class ANOVAPLAYGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ANOVAPLAYGameMode();
};
