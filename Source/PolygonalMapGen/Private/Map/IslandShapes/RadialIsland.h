// Original Work Copyright (c) 2010 Amit J Patel, Modified Work Copyright (c) 2016 Jay M Stevens

#pragma once

#include "IslandShape.h"
#include "RadialIsland.generated.h"

/**
 * The Radial Island is a circular island using overlapping sine waves.
 */
UCLASS(Blueprintable)
class URadialIsland : public UIslandShape
{
	GENERATED_BODY()

	virtual void SetSeed(int32 seed, int32 size) override;

	virtual bool IsPointLand(FVector2D point) override;

protected:
	int32 Bumps;
	float StartAngle;
	float DipAngle;
	float DipWidth;
};
