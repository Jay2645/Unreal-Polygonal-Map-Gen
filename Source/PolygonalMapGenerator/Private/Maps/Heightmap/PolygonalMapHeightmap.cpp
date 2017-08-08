// Original Work Copyright (c) 2010 Amit J Patel, Modified Work Copyright (c) 2016 Jay M Stevens

#include "PolygonalMapGeneratorPrivatePCH.h"
#include "Maps/MapDataHelper.h"
#include "Biomes/BiomeManager.h"
#include "Moisture/MoistureDistributor.h"
#include "Maps/Heightmap/HeightmapPointTask.h"
#include "PolygonalMapHeightmap.h"

void UPolygonalMapHeightmap::CreateHeightmap(UPolygonMap* PolygonMap, UBiomeManager* BiomeMngr, UMoistureDistributor* MoistureDist, const FHeightmapCreationData HeightmapCreationOptions, const FIslandGeneratorDelegate OnComplete)
{
	if (PolygonMap == NULL)
	{
		return;
	}
	BiomeManager = BiomeMngr;
	MoistureDistributor = MoistureDist;

	HeightmapProperties = HeightmapCreationOptions;
	HeightmapSize = HeightmapProperties.Size;

	HeightmapData.Empty();
	
	OnGenerationComplete = OnComplete;

	// Interpolate between the actual points
	CreateHeightmapTimer = FPlatformTime::Seconds();
	if (HeightmapCreationOptions.HeightmapGenerationPriority == EHeightmapGenerationType::Background)
	{
		FIslandGeneratorDelegate generatePoints;
		generatePoints.BindDynamic(this, &UPolygonalMapHeightmap::CheckMapPointsDone);
		FHeightmapPointGenerator::GenerateHeightmapPoints(this, PolygonMap, BiomeManager, HeightmapProperties, generatePoints);
	}
	else if (HeightmapCreationOptions.HeightmapGenerationPriority == EHeightmapGenerationType::Foreground)
	{
		FHeightmapPointGenerator::MapScale = (float)PolygonMap->GetGraphSize() / (float)HeightmapSize;
		float squaredHeightmap = (float)HeightmapSize * (float)HeightmapSize;
		float current = 0.0f;
		for (int32 x = 0; x < HeightmapSize; x++)
		{
			for (int32 y = 0; y < HeightmapSize; y++)
			{
				HeightmapData.Add(FHeightmapPointTask::MakeMapPoint(FVector2D(x, y), PolygonMap, BiomeManager, EPointSelectionMode::InterpolatedWithPolygonBiome));
				current++;
				FHeightmapPointGenerator::CompletionPercent = current / squaredHeightmap;
			}
		}
		DoHeightmapPostProcess();
	}
	else
	{
		unimplemented();
	}
}

void UPolygonalMapHeightmap::CheckMapPointsDone()
{
	HeightmapData = FHeightmapPointGenerator::HeightmapData;
	DoHeightmapPostProcess();
}

void UPolygonalMapHeightmap::DoHeightmapPostProcess()
{
	UE_LOG(LogWorldGen, Log, TEXT("%d map points created in %f seconds."), HeightmapSize * HeightmapSize, FPlatformTime::Seconds() - CreateHeightmapTimer);

	// Blur polygon edges
	int blurSteps = HeightmapProperties.PostProcessBlurSteps;
	if (blurSteps > 0)
	{
		CreateHeightmapTimer = FPlatformTime::Seconds();
		TArray<FMapData> blurredData;
		blurredData.SetNumZeroed(HeightmapData.Num());
		for (int x = 0; x < HeightmapSize; x++)
		{
			for (int y = 0; y < HeightmapSize; y++)
			{
				float averageElevation = 0.0f;
				float averageMoisture = 0.0f;
				int iterations = 0;
				for (int xOffset = x - blurSteps; xOffset <= x + blurSteps; xOffset++)
				{
					if (xOffset < 0 || xOffset >= HeightmapSize)
					{
						continue;
					}
					for (int yOffset = y - blurSteps; yOffset <= y + blurSteps; yOffset++)
					{
						if (yOffset < 0 || yOffset >= HeightmapSize)
						{
							continue;
						}
						int32 offsetIndex = xOffset + (yOffset * HeightmapSize);
						averageElevation += HeightmapData[offsetIndex].Elevation;
						averageMoisture += HeightmapData[offsetIndex].Moisture;
						iterations++;
					}
				}
				int32 index = x + (y * HeightmapSize);
				blurredData[index].Elevation = averageElevation / (float)iterations;
				blurredData[index].Moisture = averageMoisture / (float)iterations;
			}
		}
		HeightmapData = blurredData;
		UE_LOG(LogWorldGen, Log, TEXT("Points blurred in %f seconds."), FPlatformTime::Seconds() - CreateHeightmapTimer);
	}

	// Create the biomes
	CreateHeightmapTimer = FPlatformTime::Seconds();
	for (int i = 0; i < HeightmapData.Num(); i++)
	{
		HeightmapData[i].Biome = BiomeManager->DetermineBiome(HeightmapData[i]);
	}
	UE_LOG(LogWorldGen, Log, TEXT("Biomes determined in %f seconds."), FPlatformTime::Seconds() - CreateHeightmapTimer);

	// Add the rivers
	/*CreateHeightmapTimer = FPlatformTime::Seconds();
	for (int i = 0; i < MoistureDistributor->Rivers.Num(); i++)
	{
		MoistureDistributor->Rivers[i]->MoveRiverToHeightmap(this);
	}
	UE_LOG(LogWorldGen, Log, TEXT("Rivers placed in %f seconds."), FPlatformTime::Seconds() - CreateHeightmapTimer);*/

	if (OnGenerationComplete.IsBound())
	{
		OnGenerationComplete.Execute();
		OnGenerationComplete.Unbind();
	}
}

float UPolygonalMapHeightmap::GetCompletionPercent() const
{
	return FHeightmapPointGenerator::CompletionPercent;
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
		UE_LOG(LogWorldGen, Warning, TEXT("Tried to get a pixel at %d, %d, but no pixel was found."), x, y);
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
		UE_LOG(LogWorldGen, Warning, TEXT("Tried to set a pixel at %d, %d, but no pixel was found."), X, Y);
		return;
	}
	else
	{
		HeightmapData[index] = MapData;
	}
}