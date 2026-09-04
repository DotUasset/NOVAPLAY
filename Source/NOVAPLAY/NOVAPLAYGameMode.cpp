// Copyright Epic Games, Inc. All Rights Reserved.

#include "NOVAPLAYGameMode.h"
#include "NOVAPLAYCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"

ANOVAPLAYGameMode::ANOVAPLAYGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

void ANOVAPLAYGameMode::OnPlayerCaught()
{
	// Reinicia el nivel actual (vuelve al inicio del nivel en el que fue atrapado).
	// Si mas adelante queres mostrar una pantalla de "Te atraparon" antes de reiniciar,
	// este es el lugar para disparar ese widget/evento antes del OpenLevel.
	UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()));
}
