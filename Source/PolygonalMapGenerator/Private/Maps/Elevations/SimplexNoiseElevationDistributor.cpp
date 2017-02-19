// Fill out your copyright notice in the Description page of Project Settings.

#include "PolygonalMapGeneratorPrivatePCH.h"
#include "Maps/MapDataHelper.h"
#include "SimplexNoiseElevationDistributor.h"

USimplexNoiseElevationDistributor::USimplexNoiseElevationDistributor()
{
	NoiseSettings.NoiseOctaves = 16;
	NoiseSettings.NoiseType = ENoiseType::Simplex;
}

void USimplexNoiseElevationDistributor::AssignCornerElevations(UIslandShape* islandShape, bool bneedsMoreRandomness, FRandomStream& randomGenerator)
{
	int32 islandSize = islandShape->Size;

	PolygonalMapGenNoise noise;
	NoiseSettings.Seed = randomGenerator.GetCurrentSeed();
	noise.SetNoiseSettings(NoiseSettings);

	for (int i = 0; i < MapGraph->GetCornerNum(); i++)
	{
		FMapCorner corner = MapGraph->GetCorner(i);
		/*noise.SetPersistence(0.5f);
		noise.SetScale(scale);
		noise.SetBounds(-0.25f, 1.0f);*/
		
		float gradient = GetGradient(corner.CornerData.Point.X, corner.CornerData.Point.Y, islandSize);

		float noiseValue = noise.GetNoise(corner.CornerData.Point.X * NoiseScale, corner.CornerData.Point.Y * NoiseScale);
		UE_LOG(LogTemp, Log, TEXT("Noise value: %f"), noiseValue);
		// Multiply by the inverse gradient several times to put a stronger bias in the center
		noiseValue *= gradient;

		corner.CornerData = UMapDataHelper::RemoveCoast(corner.CornerData);
		
		if (noiseValue < OceanThreshold)
		{
			corner.CornerData = UMapDataHelper::SetOcean(corner.CornerData);
		}
		else if(!UMapDataHelper::IsBorder(corner.CornerData))
		{
			corner.CornerData = UMapDataHelper::RemoveOcean(corner.CornerData);
		}

		corner.CornerData.Elevation = noiseValue;
		MapGraph->UpdateCorner(corner);
	}
}

/*void USimplexNoiseElevationDistributor::AssignOceanCoastAndLand(float lakeThreshold)
{

}*/

void USimplexNoiseElevationDistributor::RedistributeElevations(TArray<int32> landCorners)
{

}

float USimplexNoiseElevationDistributor::GetGradient(float XCoord, float YCoord, int32 IslandSize) const
{
	if (GradientIterations <= 0)
	{
		return 1.0f;
	}
	float distanceX = FMath::Abs(XCoord - IslandSize * 0.5f);
	float distanceY = FMath::Abs(YCoord - IslandSize * 0.5f);
	float distance = FMath::Sqrt(distanceX * distanceX + distanceY * distanceY); // circular mask

	float maxWidth = IslandSize * 0.5f - 10.0f;
	float delta = distance / maxWidth;
	float gradient = delta * delta;

	float inverseGradient = FMath::Max(0.0f, 1.0f - gradient);

	gradient = 1.0f;
	for (int i = 0; i < GradientIterations; i++)
	{
		gradient *= inverseGradient;
	}
	return gradient;
}

/*void USimplexNoiseElevationDistributor::FlattenWaterElevations()
{

}*/
