#include "LevelExitDoor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"

ALevelExitDoor::ALevelExitDoor()
{
	FrameMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FrameMesh"));
	RootComponent = FrameMesh;

	DoorPivot = CreateDefaultSubobject<USceneComponent>(TEXT("DoorPivot"));
	DoorPivot->SetupAttachment(RootComponent);

	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	DoorMesh->SetupAttachment(DoorPivot);

	ExitTrigger->SetupAttachment(RootComponent);
}