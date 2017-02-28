// Original Work Copyright (c) 2010 Amit J Patel, Modified Work Copyright (c) 2016 Jay M Stevens

#pragma once

#include "IslandShape.h"
#include "RadialIsland.generated.h"
/**
* The Radial Island is a circular island using overlapping sine waves.
*/
UCLASS(Blueprintable)
class POLYGONALMAPGENERATOR_API URadialIsland : public UIslandShape
{
	GENERATED_BODY()

public:
	// The minimum number of sine wave "bumps" along the island.
	UPROPERTY(Category = "Bumps", BlueprintReadWrite, EditAnywhere)
	int32 BumpsMin = 1;
	// The maximum number of sine wave "bumps" along the island.
	UPROPERTY(Category = "Bumps", BlueprintReadWrite, EditAnywhere)
	int32 BumpsMax = 6;

	// The minimum start angle for the sin function, in radians.
	UPROPERTY(Category = "Angle", BlueprintReadWrite, EditAnywhere)
	float StartAngleMin = 0.0f;
	// The maximum start angle for the sin function, in radians.
	UPROPERTY(Category = "Angle", BlueprintReadWrite, EditAnywhere)
	float StartAngleMax = 2.0f;

	UPROPERTY(Category = "Angle", BlueprintReadWrite, EditAnywhere)
	float AngleOffsetMin = 0.0f;
	UPROPERTY(Category = "Angle", BlueprintReadWrite, EditAnywhere)
	float AngleOffsetMax = 2.0f;

	UPROPERTY(Category = "Angle", BlueprintReadWrite, EditAnywhere)
	float MinAngleMin = 0.2f;
	UPROPERTY(Category = "Angle", BlueprintReadWrite, EditAnywhere)
	float MinAngleMax = 0.7f;

protected:
	virtual void SetSeed_Implementation(int32 seed, int32 size) override;
	virtual bool IsPointLand_Implementation(FVector2D point) override;

	// The number of sine waves which form bumps along the island.
	UPROPERTY(Category = "Bumps", BlueprintReadWrite, VisibleAnywhere)
	int32 Bumps;
	// The start angle for the sin function, in radians.
	UPROPERTY(Category = "Angle", BlueprintReadWrite, VisibleAnywhere)
	float StartAngle;
	UPROPERTY(Category = "Angle", BlueprintReadWrite, VisibleAnywhere)
	float AngleOffset;
	UPROPERTY(Category = "Angle", BlueprintReadWrite, VisibleAnywhere)
	float MinAngle;
};
