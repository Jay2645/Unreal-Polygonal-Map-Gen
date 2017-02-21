// Original Work Copyright (c) 2010 Amit J Patel, Modified Work Copyright (c) 2016 Jay M Stevens

#include "PolygonalMapGeneratorPrivatePCH.h"
#include "Maps/MapDataHelper.h"
#include "Biomes/BiomeManager.h"
#include "PolygonalMapHeightmap.h"

FMapData UPolygonalMapHeightmap::MakeMapPoint(FVector2D PixelPosition, TArray<FMapData> MapData, UBiomeManager* BiomeManager)
{
	TArray<FMapData> closestPoints;
	// Iterate over the entire mapData array to find how many points we need to average
		for (int i = 0; i < MapData.Num(); i++)
		{
			if (closestPoints.Num() == 0)
			{
				closestPoints.Add(MapData[i]);
				continue;
			}
			float distance = FVector2D::DistSquared(PixelPosition, MapData[i].Point);
			if (distance <= 0.001f)
			{
				// Close enough
				FMapData pixelData = MapData[i];
				pixelData.Biome = BiomeManager->DetermineBiome(pixelData);
				return pixelData;
			}

			// This will hold the index of first point we find that's further away than our point
			int addPointIndex = -1;
			for (int j = 0; j < closestPoints.Num(); j++)
			{
				// Get the distance of this point
				float pointDist = FVector2D::DistSquared(PixelPosition, closestPoints[j].Point);
				if (distance < pointDist)
				{
					addPointIndex = j;
					break;
				}
			}

			// If we found a point that's further away than our point, place it in the array and move everything else down
			if (addPointIndex >= 0)
			{
				FMapData last = MapData[i];
				for (int j = addPointIndex; j < closestPoints.Num(); j++)
				{
					FMapData temp = closestPoints[j];
					closestPoints[j] = last;
					last = temp;
				}
				// If we are below the number of points we need to add, then add the furthest point to the end
				if (closestPoints.Num() < NumberOfPointsToAverage)
				{
					closestPoints.Add(last);
				}
			}
			else if (closestPoints.Num() < NumberOfPointsToAverage)
			{
				closestPoints.Add(MapData[i]);
			}
		}

	// Cache the distances
	TArray<float> closestPointDistances;
	float totalDistance = 0.0f;
	for (int i = 0; i < closestPoints.Num(); i++)
	{
		float distance = FVector2D::DistSquared(PixelPosition, closestPoints[i].Point);
		totalDistance += distance;
		closestPointDistances.Add(distance);
	}

	float inversePercentageTotal = 0.0f;

	for (int i = 0; i < closestPoints.Num(); i++)
	{
		// Get the total percentage that this point contributed to the overall distance
		float percentageOfDistance = closestPointDistances[i] / totalDistance;

		// Take the inverse of the distance percentage -- points which are closer get a larger weight
		float inversePercentage = 1.0f - percentageOfDistance;

		// We re-add the inverse percentage to the array so we can make sure it all totals up to 1
		closestPointDistances[i] = inversePercentage;
		inversePercentageTotal += inversePercentage;
	}

	// Now gather the weighted distance for each point
	TArray<TPair<FMapData, float>> pointWeights;
	for (int i = 0; i < closestPoints.Num(); i++)
	{
		TPair<FMapData, float> weight;
		weight.Key = closestPoints[i];
		weight.Value = closestPointDistances[i] / inversePercentageTotal;
		pointWeights.Add(weight);
	}

	/*float isBorder = 0.0f;
	float isWater = 0.0f;
	float isOcean = 0.0f;
	float isCoast = 0.0f;
	float isRiver = 0.0f;*/

	float elevation = 0.0f;
	float moisture = 0.0f;

	TMap<FGameplayTag, float> tagWeights;

	for (int i = 0; i < pointWeights.Num(); i++)
	{
		FMapData curPoint = pointWeights[i].Key;
		float weight = pointWeights[i].Value;

		/*if (UMapDataHelper::IsBorder(curPoint))
		{
			isBorder += weight;
		}
		if (curPoint.bIsCoast)
		{
			isCoast += weight;
		}
		if (curPoint.bIsOcean)
		{
			isOcean += weight;
		}
		if (curPoint.bIsRiver)
		{
			isRiver += weight;
		}
		if (curPoint.bIsWater)
		{
			isWater += weight;
		}*/

		elevation += (curPoint.Elevation * weight);
		moisture += (curPoint.Moisture * weight);

		for (int j = 0; j < curPoint.Tags.Num(); j++)
		{
			FGameplayTag tag = curPoint.Tags.GetByIndex(i);
			float currentTagWeight = tagWeights.FindOrAdd(tag);
			currentTagWeight += weight;
			tagWeights[tag] = currentTagWeight;
		}
	}

	FMapData pixelData;

	pixelData.Point = PixelPosition;
	/*pixelData.bIsBorder = isBorder >= 0.5f;
	pixelData.bIsCoast = isCoast >= 0.5f;
	pixelData.bIsOcean = isOcean >= 0.5f;
	pixelData.bIsRiver = isRiver >= 0.5f;
	pixelData.bIsWater = isWater >= 0.5f;*/

	pixelData.Tags.Reset();
	for (auto& elem : tagWeights)
	{
		if (elem.Value >= 0.5f)
		{
			pixelData.Tags.AddTagFast(elem.Key);
		}
	}

	pixelData.Elevation = elevation;
	pixelData.Moisture = moisture;
	pixelData.Biome = BiomeManager->DetermineBiome(pixelData);
	return pixelData;
}

void UPolygonalMapHeightmap::CreateHeightmap(UPolygonMap* PolygonMap, UBiomeManager* BiomeManager, int32 Size)
{
	if (PolygonMap == NULL)
	{
		return;
	}
	HeightmapSize = Size;
	TArray<FMapData> graph = PolygonMap->GetAllMapData();
	// First, insert a border around the map
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

	// Now, interpolate between the actual points
	for (int x = 0; x < HeightmapSize; x++)
	{
		for (int y = 0; y < HeightmapSize; y++)
		{
			FVector2D point = FVector2D(x, y);
			HeightmapData.Add(MakeMapPoint(point,graph, BiomeManager));
		}
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