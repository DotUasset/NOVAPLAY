#include "ClassroomSeatManager.h"
#include "ClassroomSeat.h"
#include "Kismet/GameplayStatics.h"
#include "NOVAPLAYDebug.h"

AClassroomSeatManager::AClassroomSeatManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AClassroomSeatManager::BeginPlay()
{
	Super::BeginPlay();
	PickRandomSeat();
}

void AClassroomSeatManager::PickRandomSeat()
{
	TArray<AActor*> Found;
	UGameplayStatics::GetAllActorsOfClass(this, AClassroomSeat::StaticClass(), Found);

	if (Found.Num() == 0)
	{
		NOVAPLAYDebug::ScreenMessage(TEXT("SeatManager: no hay ClassroomSeat en el nivel!"), FColor::Red, 5.0f);
		return;
	}

	// Apaga todos y enciende solo uno
	for (AActor* Actor : Found)
	{
		if (AClassroomSeat* Seat = Cast<AClassroomSeat>(Actor))
		{
			Seat->SetAssigned(false);
		}
	}

	AssignedSeat = Cast<AClassroomSeat>(Found[FMath::RandRange(0, Found.Num() - 1)]);
	if (AssignedSeat)
	{
		AssignedSeat->SetAssigned(true);
	}
}
