// Original Work Copyright (c) 2010 Amit J Patel, Modified Work Copyright (c) 2016 Jay M Stevens

#include "PolygonalMapGeneratorPrivatePCH.h"
#include "Maps/MapDataHelper.h"
#include "Biomes/BiomeManager.h"
#include "Moisture/MoistureDistributor.h"
#include "Maps/Heightmap/HeightmapPointTask.h"
#include "PolygonalMapHeightmap.h"

void UPolygonalMapHeightmap::CreateHeightmap(UPolygonMap* PolygonMap, UBiomeManager* BiomeManager, UMoistureDistributor* MoistureDist, const int32 Size, const FIslandGeneratorDelegate OnComplete)
{
	if (PolygonMap == NULL)
	{
		return;
	}
	CreateHeightmapTimer = FPlatformTime::Seconds();
	MoistureDistributor = MoistureDist;
	HeightmapSize = Size;
	OnGenerationComplete = OnComplete;
	TArray<FMapData> graph = PolygonMap->GetAllMapData();
	UE_LOG(LogWorldGen, Log, TEXT("Map Data fetched in %f seconds."), FPlatformTime::Seconds() - CreateHeightmapTimer);

	// First, insert a border around the map
	CreateHeightmapTimer = FPlatformTime::Seconds();
	for (int x = 0; x < HeightmapSize; x++)
	{
		FMapData borderPoint = FMapData();
		borderPoint.Elevation = 0.0f;
		borderPoint.Moisture = 0.0f;
		borderPoint = UMapDataHelper::SetOcean(borderPoint);
		borderPoint = UMapDataHelper::SetBorder(borderPoint);
		borderPoint.Point = FVector2D(x, 0);
		graph.Add(borderPoint);
		borderPoint.Point = FVector2D(x, Size - 1);
		graph.Add(borderPoint);
	}
	for (int y = 0; y < HeightmapSize; y++)
	{
		FMapData borderPoint = FMapData();
		borderPoint.Elevation = 0.0f;
		borderPoint.Moisture = 0.0f;
		borderPoint = UMapDataHelper::SetOcean(borderPoint);
		borderPoint = UMapDataHelper::SetBorder(borderPoint);
		borderPoint.Point = FVector2D(0, y);
		graph.Add(borderPoint);
		borderPoint.Point = FVector2D(Size - 1, y);
		graph.Add(borderPoint);
	}
	UE_LOG(LogWorldGen, Log, TEXT("Border generated in %f seconds."), FPlatformTime::Seconds() - CreateHeightmapTimer);

	// Now, interpolate between the actual points
	CreateHeightmapTimer = FPlatformTime::Seconds();

	FIslandGeneratorDelegate generatePoints;
	generatePoints.BindDynamic(this, &UPolygonalMapHeightmap::CheckMapPointsDone);
	FHeightmapPointGenerator::GenerateHeightmapPoints(HeightmapSize, NumberOfPointsToAverage, this, PolygonMap, BiomeManager, generatePoints);
}

void UPolygonalMapHeightmap::CheckMapPointsDone()
{
	HeightmapData = FHeightmapPointGenerator::HeightmapData;
	UE_LOG(LogWorldGen, Log, TEXT("%d map points created in %f seconds."), HeightmapSize * HeightmapSize, FPlatformTime::Seconds() - CreateHeightmapTimer);

	// Add the rivers
	CreateHeightmapTimer = FPlatformTime::Seconds();
	for (int i = 0; i < MoistureDistributor->Rivers.Num(); i++)
	{
		MoistureDistributor->Rivers[i]->MoveRiverToHeightmap(this);
	}
	UE_LOG(LogWorldGen, Log, TEXT("Rivers placed in %f seconds."), FPlatformTime::Seconds() - CreateHeightmapTimer);

	if (OnGenerationComplete.IsBound())
	{
		OnGenerationComplete.Execute();
		OnGenerationComplete.Unbind();
	}
}

TArray<FMapData> UPolygonalMapHeightmap::GetMapData()
{
	return HeightmapData;
}

FMapData UPolygonalMapHeightmap::GetMapPoint(int32 x, int32 y)
{
	int32 index = x + (y * HeightmapSize);
	if (index < 0 || HeightmapData.Num() <= index)
	{
		UE_LOG(LogWorldGen, Warning, TEXT("Tried to fetch a pixel at %d, %d, but no pixel was found."), x, y);
		return FMapData();
	}
	else
	{
		return HeightmapData[index];
	}
}

void UPolygonalMapHeightmap::SetMapPoint(int32 X, int32 Y, FMapData MapData)
{
	int32 index = X + (Y * HeightmapSize);
	if (index < 0 || HeightmapData.Num() <= index)
	{
		UE_LOG(LogWorldGen, Warning, TEXT("Tried to fetch a pixel at %d, %d, but no pixel was found."), X, Y);
		return;
	}
	else
	{
		HeightmapData[index] = MapData;
	}
}