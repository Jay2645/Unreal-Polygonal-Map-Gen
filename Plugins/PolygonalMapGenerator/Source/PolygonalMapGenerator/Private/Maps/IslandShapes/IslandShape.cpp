// Original Work Copyright (c) 2010 Amit J Patel, Modified Work Copyright (c) 2016 Jay M Stevens

#include "PolygonalMapGeneratorPrivatePCH.h"
#include "Maps/IslandShapes/IslandShape.h"

void UIslandShape::SetSeed_Implementation(int32 seed, int32 size)
{
	StreamRNG.Initialize(seed);
	Size = size;
}

// Default does nothing
bool UIslandShape::IsPointLand_Implementation(FVector2D point)
{
	return false;
}