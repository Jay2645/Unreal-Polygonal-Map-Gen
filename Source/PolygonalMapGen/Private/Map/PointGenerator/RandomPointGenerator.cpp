// Original Work Copyright (c) 2010 Amit J Patel

#include "PolygonalMapGen.h"
#include "RandomPointGenerator.h"

// Random Point Selector
TArray<FVector2D> URandomPointGenerator::GeneratePoints_Implementation(int32 numberOfPoints)
{
	TArray<FVector2D> pointArray;
	for (int32 i = 0; i < numberOfPoints; i++)
	{
		float randomX = RandomGenerator.FRandRange(Border, MapSize - Border);
		float randomY = RandomGenerator.FRandRange(Border, MapSize - Border);
		randomX -= MapSize * 0.5f;
		randomY -= MapSize * 0.5f;
		pointArray.Add(FVector2D(randomX, randomY));
	}
	return pointArray;
}