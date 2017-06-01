// Original Work Copyright (c) 2010 Amit J Patel, Modified Work Copyright (c) 2016 Jay M Stevens

#include "PolygonalMapGeneratorPrivatePCH.h"
#include "Maps/PointGenerators/PointGenerator.h"

void UPointGenerator::InitializeSelector(int32 mapSize, int32 seed, int32 border)
{
	MapSize = mapSize;
	RandomGenerator.Initialize(seed);
	Border = border;
}

bool UPointGenerator::NeedsMoreRandomness()
{
	return bNeedsMoreRandomness;
}

int32 UPointGenerator::MinPoint()
{
	return Border;
}

int32 UPointGenerator::MaxPoint()
{
	return MapSize - Border;
}

bool UPointGenerator::PointIsOnBorder(FVector2D point)
{
	if (point.X <= (Border * 2) || point.Y <= (Border * 2))
	{
		return true;
	}
	if (point.X >= MapSize - (Border * 2) || point.Y >= MapSize - (Border * 2))
	{
		return true;
	}
	return false;
}

TArray<FVector2D> UPointGenerator::GeneratePoints_Implementation(int32 numberOfPoints)
{
	TArray<FVector2D> pointArray;
	pointArray.AddDefaulted(numberOfPoints);
	return pointArray;
}