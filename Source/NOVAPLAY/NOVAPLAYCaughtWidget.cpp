// NOVAPLAY - Base en C++ de la pantalla de "Te atraparon"

#include "NOVAPLAYCaughtWidget.h"
#include "Kismet/GameplayStatics.h"

void UNOVAPLAYCaughtWidget::NativeConstruct()
{
	Super::NativeConstruct();
	OnShown();
}

void UNOVAPLAYCaughtWidget::RestartCurrentLevel()
{
	UGameplayStatics::SetGamePaused(GetWorld(), false);

	// GetCurrentLevelName(..., true) quita el prefijo "UEDPIE_0_" que el editor agrega en PIE.
	// Con GetWorld()->GetName() el reintento fallaba en Play In Editor (nivel inexistente).
	UGameplayStatics::OpenLevel(this, FName(*UGameplayStatics::GetCurrentLevelName(this, true)));
}

void UNOVAPLAYCaughtWidget::Dismiss()
{
	UGameplayStatics::SetGamePaused(GetWorld(), false);

	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->bShowMouseCursor = false;
	}

	RemoveFromParent();
}
