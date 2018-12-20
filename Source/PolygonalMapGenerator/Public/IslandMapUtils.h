/*
* From http://www.redblobgames.com/maps/mapgen2/
* Original work copyright 2017 Red Blob Games <redblobgames@gmail.com>
* Unreal Engine 4 implementation copyright 2018 Jay Stevens <jaystevens42@gmail.com>
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#pragma once

#include "CoreMinimal.h"
#include "PolygonalMapGenerator.h"
#include "DelaunayHelper.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "IslandMapUtils.generated.h"

USTRUCT(BlueprintType)
struct POLYGONALMAPGENERATOR_API FIslandShape
{
	GENERATED_BODY()
	// How many iterations we should have when smoothing the island.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Points", meta = (ClampMin = "0"))
	int32 Octaves;
	// The ratio of island to water.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Points", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Round;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Points", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Inflate;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Map")
	TArray<float> Amplitudes;

	FIslandShape()
	{
		Octaves = 5;
		Round = 0.5f;
		Inflate = 0.4f;
	}
};

USTRUCT(BlueprintType)
struct POLYGONALMAPGENERATOR_API FBiomeBias
{
	GENERATED_BODY()
		// How much rainfall the island receives. Higher values are wetter.
		UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Map", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
		float Rainfall;
	// How hot the northern part of the island is. Higher values are hotter.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Map", meta = (ClampMin = "-1.5", ClampMax = "1.5"))
		float NorthernTemperature;
	// How hot the southern part of the island is. Higher values are hotter.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Map", meta = (ClampMin = "-1.5", ClampMax = "1.5"))
		float SouthernTemperature;

	FBiomeBias()
	{
		Rainfall = 0.0f;
		NorthernTemperature = 0.0f;
		SouthernTemperature = 0.0f;
	}
};

/**
 * 
 */
UCLASS()
class POLYGONALMAPGENERATOR_API UIslandMapUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	static void RandomShuffle(TArray<FTriangleIndex>& OutShuffledArray, FRandomStream& Rng);
	static float FBMNoise(TArray<float> Amplitudes, FVector2D Position);
};