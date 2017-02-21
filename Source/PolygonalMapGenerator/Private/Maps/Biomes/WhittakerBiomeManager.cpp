// Original Work Copyright (c) 2010 Amit J Patel, Modified Work Copyright (c) 2017 Jay M Stevens

#include "PolygonalMapGeneratorPrivatePCH.h"
#include "WhittakerBiomeManager.h"

const FName UWhittakerBiomeManager::BIOME_IceTag = TEXT("MapData.Biome.Frozen.Ice");
const FName UWhittakerBiomeManager::BIOME_MarshTag = TEXT("MapData.Biome.Water.Marsh");
const FName UWhittakerBiomeManager::BIOME_LakeTag = TEXT("MapData.Biome.Water.Lake");
const FName UWhittakerBiomeManager::BIOME_SnowTag = TEXT("MapData.Biome.Frozen.Snow");
const FName UWhittakerBiomeManager::BIOME_TundraTag = TEXT("MapData.Biome.Frozen.Tundra");
const FName UWhittakerBiomeManager::BIOME_BareTag = TEXT("MapData.Biome.Bare");
const FName UWhittakerBiomeManager::BIOME_ScorchedTag = TEXT("MapData.Biome.Scorched");
const FName UWhittakerBiomeManager::BIOME_TaigaTag = TEXT("MapData.Biome.Frozen.Taiga");
const FName UWhittakerBiomeManager::BIOME_ShrublandTag = TEXT("MapData.Biome.Shrubland");
const FName UWhittakerBiomeManager::BIOME_TemperateDesertTag = TEXT("MapData.Biome.TemperateDesert");
const FName UWhittakerBiomeManager::BIOME_TemperateRainForestTag = TEXT("MapData.Biome.TemperateRainForest");
const FName UWhittakerBiomeManager::BIOME_TemperateDeciduousForestTag = TEXT("MapData.Biome.TemperateDeciduousForest");
const FName UWhittakerBiomeManager::BIOME_GrasslandTag = TEXT("MapData.Biome.Grassland");
const FName UWhittakerBiomeManager::BIOME_TropicalRainForestTag = TEXT("MapData.Biome.TropicalRainForest");
const FName UWhittakerBiomeManager::BIOME_TropicalSeasonalForestTag = TEXT("MapData.Biome.TropicalSeasonalForest");
const FName UWhittakerBiomeManager::BIOME_SubtropicalDesertTag = TEXT("MapData.Biome.SubtropicalDesert");

