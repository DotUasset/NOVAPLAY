#pragma once

#include "CoreMinimal.h"
#include "LevelExitBase.h"
#include "LevelExitWall.generated.h"

UCLASS()
class NOVAPLAY_API ALevelExitWall : public ALevelExitBase
{
	GENERATED_BODY()

public:
	ALevelExitWall();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wall")
	class UStaticMeshComponent* WallMesh;
};