// Copyright Epic Games, Inc. All Rights Reserved.

#include "NOVAPLAYGameMode.h"
#include "NOVAPLAYCharacter.h"
#include "UObject/ConstructorHelpers.h"

ANOVAPLAYGameMode::ANOVAPLAYGameMode()
{
	// Si el Blueprint del personaje no se encuentra (movido/renombrado), cae al personaje C++
	// en vez de dejar el pawn por defecto del motor (que no tiene camara ni inventario).
	DefaultPawnClass = ANOVAPLAYCharacter::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
