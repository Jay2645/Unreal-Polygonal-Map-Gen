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
#include "IslandMapUtils.h"
#include "RandomSampling/SimplexNoise.h"

void UIslandMapUtils::RandomShuffle(TArray<FTriangleIndex>& OutShuffledArray, FRandomStream& Rng)
{
	for (int i = OutShuffledArray.Num() - 1; i > 0; i--)
	{
		int32 j = Rng.RandRange(0, i);
		int32 swap = OutShuffledArray[i];
		OutShuffledArray[i] = OutShuffledArray[j];
		OutShuffledArray[j] = swap;
	}
}

float UIslandMapUtils::FBMNoise(const TArray<float>& Amplitudes, const FVector2D& Position)
{
	float sum = 0.0f;
	float sumOfAmplitudes = 0.0f;
	FCustomSimplexNoise noise;
	for (size_t octave = 0; octave < Amplitudes.Num(); octave++)
	{
		size_t frequency = ((size_t)1) << octave;
		sum += Amplitudes[octave] * noise.fractal(octave, Position * frequency);
		sumOfAmplitudes += Amplitudes[octave];
	}

	if (sumOfAmplitudes == 0.0f)
	{
		return 0.0f;
	}
	return sum / sumOfAmplitudes;
}

FBiomeData UIslandMapUtils::GetBiome(const UDataTable* BiomeData, bool bIsOcean, bool bIsWater, bool bIsCoast, float Temperature, float Moisture)
{
	if (BiomeData == NULL)
	{
		UE_LOG(LogMapGen, Error, TEXT("Passed in an empty Biome Data table! Can't determine any biomes."));
		return FBiomeData();
	}

	Moisture = FMath::Clamp(Moisture, 0.0f, 1.0f);
	Temperature = FMath::Clamp(Temperature, 0.0f, 1.0f);

	TArray<FBiomeData> possibleBiomes;
	for (auto it : BiomeData->GetRowMap())
	{
		FName rowName = it.Key;
		if (it.Value == NULL)
		{
			// Should never happen
			checkNoEntry();
		}
		possibleBiomes.Add(*((FBiomeData*)it.Value));
	}

	TArray<FBiomeData> newPossibleBiomes;

	// Check ocean
	if (bIsOcean)
	{
		for (FBiomeData biome : possibleBiomes)
		{
			if (biome.bIsOcean)
			{
				newPossibleBiomes.Add(biome);
			}
		}
		possibleBiomes = newPossibleBiomes;
		if (possibleBiomes.Num() == 1)
		{
			return possibleBiomes[0];
		}
		else if (possibleBiomes.Num() == 0)
		{
			UE_LOG(LogMapGen, Error, TEXT("Could not find any ocean biomes!"));
			return FBiomeData();
		}
		// Recycle the new biomes array
		newPossibleBiomes.Empty();
	}
	else
	{
		for (FBiomeData biome : possibleBiomes)
		{
			if (!biome.bIsOcean)
			{
				newPossibleBiomes.Add(biome);
			}
		}
		possibleBiomes = newPossibleBiomes;
		if (possibleBiomes.Num() == 1)
		{
			return possibleBiomes[0];
		}
		else if (possibleBiomes.Num() == 0)
		{
			UE_LOG(LogMapGen, Error, TEXT("Could not find any ocean biomes!"));
			return FBiomeData();
		}
		// Recycle the new biomes array
		newPossibleBiomes.Empty();
	}

	// Check water
	if (bIsWater)
	{
		for (FBiomeData biome : possibleBiomes)
		{
			if (biome.bIsWater)
			{
				newPossibleBiomes.Add(biome);
			}
		}
		possibleBiomes = newPossibleBiomes;
		if (possibleBiomes.Num() == 1)
		{
			return possibleBiomes[0];
		}
		else if (possibleBiomes.Num() == 0)
		{
			UE_LOG(LogMapGen, Error, TEXT("Could not find any water biomes!"));
			return FBiomeData();
		}
		newPossibleBiomes.Empty();
	}
	else
	{
		for (FBiomeData biome : possibleBiomes)
		{
			if (!biome.bIsWater)
			{
				newPossibleBiomes.Add(biome);
			}
		}
		possibleBiomes = newPossibleBiomes;
		if (possibleBiomes.Num() == 1)
		{
			return possibleBiomes[0];
		}
		else if (possibleBiomes.Num() == 0)
		{
			UE_LOG(LogMapGen, Error, TEXT("Could not find any water biomes!"));
			return FBiomeData();
		}
		newPossibleBiomes.Empty();
	}

	// Check coast
	if (bIsCoast)
	{
		for (FBiomeData biome : possibleBiomes)
		{
			if (biome.bIsCoast)
			{
				newPossibleBiomes.Add(biome);
			}
		}
		possibleBiomes = newPossibleBiomes;
		if (possibleBiomes.Num() == 1)
		{
			return possibleBiomes[0];
		}
		else if (possibleBiomes.Num() == 0)
		{
			UE_LOG(LogMapGen, Error, TEXT("Could not find any coastal biomes!"));
			return FBiomeData();
		}
		newPossibleBiomes.Empty();
	}
	else
	{
		for (FBiomeData biome : possibleBiomes)
		{
			if (!biome.bIsCoast)
			{
				newPossibleBiomes.Add(biome);
			}
		}
		possibleBiomes = newPossibleBiomes;
		if (possibleBiomes.Num() == 1)
		{
			return possibleBiomes[0];
		}
		else if (possibleBiomes.Num() == 0)
		{
			UE_LOG(LogMapGen, Error, TEXT("Could not find any coastal biomes!"));
			return FBiomeData();
		}
		newPossibleBiomes.Empty();
	}
	
	// Check moisture
	for (FBiomeData biome : possibleBiomes)
	{
		if ((biome.MinMoisture < Moisture || biome.MinMoisture == 0.0f && Moisture == 0.0f) && biome.MaxMoisture >= Moisture)
		{
			newPossibleBiomes.Add(biome);
		}
	}
	possibleBiomes = newPossibleBiomes;
	if (possibleBiomes.Num() == 1)
	{
		return possibleBiomes[0];
	}
	else if (possibleBiomes.Num() == 0)
	{
		UE_LOG(LogMapGen, Error, TEXT("Could not find any biomes with moisture %f!"), Moisture);
		return FBiomeData();
	}
	newPossibleBiomes.Empty();

	// Check temperature
	for (FBiomeData biome : possibleBiomes)
	{
		if ((biome.MinTemperature < Temperature || biome.MinTemperature == 0.0f && Temperature == 0.0f) && biome.MaxTemperature >= Temperature)
		{
			newPossibleBiomes.Add(biome);
		}
	}
	possibleBiomes = newPossibleBiomes;
	if (possibleBiomes.Num() == 1)
	{
		return possibleBiomes[0];
	}
	else if (possibleBiomes.Num() == 0)
	{
		UE_LOG(LogMapGen, Error, TEXT("Could not find any biomes with moisture %f!"), Moisture);
		return FBiomeData();
	}

	UE_LOG(LogMapGen, Warning, TEXT("Had %d possible candidates for temperature %f and moisture %f."), possibleBiomes.Num(), Temperature, Moisture);

	return possibleBiomes[0];
}
