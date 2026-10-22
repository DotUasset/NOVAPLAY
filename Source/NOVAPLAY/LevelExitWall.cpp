#include "LevelExitWall.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"

ALevelExitWall::ALevelExitWall()
{
	WallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WallMesh"));
	RootComponent = WallMesh;

	ExitTrigger->SetupAttachment(RootComponent);
}