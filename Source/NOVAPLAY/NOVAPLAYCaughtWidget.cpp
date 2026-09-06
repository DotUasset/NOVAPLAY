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
	UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()));
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
