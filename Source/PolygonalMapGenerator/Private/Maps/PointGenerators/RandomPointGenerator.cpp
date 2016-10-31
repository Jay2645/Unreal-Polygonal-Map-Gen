// Original Work Copyright (c) 2010 Amit J Patel, Modified Work Copyright (c) 2016 Jay M Stevens

#include "PolygonalMapGeneratorPrivatePCH.h"
#include "Maps/PointGenerators/RandomPointGenerator.h"

// Random Point Selector
TArray<FVector2D> URandomPointGenerator::GeneratePoints_Implementation(int32 numberOfPoints)
{
	TArray<FVector2D> pointArray;
	for (int32 i = 0; i < numberOfPoints; i++)
	{
		float randomX = RandomGenerator.FRandRange(Border, MapSize - Border);
		float randomY = RandomGenerator.FRandRange(Border, MapSize - Border);
		pointArray.Add(FVector2D(randomX, randomY));
	}
	return pointArray;
}