// NOVAPLAY - Helper para mensajes de debug en pantalla.
//
// Todo el feedback tipo "Palancas: 1/3" que antes se escribia con GEngine->AddOnScreenDebugMessage
// repetido en cada archivo pasa por aca. En builds de Shipping se compila a nada, asi que
// el jugador final nunca ve texto de debug aunque se te olvide quitar una llamada.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"

namespace NOVAPLAYDebug
{
	/** Muestra un mensaje en pantalla (no-op en Shipping). Key = -1 apila mensajes nuevos. */
	FORCEINLINE void ScreenMessage(const FString& Message, const FColor& Color = FColor::Yellow, float Duration = 3.0f, int32 Key = -1)
	{
#if !UE_BUILD_SHIPPING
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(Key, Duration, Color, Message);
		}
#endif
	}
}
