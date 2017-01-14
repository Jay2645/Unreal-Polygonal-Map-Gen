// Original Work Copyright (c) 2010 Amit J Patel, Modified Work Copyright (c) 2017 Jay M Stevens

#include "PolygonalMapGeneratorPrivatePCH.h"
#include "WhittakerBiomeManager.h"

const FName UWhittakerBiomeManager::BIOME_Ice = TEXT("Ice");
const FName UWhittakerBiomeManager::BIOME_Marsh = TEXT("Marsh");
const FName UWhittakerBiomeManager::BIOME_Lake = TEXT("Lake");
const FName UWhittakerBiomeManager::BIOME_Snow = TEXT("Snow");
const FName UWhittakerBiomeManager::BIOME_Tundra = TEXT("Tundra");
const FName UWhittakerBiomeManager::BIOME_Bare = TEXT("Bare");
const FName UWhittakerBiomeManager::BIOME_Scorched = TEXT("Scorched");
const FName UWhittakerBiomeManager::BIOME_Taiga = TEXT("Taiga");
const FName UWhittakerBiomeManager::BIOME_Shrubland = TEXT("Shrubland");
const FName UWhittakerBiomeManager::BIOME_TemperateDesert = TEXT("Temperate Desert");
const FName UWhittakerBiomeManager::BIOME_TemperateRainForest = TEXT("Temperate Rain Forest");
const FName UWhittakerBiomeManager::BIOME_TemperateDeciduousForest = TEXT("Temperate Deciduous Forest");
const FName UWhittakerBiomeManager::BIOME_Grassland = TEXT("Grassland");
const FName UWhittakerBiomeManager::BIOME_TropicalRainForest = TEXT("Tropical Rain Forest");
const FName UWhittakerBiomeManager::BIOME_TropicalSeasonalForest = TEXT("Tropical Seasonal Forest");
const FName UWhittakerBiomeManager::BIOME_SubtropicalDesert = TEXT("Subtropical Desert");

FName UWhittakerBiomeManager::DetermineBiome_Implementation(const FMapData& MapData)
{
	if (UMapDataHelper::IsOcean(MapData))
	{
		return BIOME_Ocean;
	}
	else if (UMapDataHelper::IsWater(MapData))
	{
		if (MapData.Elevation > 0.75f && bAllowSnow)
		{
			return BIOME_Ice;
		}
		else if(MapData.Elevation < 0.25f)
		{
			return BIOME_Marsh;
		}
		else
		{
			return BIOME_Lake;
		}
	}
	else if (UMapDataHelper::IsCoast(MapData))
	{
		return BIOME_Coast;
	}
	else if (MapData.Elevation > 0.75f)
	{
		if (MapData.Moisture > 0.5f && bAllowSnow)
		{
			return BIOME_Snow;
		}
		else if (MapData.Moisture > 0.33f && bAllowSnow)
		{
			return BIOME_Tundra;
		}
		else if (MapData.Moisture > 0.16f)
		{
			return BIOME_Bare;
		}
		else
		{
			return BIOME_Scorched;
		}
	}
	else if (MapData.Elevation > 0.5f)
	{
		if (MapData.Moisture > 0.66f && bAllowSnow)
		{
			return BIOME_Taiga;
		}
		else if (MapData.Moisture > 0.33f)
		{
			return BIOME_Shrubland;
		}
		else
		{
			return BIOME_TemperateDesert;
		}
	}
	else if (MapData.Elevation > 0.25f)
	{
		if (MapData.Moisture > 0.83f)
		{
			return BIOME_TemperateRainForest;
		}
		else if (MapData.Moisture > 0.5f)
		{
			return BIOME_TemperateDeciduousForest;
		}
		else if (MapData.Moisture > 0.16f)
		{
			return BIOME_Grassland;
		}
		else
		{
			return BIOME_TemperateDesert;
		}
	}
	else
	{
		if (MapData.Moisture > 0.66f)
		{
			return BIOME_TropicalRainForest;
		}
		else if (MapData.Moisture > 0.33f)
		{
			return BIOME_TropicalSeasonalForest;
		}
		else if (MapData.Moisture > 0.16f)
		{
			return BIOME_Grassland;
		}
		else
		{
			return BIOME_SubtropicalDesert;
		}
	}
}

EWhittakerBiome UWhittakerBiomeManager::ConvertToWhittakerBiomeEnum(const FName& BiomeName)
{
	if (BiomeName == BIOME_Ocean)
	{
		return EWhittakerBiome::Ocean;
	}
	else if (BiomeName == BIOME_Coast)
	{
		return EWhittakerBiome::Coast;
	}
	else if (BiomeName == BIOME_Bare)
	{
		return EWhittakerBiome::Bare;
	}
	else if (BiomeName == BIOME_Grassland)
	{
		return EWhittakerBiome::Grassland;
	}
	else if (BiomeName == BIOME_Ice)
	{
		return EWhittakerBiome::Ice;
	}
	else if (BiomeName == BIOME_Lake)
	{
		return EWhittakerBiome::Lake;
	}
	else if (BiomeName == BIOME_Marsh)
	{
		return EWhittakerBiome::Marsh;
	}
	else if (BiomeName == BIOME_Scorched)
	{
		return EWhittakerBiome::Scorched;
	}
	else if (BiomeName == BIOME_Shrubland)
	{
		return EWhittakerBiome::Shrubland;
	}
	else if (BiomeName == BIOME_Snow)
	{
		return EWhittakerBiome::Snow;
	}
	else if (BiomeName == BIOME_SubtropicalDesert)
	{
		return EWhittakerBiome::SubtropicalDesert;
	}
	else if (BiomeName == BIOME_Taiga)
	{
		return EWhittakerBiome::Taiga;
	}
	else if (BiomeName == BIOME_TemperateDeciduousForest)
	{
		return EWhittakerBiome::TemperateDeciduousForest;
	}
	else if (BiomeName == BIOME_TemperateDesert)
	{
		return EWhittakerBiome::TemperateDesert;
	}
	else if (BiomeName == BIOME_TemperateRainForest)
	{
		return EWhittakerBiome::TemperateRainForest;
	}
	else if (BiomeName == BIOME_TropicalRainForest)
	{
		return EWhittakerBiome::TropicalRainForest;
	}
	else if (BiomeName == BIOME_TropicalSeasonalForest)
	{
		return EWhittakerBiome::TropicalSeasonalForest;
	}
	else if (BiomeName == BIOME_Tundra)
	{
		return EWhittakerBiome::Tundra;
	}
	else
	{
		return EWhittakerBiome::Ocean;
	}
}