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
	BiomeEnumMap.Add(oceanBiome.BiomeTag, EWhittakerBiome::Ocean);

	FBiome iceBiome;
	iceBiome.BiomeTag = FGameplayTag::RequestGameplayTag(BIOME_IceTag);
	iceBiome.RequiredTags.AddTagFast(FGameplayTag::RequestGameplayTag(TEXT("MapData.MetaData.Water.Freshwater")));
	iceBiome.MinElevation = 0.75f;
	Biomes.Add(iceBiome);
	BiomeEnumMap.Add(iceBiome.BiomeTag, EWhittakerBiome::Ice);

	FBiome marshBiome;
	marshBiome.BiomeTag = FGameplayTag::RequestGameplayTag(BIOME_MarshTag);
	marshBiome.RequiredTags.AddTagFast(FGameplayTag::RequestGameplayTag(TEXT("MapData.MetaData.Water.Freshwater")));
	marshBiome.MaxElevation = 0.25f;
	Biomes.Add(marshBiome);
	BiomeEnumMap.Add(marshBiome.BiomeTag, EWhittakerBiome::Marsh);

	FBiome lakeBiome;
	lakeBiome.BiomeTag = FGameplayTag::RequestGameplayTag(BIOME_LakeTag);
	lakeBiome.RequiredTags.AddTagFast(FGameplayTag::RequestGameplayTag(TEXT("MapData.MetaData.Water.Freshwater")));
	lakeBiome.MinElevation = 0.25f;
	lakeBiome.MaxElevation = 0.75f;
	Biomes.Add(lakeBiome);
	BiomeEnumMap.Add(lakeBiome.BiomeTag, EWhittakerBiome::Lake);

	FBiome coastalBiome;
	coastalBiome.BiomeTag = FGameplayTag::RequestGameplayTag(BIOME_CoastTag);
	coastalBiome.RequiredTags.AddTagFast(FGameplayTag::RequestGameplayTag(TEXT("MapData.MetaData.Coast")));
	Biomes.Add(coastalBiome);
	BiomeEnumMap.Add(coastalBiome.BiomeTag, EWhittakerBiome::Coast);

	FBiome snowBome;
	snowBome.BiomeTag = FGameplayTag::RequestGameplayTag(BIOME_SnowTag);
	snowBome.MinElevation = 0.75f;
	snowBome.MinMoisture = 0.5f;
	Biomes.Add(snowBome);
	BiomeEnumMap.Add(snowBome.BiomeTag, EWhittakerBiome::Snow);

	FBiome tundraBiome;
	tundraBiome.BiomeTag = FGameplayTag::RequestGameplayTag(BIOME_TundraTag);
	tundraBiome.MinElevation = 0.75f;
	tundraBiome.MaxMoisture = 0.5f;
	tundraBiome.MinMoisture = 0.33f;
	Biomes.Add(tundraBiome);
	BiomeEnumMap.Add(tundraBiome.BiomeTag, EWhittakerBiome::Tundra);

	FBiome bareBiome;
	bareBiome.BiomeTag = FGameplayTag::RequestGameplayTag(BIOME_BareTag);
	bareBiome.MinElevation = 0.75f;
	bareBiome.MinMoisture = 0.16f;
	Biomes.Add(bareBiome);
	BiomeEnumMap.Add(bareBiome.BiomeTag, EWhittakerBiome::Bare);

	FBiome scorchedBiome;
	scorchedBiome.BiomeTag = FGameplayTag::RequestGameplayTag(BIOME_ScorchedTag);
	scorchedBiome.MinElevation = 0.75f;
	scorchedBiome.MaxMoisture = 0.16f;
	Biomes.Add(scorchedBiome);
	BiomeEnumMap.Add(scorchedBiome.BiomeTag, EWhittakerBiome::Scorched);

	FBiome taigaBiome;
	taigaBiome.BiomeTag = FGameplayTag::RequestGameplayTag(BIOME_TaigaTag);
	taigaBiome.MaxElevation = 0.75f;
	taigaBiome.MinElevation = 0.5f;
	taigaBiome.MinMoisture = 0.66f;
	Biomes.Add(taigaBiome);
	BiomeEnumMap.Add(taigaBiome.BiomeTag, EWhittakerBiome::Taiga);

	FBiome shrublandBiome;
	shrublandBiome.BiomeTag = FGameplayTag::RequestGameplayTag(BIOME_ShrublandTag);
	shrublandBiome.MaxElevation = 0.75f;
	shrublandBiome.MinElevation = 0.5f;
	shrublandBiome.MinMoisture = 0.33f;
	Biomes.Add(shrublandBiome);
	BiomeEnumMap.Add(shrublandBiome.BiomeTag, EWhittakerBiome::Shrubland);

	FBiome temperateDesertBiomeLow;
	temperateDesertBiomeLow.BiomeTag = FGameplayTag::RequestGameplayTag(BIOME_TemperateDesertTag);
	temperateDesertBiomeLow.MaxElevation = 0.5f;
	temperateDesertBiomeLow.MinElevation = 0.25f;
	temperateDesertBiomeLow.MaxMoisture = 0.16f;
	Biomes.Add(temperateDesertBiomeLow);
	BiomeEnumMap.Add(temperateDesertBiomeLow.BiomeTag, EWhittakerBiome::TemperateDesert);

	FBiome temperateDesertBiomeHigh;
	temperateDesertBiomeHigh.BiomeTag = FGameplayTag::RequestGameplayTag(BIOME_TemperateDesertTag);
	temperateDesertBiomeHigh.MaxElevation = 0.75f;
	temperateDesertBiomeHigh.MinElevation = 0.5f;
	temperateDesertBiomeHigh.MaxMoisture = 0.33f;
	Biomes.Add(temperateDesertBiomeHigh);

	FBiome temperateRainForestBiome;
	temperateRainForestBiome.BiomeTag = FGameplayTag::RequestGameplayTag(BIOME_TemperateRainForestTag);
	temperateRainForestBiome.MaxElevation = 0.5f;
	temperateRainForestBiome.MinElevation = 0.25f;
	temperateRainForestBiome.MinMoisture = 0.83f;
	Biomes.Add(temperateRainForestBiome);
	BiomeEnumMap.Add(temperateRainForestBiome.BiomeTag, EWhittakerBiome::TemperateRainForest);

	FBiome temperateDeciduousForestBiome;
	temperateDeciduousForestBiome.BiomeTag = FGameplayTag::RequestGameplayTag(BIOME_TemperateDeciduousForestTag);
	temperateDeciduousForestBiome.MaxElevation = 0.5f;
	temperateDeciduousForestBiome.MinElevation = 0.25f;
	temperateDeciduousForestBiome.MaxMoisture = 0.83f;
	temperateDeciduousForestBiome.MinMoisture = 0.5f;
	Biomes.Add(temperateDeciduousForestBiome);
	BiomeEnumMap.Add(temperateDeciduousForestBiome.BiomeTag, EWhittakerBiome::TemperateDeciduousForest);

	FBiome grasslandBiomeHigh;
	grasslandBiomeHigh.BiomeTag = FGameplayTag::RequestGameplayTag(BIOME_GrasslandTag);
	grasslandBiomeHigh.MaxElevation = 0.5f;
	grasslandBiomeHigh.MinElevation = 0.25f;
	grasslandBiomeHigh.MaxMoisture = 0.5f;
	grasslandBiomeHigh.MinMoisture = 0.16f;
	Biomes.Add(grasslandBiomeHigh);
	BiomeEnumMap.Add(grasslandBiomeHigh.BiomeTag, EWhittakerBiome::Grassland);

	FBiome grasslandBiomeLow;
	grasslandBiomeLow.BiomeTag = FGameplayTag::RequestGameplayTag(BIOME_GrasslandTag);
	grasslandBiomeLow.MaxElevation = 0.25f;
	grasslandBiomeLow.MaxMoisture = 0.33f;
	grasslandBiomeLow.MinMoisture = 0.16f;
	Biomes.Add(grasslandBiomeLow);

	FBiome tropicalRainForestBiome;
	tropicalRainForestBiome.BiomeTag = FGameplayTag::RequestGameplayTag(BIOME_TropicalRainForestTag);
	tropicalRainForestBiome.MaxElevation = 0.25f;
	tropicalRainForestBiome.MinMoisture = 0.66f;
	Biomes.Add(tropicalRainForestBiome);
	BiomeEnumMap.Add(tropicalRainForestBiome.BiomeTag, EWhittakerBiome::TropicalRainForest);

	FBiome tropicalSeasonalForestBiome;
	tropicalSeasonalForestBiome.BiomeTag = FGameplayTag::RequestGameplayTag(BIOME_TropicalSeasonalForestTag);
	tropicalSeasonalForestBiome.MaxElevation = 0.25f;
	tropicalSeasonalForestBiome.MaxMoisture = 0.66f;
	tropicalSeasonalForestBiome.MinMoisture = 0.33f;
	Biomes.Add(tropicalSeasonalForestBiome);
	BiomeEnumMap.Add(tropicalSeasonalForestBiome.BiomeTag, EWhittakerBiome::TropicalSeasonalForest);

	FBiome subtropicalDesertBiome;
	subtropicalDesertBiome.BiomeTag = FGameplayTag::RequestGameplayTag(BIOME_SubtropicalDesertTag);
	subtropicalDesertBiome.MaxElevation = 0.25f;
	subtropicalDesertBiome.MaxMoisture = 0.33f;
	Biomes.Add(subtropicalDesertBiome);
	BiomeEnumMap.Add(subtropicalDesertBiome.BiomeTag, EWhittakerBiome::SubtropicalDesert);
}

EWhittakerBiome UWhittakerBiomeManager::ConvertToWhittakerBiomeEnum(const FGameplayTag& BiomeName)
{
	if (BiomeEnumMap.Contains(BiomeName))
	{
		return BiomeEnumMap[BiomeName];
	}
	else
	{
		UE_LOG(LogWorldGen, Warning, TEXT("Could not find biome for %s!"), *BiomeName.ToString());
		return EWhittakerBiome::Ocean;
	}
}