UWhittakerBiomeManager::UWhittakerBiomeManager()
{
	Biomes.Empty();

	FBiome oceanBiome;
	oceanBiome.BiomeTag = FGameplayTag::RequestGameplayTag(BIOME_OceanTag);
	oceanBiome.RequiredTags.AddTagFast(FGameplayTag::RequestGameplayTag(TEXT("MapData.MetaData.Water.Saltwater")));
	Biomes.Add(oceanBiome);

	FBiome iceBiome;
	iceBiome.BiomeTag = FGameplayTag::RequestGameplayTag(BIOME_IceTag);
	iceBiome.RequiredTags.AddTagFast(FGameplayTag::RequestGameplayTag(TEXT("MapData.MetaData.Water.Freshwater")));
	FBiomeSettings iceBiomeSettings;
	iceBiomeSettings.MinElevation = 0.75f;
	iceBiome.BiomeSettings.Add(iceBiomeSettings);
	Biomes.Add(iceBiome);

	FBiome marshBiome;
	marshBiome.BiomeTag = FGameplayTag::RequestGameplayTag(BIOME_MarshTag);
	marshBiome.RequiredTags.AddTagFast(FGameplayTag::RequestGameplayTag(TEXT("MapData.MetaData.Water.Freshwater")));
	FBiomeSettings marshBiomeSettings;
	marshBiomeSettings.MaxElevation = 0.25f;
	marshBiome.BiomeSettings.Add(marshBiomeSettings);
	Biomes.Add(marshBiome);

	FBiome lakeBiome;
	lakeBiome.BiomeTag = FGameplayTag::RequestGameplayTag(BIOME_LakeTag);
	lakeBiome.RequiredTags.AddTagFast(FGameplayTag::RequestGameplayTag(TEXT("MapData.MetaData.Water.Freshwater")));
	FBiomeSettings lakeBiomeSettings;
	lakeBiomeSettings.MinElevation = 0.25f;
	lakeBiomeSettings.MaxElevation = 0.75f;
	lakeBiome.BiomeSettings.Add(lakeBiomeSettings);
	Biomes.Add(lakeBiome);

	FBiome coastalBiome;
	coastalBiome.BiomeTag = FGameplayTag::RequestGameplayTag(BIOME_CoastTag);
	coastalBiome.RequiredTags.AddTagFast(FGameplayTag::RequestGameplayTag(TEXT("MapData.MetaData.Coast")));
	Biomes.Add(coastalBiome);

	FBiome snowBome;
	snowBome.BiomeTag = FGameplayTag::RequestGameplayTag(BIOME_SnowTag);
	FBiomeSettings snowBiomeSettings;
	snowBiomeSettings.MinElevation = 0.75f;
	snowBiomeSettings.MinMoisture = 0.5f;
	snowBome.BiomeSettings.Add(snowBiomeSettings);
	Biomes.Add(snowBome);

	FBiome tundraBiome;
	tundraBiome.BiomeTag = FGameplayTag::RequestGameplayTag(BIOME_TundraTag);
	FBiomeSettings tundraBiomeSettings;
	tundraBiomeSettings.MinElevation = 0.75f;
	tundraBiomeSettings.MaxMoisture = 0.5f;
	tundraBiomeSettings.MinMoisture = 0.33f;
	tundraBiome.BiomeSettings.Add(tundraBiomeSettings);
	Biomes.Add(tundraBiome);

	FBiome bareBiome;
	bareBiome.BiomeTag = FGameplayTag::RequestGameplayTag(BIOME_BareTag);
	FBiomeSettings bareBiomeSettings;
	bareBiomeSettings.MinElevation = 0.75f;
	bareBiomeSettings.MinMoisture = 0.16f;
	bareBiome.BiomeSettings.Add(bareBiomeSettings);
	Biomes.Add(bareBiome);

	FBiome scorchedBiome;
	scorchedBiome.BiomeTag = FGameplayTag::RequestGameplayTag(BIOME_ScorchedTag);
	FBiomeSettings scorchedBiomeSettings;
	scorchedBiomeSettings.MinElevation = 0.75f;
	scorchedBiomeSettings.MaxMoisture = 0.16f;
	scorchedBiome.BiomeSettings.Add(scorchedBiomeSettings);
	Biomes.Add(scorchedBiome);

	FBiome taigaBiome;
	taigaBiome.BiomeTag = FGameplayTag::RequestGameplayTag(BIOME_TaigaTag);
	FBiomeSettings taigaBiomeSettings;
	taigaBiomeSettings.MaxElevation = 0.75f;
	taigaBiomeSettings.MinElevation = 0.5f;
	taigaBiomeSettings.MinMoisture = 0.66f;
	taigaBiome.BiomeSettings.Add(taigaBiomeSettings);
	Biomes.Add(taigaBiome);

	FBiome shrublandBiome;
	shrublandBiome.BiomeTag = FGameplayTag::RequestGameplayTag(BIOME_ShrublandTag);
	FBiomeSettings shrublandBiomeSettings;
	shrublandBiomeSettings.MaxElevation = 0.75f;
	shrublandBiomeSettings.MinElevation = 0.5f;
	shrublandBiomeSettings.MinMoisture = 0.33f;
	shrublandBiome.BiomeSettings.Add(shrublandBiomeSettings);
	Biomes.Add(shrublandBiome);

	FBiome temperateDesertBiome;
	temperateDesertBiome.BiomeTag = FGameplayTag::RequestGameplayTag(BIOME_TemperateDesertTag);
	FBiomeSettings temperateDesertBiomeHighSettings;
	temperateDesertBiomeHighSettings.MaxElevation = 0.75f;
	temperateDesertBiomeHighSettings.MinElevation = 0.5f;
	temperateDesertBiomeHighSettings.MaxMoisture = 0.33f;
	temperateDesertBiome.BiomeSettings.Add(temperateDesertBiomeHighSettings);
	FBiomeSettings temperateDesertBiomeLowSettings;
	temperateDesertBiomeLowSettings.MaxElevation = 0.5f;
	temperateDesertBiomeLowSettings.MinElevation = 0.25f;
	temperateDesertBiomeLowSettings.MaxMoisture = 0.16f;
	temperateDesertBiome.BiomeSettings.Add(temperateDesertBiomeLowSettings);
	Biomes.Add(temperateDesertBiome);

	FBiome temperateRainForestBiome;
	temperateRainForestBiome.BiomeTag = FGameplayTag::RequestGameplayTag(BIOME_TemperateRainForestTag);
	FBiomeSettings temperateRainForestBiomeSettings;
	temperateRainForestBiomeSettings.MaxElevation = 0.5f;
	temperateRainForestBiomeSettings.MinElevation = 0.25f;
	temperateRainForestBiomeSettings.MinMoisture = 0.83f;
	temperateRainForestBiome.BiomeSettings.Add(temperateRainForestBiomeSettings);
	Biomes.Add(temperateRainForestBiome);

	FBiome temperateDeciduousForestBiome;
	temperateDeciduousForestBiome.BiomeTag = FGameplayTag::RequestGameplayTag(BIOME_TemperateDeciduousForestTag);
	FBiomeSettings temperateDeciduousForestBiomeSettings;
	temperateDeciduousForestBiomeSettings.MaxElevation = 0.5f;
	temperateDeciduousForestBiomeSettings.MinElevation = 0.25f;
	temperateDeciduousForestBiomeSettings.MaxMoisture = 0.83f;
	temperateDeciduousForestBiomeSettings.MinMoisture = 0.5f;
	temperateDeciduousForestBiome.BiomeSettings.Add(temperateDeciduousForestBiomeSettings);
	Biomes.Add(temperateDeciduousForestBiome);

	FBiome grasslandBiome;
	grasslandBiome.BiomeTag = FGameplayTag::RequestGameplayTag(BIOME_GrasslandTag);
	FBiomeSettings grasslandBiomeHighSettings;
	grasslandBiomeHighSettings.MaxElevation = 0.5f;
	grasslandBiomeHighSettings.MinElevation = 0.25f;
	grasslandBiomeHighSettings.MaxMoisture = 0.5f;
	grasslandBiomeHighSettings.MinMoisture = 0.16f;
	grasslandBiome.BiomeSettings.Add(grasslandBiomeHighSettings);
	FBiomeSettings grasslandBiomeLowSettings;
	grasslandBiomeLowSettings.MaxElevation = 0.25f;
	grasslandBiomeLowSettings.MaxMoisture = 0.33f;
	grasslandBiomeLowSettings.MinMoisture = 0.16f;
	grasslandBiome.BiomeSettings.Add(grasslandBiomeLowSettings);
	Biomes.Add(grasslandBiome);

	FBiome tropicalRainForestBiome;
	tropicalRainForestBiome.BiomeTag = FGameplayTag::RequestGameplayTag(BIOME_TropicalRainForestTag);
	FBiomeSettings tropicalRainForestBiomeSettings;
	tropicalRainForestBiomeSettings.MaxElevation = 0.25f;
	tropicalRainForestBiomeSettings.MinMoisture = 0.66f;
	tropicalRainForestBiome.BiomeSettings.Add(tropicalRainForestBiomeSettings);
	Biomes.Add(tropicalRainForestBiome);

	FBiome tropicalSeasonalForestBiome;
	tropicalSeasonalForestBiome.BiomeTag = FGameplayTag::RequestGameplayTag(BIOME_TropicalSeasonalForestTag);
	FBiomeSettings tropicalSeasonalForestBiomeSettings;
	tropicalSeasonalForestBiomeSettings.MaxElevation = 0.25f;
	tropicalSeasonalForestBiomeSettings.MaxMoisture = 0.66f;
	tropicalSeasonalForestBiomeSettings.MinMoisture = 0.33f;
	tropicalSeasonalForestBiome.BiomeSettings.Add(tropicalSeasonalForestBiomeSettings);
	Biomes.Add(tropicalSeasonalForestBiome);

	FBiome subtropicalDesertBiome;
	subtropicalDesertBiome.BiomeTag = FGameplayTag::RequestGameplayTag(BIOME_SubtropicalDesertTag);
	FBiomeSettings subtropicalDesertBiomeSettings;
	subtropicalDesertBiomeSettings.MaxElevation = 0.25f;
	subtropicalDesertBiomeSettings.MaxMoisture = 0.33f;
	subtropicalDesertBiome.BiomeSettings.Add(subtropicalDesertBiomeSettings);
	Biomes.Add(subtropicalDesertBiome);
}

EWhittakerBiome UWhittakerBiomeManager::ConvertToWhittakerBiomeEnum(const FGameplayTag& BiomeName)
{
	for (int i = 0; i < Biomes.Num(); i++)
	{
		if (Biomes[i].BiomeTag.MatchesTag(BiomeName))
		{
			return (EWhittakerBiome)i;
		}
	}
	UE_LOG(LogWorldGen, Warning, TEXT("Could not find biome for %s!"), *BiomeName.ToString());
	return EWhittakerBiome::Snow;
}