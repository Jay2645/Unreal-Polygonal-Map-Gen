// Original Work Copyright (c) 2010 Amit J Patel, Modified Work Copyright (c) 2016 Jay M Stevens

#include "PolygonalMapGeneratorPrivatePCH.h"
#include "Maps/IslandShapes/RadialIsland.h"
void URadialIsland::SetSeed_Implementation(int32 seed, int32 size)
{
	StreamRNG.Initialize(seed);
	Size = size;
	Bumps = StreamRNG.RandRange(BumpsMin, BumpsMax);
	StartAngle = StreamRNG.FRandRange(StartAngleMin, StartAngleMax) * PI;
	AngleOffset = StreamRNG.FRandRange(AngleOffsetMin, AngleOffsetMax) * PI;
	MinAngle = StreamRNG.FRandRange(MinAngleMin, MinAngleMax);
}

bool URadialIsland::IsPointLand_Implementation(FVector2D point)
{
	point.X = 2.0f * ((point.X / Size) - 0.5f);
	point.Y = 2.0f * ((point.Y / Size) - 0.5f);
	// Get the angle of the point from the center of the island
	float angle = FMath::Atan2(point.Y, point.X);
	// Get the normalized length of whichever axis is longer
	float length = 0.5f * (FMath::Max(FMath::Abs(point.X), FMath::Abs(point.Y)) + FVector2D::Distance(FVector2D::ZeroVector, point));
	// The inner radius has to be smaller than the length for this to be land
	float innerRadius;
	// The outer radius has to be larger than the length for this to be land
	float outerRadius;

	if ((FMath::Abs(angle - AngleOffset) < MinAngle
		|| FMath::Abs(angle - AngleOffset + 2.0f * PI) < MinAngle
		|| FMath::Abs(angle - AngleOffset - 2.0f * PI) < MinAngle))
	{
		// Our angle is less than the minimum angle
		innerRadius = 0.2f;
		outerRadius = 0.2f;
	}
	else
	{
		innerRadius = 0.5f + 0.4f * FMath::Sin(StartAngle + Bumps * angle + FMath::Cos((Bumps + 3) * angle));
		outerRadius = 0.7f - 0.2f * FMath::Sin(StartAngle + Bumps * angle - FMath::Sin((Bumps + 2) * angle));
	}

	//return innerRadius * IslandFactor < length && length * IslandFactor < outerRadius;
	return (length < innerRadius) || (length > innerRadius * IslandFactor && length < outerRadius);
}
