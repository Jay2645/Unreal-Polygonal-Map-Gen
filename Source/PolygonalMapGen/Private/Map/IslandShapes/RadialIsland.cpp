// Original Work Copyright (c) 2010 Amit J Patel, Modified Work Copyright (c) 2016 Jay M Stevens

#include "PolygonalMapGen.h"
#include "RadialIsland.h"

void URadialIsland::SetSeed(int32 seed, int32 size)
{
	Super::SetSeed(seed, size);
	Bumps = StreamRNG.RandRange(1, 6);
	StartAngle = StreamRNG.FRandRange(0.0f, 2.0 * PI);
	DipAngle = StreamRNG.FRandRange(0.0f, 2.0 * PI);
	DipWidth = StreamRNG.FRandRange(0.2f, 0.7f);
}

bool URadialIsland::IsPointLand(FVector2D point)
{
	float angle = FMath::Atan2(point.Y, point.X);
	float length = FMath::Max(FMath::Abs(point.X), FMath::Abs(point.Y)) / Size;

	float innerRadius;
	float outerRadius;
	if ((FMath::Abs(angle - DipAngle) < DipWidth
		|| FMath::Abs(angle - DipAngle + 2.0f * PI) < DipWidth
		|| FMath::Abs(angle - DipAngle - 2.0f * PI) < DipWidth)
		|| StreamRNG.GetFraction() > 1.0f / (IslandFactor * IslandFactor))
	{
		innerRadius = 0.325f;
		outerRadius = 1.15f;
	}
	else
	{
		innerRadius = 0.475f + 0.4f * FMath::Sin(StartAngle + Bumps * angle + FMath::Cos((Bumps + 3) * angle));
		outerRadius = 1.25f - 0.2f * FMath::Sin(StartAngle + Bumps * angle - FMath::Sin((Bumps + 2) * angle));
	}

	return innerRadius * IslandFactor < length && length * IslandFactor < outerRadius;
	//return (length < innerRadius) || (length < outerRadius && length > innerRadius * IslandFactor);
}
