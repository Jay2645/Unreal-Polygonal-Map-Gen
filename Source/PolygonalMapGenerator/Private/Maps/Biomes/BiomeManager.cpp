// Original Work Copyright (c) 2010 Amit J Patel, Modified Work Copyright (c) 2016 Jay M Stevens

#include "PolygonalMapGeneratorPrivatePCH.h"
#include "Maps/MapDataHelper.h"
#include "BiomeManager.h"

const FName UBiomeManager::BIOME_OceanTag = TEXT("MapData.Biome.Water.Ocean");
const FName UBiomeManager::BIOME_CoastTag = TEXT("MapData.Biome.Coast");

// Sets default values for this component's properties
UBiomeManager::UBiomeManager()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	FBiome oceanBiome;
	oceanBiome.BiomeTag = FGameplayTag::RequestGameplayTag(BIOME_OceanTag);
	oceanBiome.RequiredTags.AddTagFast(FGameplayTag::RequestGameplayTag(TEXT("MapData.MetaData.Water.Saltwater")));
	Biomes.Add(oceanBiome);

	FBiome coastalBiome;
	coastalBiome.BiomeTag = FGameplayTag::RequestGameplayTag(BIOME_CoastTag);
	coastalBiome.RequiredTags.AddTagFast(FGameplayTag::RequestGameplayTag(TEXT("MapData.MetaData.Coast")));
	Biomes.Add(coastalBiome);
}

//FGameplayTag UBiomeManager::DetermineBiome_Implementation(const FMapData& MapData)
FGameplayTag UBiomeManager::DetermineBiome(const FMapData& MapData)
{
	//UE_LOG(LogWorldGen, Warning, TEXT("Beginning new point: %f, %f"), MapData.Point.X, MapData.Point.Y); 
	for (int i = 0; i < Biomes.Num(); i++)
	{
		// If we don't allow snow and this is a frozen biome, continue
		if (!bAllowSnow && Biomes[i].BiomeTag.MatchesTag(FGameplayTag::RequestGameplayTag(TEXT("MapData.Biome.Frozen"))))
		{
			//UE_LOG(LogWorldGen, Log, TEXT("Passing on %s because we don't allow snow."), *Biomes[i].BiomeTag.ToString());
			continue;
		}

		// Verify that the biome is in the right elevation with the right moisture level
		bool bSettingsOkay = Biomes[i].BiomeSettings.Num() == 0;
		if(!bSettingsOkay)
		{
			for(int j = 0; j < Biomes[i].BiomeSettings.Num(); j++)
			{
				if (MapData.Elevation <= Biomes[i].BiomeSettings[j].MaxElevation && MapData.Elevation >= Biomes[i].BiomeSettings[j].MinElevation &&
					MapData.Moisture <= Biomes[i].BiomeSettings[j].MaxMoisture && MapData.Moisture >= Biomes[i].BiomeSettings[j].MinMoisture)
				{
					bSettingsOkay = true;
					break;
				}
			}
		}
		if (!bSettingsOkay)
		{
			//UE_LOG(LogWorldGen, Log, TEXT("Passing on %s because the map data's elevation (%f) or moisture (%f) is out of range."), *Biomes[i].BiomeTag.ToString(), MapData.Elevation, MapData.Moisture);
			continue;
		}

		// Verify that the biome has the required tags
		bool bHasRequiredTags = Biomes[i].RequiredTags.Num() == 0;

		if(!bHasRequiredTags)
		{
			// Reset the required tags flag
			bHasRequiredTags = true;
			// Iterate over all required tags and see if we have them all
			for (int j = 0; j < Biomes[i].RequiredTags.Num(); j++)
			{
				if (!UMapDataHelper::HasTag(MapData, Biomes[i].RequiredTags.GetByIndex(j)))
				{
					// We're missing one, so we need to move on

					//UE_LOG(LogWorldGen, Log, TEXT("Passing on %s because the map data struct is missing the tag %s."), *Biomes[i].BiomeTag.ToString(), *Biomes[i].RequiredTags.GetByIndex(j).ToString());
					bHasRequiredTags = false;
					break;
				}
			}
		}
		if (bHasRequiredTags)
		{
			//UE_LOG(LogWorldGen, Warning, TEXT("Selected biome %s!"), *Biomes[i].BiomeTag.ToString());
			return Biomes[i].BiomeTag;
		}
	}
	UE_LOG(LogWorldGen, Error, TEXT("A MapData point was missing a biome!"));
	return FGameplayTag::EmptyTag;
}