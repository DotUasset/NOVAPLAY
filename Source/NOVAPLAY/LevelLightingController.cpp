#include "LevelLightingController.h"
#include "PuzzleBase.h"
#include "Components/LightComponent.h"
#include "Components/LightComponentBase.h"
#include "Kismet/GameplayStatics.h"
#include "NOVAPLAYCharacter.h"
#include "NOVAPLAYObjectiveComponent.h"
#include "TimerManager.h"

ALevelLightingController::ALevelLightingController()
{
	// El tick solo se prende durante el fade-in
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

void ALevelLightingController::BeginPlay()
{
	Super::BeginPlay();

	// 1) Apagar las luces del nivel (guardando la intensidad original)
	for (AActor* LightActor : LightActors)
	{
		if (!LightActor)
		{
			continue;
		}

		TArray<ULightComponentBase*> Comps;
		LightActor->GetComponents<ULightComponentBase>(Comps);

		for (ULightComponentBase* Comp : Comps)
		{
			FManagedLight Managed;
			Managed.Component = Comp;

			if (ULightComponent* Light = Cast<ULightComponent>(Comp))
			{
				Managed.bHasIntensity = true;
				Managed.TargetIntensity = Light->Intensity;
			}

			ManagedLights.Add(Managed);
			Comp->SetVisibility(false);
		}
	}

	// 2) Esconder lo que solo debe aparecer con luz (la llave)
	for (AActor* Actor : ActorsToRevealOnRestore)
	{
		if (Actor)
		{
			Actor->SetActorHiddenInGame(true);
			Actor->SetActorEnableCollision(false);
		}
	}

	// 3) Escuchar el puzzle de las palancas
	if (TriggerPuzzle)
	{
		TriggerPuzzle->OnPuzzleSolved.AddDynamic(this, &ALevelLightingController::HandlePuzzleSolved);
	}
	else if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Red, TEXT("LevelLightingController sin TriggerPuzzle!"));
	}

	// 4) Linterna
	if (bGivePlayerFlashlight)
	{
		GiveFlashlightToPlayer();
	}
}

void ALevelLightingController::GiveFlashlightToPlayer()
{
	if (ANOVAPLAYCharacter* Player = Cast<ANOVAPLAYCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
	{
		Player->SetFlashlightAvailable(true);
	}
	else
	{
		// Por si el personaje todavia no existe en este frame: reintenta al siguiente
		GetWorldTimerManager().SetTimerForNextTick(this, &ALevelLightingController::GiveFlashlightToPlayer);
	}
}

void ALevelLightingController::HandlePuzzleSolved(APuzzleBase* SolvedPuzzle)
{
	RestoreLights();
}

void ALevelLightingController::RestoreLights()
{
	if (bLightsOn)
	{
		return;
	}
	bLightsOn = true;

	for (FManagedLight& Managed : ManagedLights)
	{
		if (!Managed.Component)
		{
			continue;
		}

		if (Managed.bHasIntensity && FadeInDuration > 0.0f)
		{
			// Arranca en 0 y sube en Tick
			Cast<ULightComponent>(Managed.Component)->SetIntensity(0.0f);
		}
		Managed.Component->SetVisibility(true);
	}

	if (FadeInDuration > 0.0f)
	{
		FadeElapsed = 0.0f;
		bFading = true;
		SetActorTickEnabled(true);
	}

	for (AActor* Actor : ActorsToRevealOnRestore)
	{
		if (Actor)
		{
			Actor->SetActorHiddenInGame(false);
			Actor->SetActorEnableCollision(true);
		}
	}

	if (RestoreSound)
	{
		UGameplayStatics::PlaySound2D(this, RestoreSound);
	}

	if (bCompleteObjectiveOnRestore)
	{
		if (ACharacter* Player = UGameplayStatics::GetPlayerCharacter(this, 0))
		{
			if (UNOVAPLAYObjectiveComponent* Objectives = Player->FindComponentByClass<UNOVAPLAYObjectiveComponent>())
			{
				Objectives->CompleteCurrentObjective();
			}
		}
	}

	OnLightsRestored();
}

void ALevelLightingController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bFading)
	{
		return;
	}

	FadeElapsed += DeltaTime;
	const float Alpha = FMath::Clamp(FadeElapsed / FMath::Max(FadeInDuration, KINDA_SMALL_NUMBER), 0.0f, 1.0f);

	for (FManagedLight& Managed : ManagedLights)
	{
		if (Managed.Component && Managed.bHasIntensity)
		{
			Cast<ULightComponent>(Managed.Component)->SetIntensity(Managed.TargetIntensity * Alpha);
		}
	}

	if (Alpha >= 1.0f)
	{
		bFading = false;
		SetActorTickEnabled(false);
	}
}
