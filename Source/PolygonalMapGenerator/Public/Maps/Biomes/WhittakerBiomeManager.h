// Original Work Copyright (c) 2010 Amit J Patel, Modified Work Copyright (c) 2017 Jay M Stevens
#pragma once

#include "Maps/Biomes/BiomeManager.h"
#include "WhittakerBiomeManager.generated.h"

/**
* An enum representing the different Whittaker biomes.
* There are 18 biomes total.
*/
UENUM(BlueprintType)
enum class EWhittakerBiome : uint8
{
	Ocean						UMETA(DisplayName = "Ocean"),
	Ice							UMETA(DisplayName = "Ice"),
	Marsh						UMETA(DisplayName = "Marsh"),
	Lake						UMETA(DisplayName = "Lake"),
	Coast						UMETA(DisplayName = "Coast"),
	Snow						UMETA(DisplayName = "Snow"),
	Tundra						UMETA(DisplayName = "Tundra"),
	Bare						UMETA(DisplayName = "Bare"),
	Scorched					UMETA(DisplayName = "Scorched"),
	Taiga						UMETA(DisplayName = "Taiga"),
	Shrubland					UMETA(DisplayName = "Shrubland"),
	TemperateDesert				UMETA(DisplayName = "Temperate Desert"),
	TemperateRainForest			UMETA(DisplayName = "Temperate Rain Forest"),
	TemperateDeciduousForest	UMETA(DisplayName = "Temperate Deciduous Forest"),
	Grassland					UMETA(DisplayName = "Grassland"),
	TropicalRainForest			UMETA(DisplayName = "Tropical Rain Forest"),
	TropicalSeasonalForest		UMETA(DisplayName = "Tropical Seasonal Forest"),
	SubtropicalDesert			UMETA(DisplayName = "Subtropical Desert")
};

/**
 * An implementation of `BiomeManager` which uses the Whittaker diagram for its biomes.
 * Total list of biomes:
 *		- OCEAN: Has tag "Ocean"
 *		- ICE: Has tag "Water", Elevation > 0.75, allow snow
 *		- MARSH: Has tag "Water", Elevation < 0.25
 *		- LAKE: Has tag "Water"; does NOT have tags "Ocean", "Ice", or "Marsh"
 *		- COAST: Has tag "Coast"
 *		- SNOW: Elevation > 0.75 and Moisture > 0.5, allow snow
 *		- TUNDRA: Elevation > 0.75 and 0.5 > Moisture > 0.33, allow snow
 *		- BARE: Elevation > 0.75 and 0.33 > Moisture > 0.16
 *		- SCORCHED: Elevation > 0.75 and Moisture < 0.16
 *		- TAIGA: 0.75 > Elevation > 0.5 and Moisture > 0.66, allow snow
 *		- SHRUBLAND: 0.75 > Elevation > 0.5 and 0.66 > Moisture > 0.33
 *		- TEMPERATE DESERT: 0.75 > Elevation > 0.5 and Moisture < 0.33 OR 0.5 > Elevation > 0.25 and Moisture < 0.16
 *		- TEMPERATE RAIN FOREST: 0.5 > Elevation > 0.25 and Moisture > 0.83
 *		- TEMPERATE DECIDUOUS FOREST: 0.5 > Elevation > 0.25 and 0.83 > Moisture > 0.5
 *		- GRASSLAND: 0.5 > Elevation > 0.25 and 0.5 > Moisture > 0.16 OR Elevation < 0.25 and 0.33 > Moisture > 0.16
 *		- TROPICAL RAIN FOREST: Elevation < 0.25 and Moisture > 0.66
 *		- TROPICAL SEASONAL FOREST: Elevation < 0.25 and 0.66 > Moisture > 0.33
 *		- SUBTROPICAL DESERT: Elevation < 0.25 and Moisture < 0.16
 */
UCLASS(Blueprintable)
class POLYGONALMAPGENERATOR_API UWhittakerBiomeManager : public UBiomeManager
{
	GENERATED_BODY()
protected:
	static const FName BIOME_IceTag;
	static const FName BIOME_MarshTag;
	static const FName BIOME_LakeTag;
	static const FName BIOME_SnowTag;
	static const FName BIOME_TundraTag;
	static const FName BIOME_BareTag;
	static const FName BIOME_ScorchedTag;
	static const FName BIOME_TaigaTag;
	static const FName BIOME_ShrublandTag;
	static const FName BIOME_TemperateDesertTag;
	static const FName BIOME_TemperateRainForestTag;
	static const FName BIOME_TemperateDeciduousForestTag;
	static const FName BIOME_GrasslandTag;
	static const FName BIOME_TropicalRainForestTag;
	static const FName BIOME_TropicalSeasonalForestTag;
	static const FName BIOME_SubtropicalDesertTag;

	//virtual FGameplayTag DetermineBiome_Implementation(const FMapData& MapData) override;
public:
	UWhittakerBiomeManager();

	// Converts a biome's FName into an enum representation.
	// Will return EWhittakerBiome::Ocean if there are no matches in this biome.
	// The enum returned can be used in a switch statement or converted into a uint8 (for example, for use as a material ID).
	UFUNCTION(BlueprintPure, Category = "Biome")
	EWhittakerBiome ConvertToWhittakerBiomeEnum(const FGameplayTag& BiomeName);

protected:
	TMap<FGameplayTag, EWhittakerBiome> BiomeEnumMap;
};
