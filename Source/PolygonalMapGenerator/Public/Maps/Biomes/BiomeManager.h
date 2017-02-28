// Original Work Copyright (c) 2010 Amit J Patel, Modified Work Copyright (c) 2016 Jay M Stevens

#pragma once

#include "Components/ActorComponent.h"
#include "PolygonMap.h"
#include "GameplayTagContainer.h"
#include "BiomeManager.generated.h"

USTRUCT(BlueprintType)
struct FBiome
{
	GENERATED_BODY()
public:
	// The GameplayTag associated with this biome.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Map Biome", meta = (Categories = "MapData.Biome"))
	FGameplayTag BiomeTag;

	// The minimum elevation for this biome
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Elevation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinElevation;
	// The maximum elevation for this biome
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Elevation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MaxElevation;
	// The minimum amount of moisture for this biome
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Moisture", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinMoisture;
	// The maxmimum amount of moisture for this biome
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Moisture", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MaxMoisture;

	// Any required tags that need to be on the FMapData struct for this biome to be selected
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Map Biome", meta = (Categories = "MapData.MetaData"))
	FGameplayTagContainer RequiredTags;

	FBiome()
	{
		MinElevation = 0.0f;
		MaxElevation = 1.0f;

		MinMoisture = 0.0f;
		MaxMoisture = 1.0f;
	}
};

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class POLYGONALMAPGENERATOR_API UBiomeManager : public UActorComponent
{
	GENERATED_BODY()
protected:
	// Our implementation for determining the biome of a point
	virtual FGameplayTag DetermineBiome_Implementation(const FMapData& MapData);

public:	
	// Sets default values for this component's properties
	UBiomeManager();

	static const FName BIOME_OceanTag;
	static const FName BIOME_CoastTag;

	// A list of all valid biomes in this BiomeManager.
	// Users should add any biomes they'd like to iterate over and choose from to this list.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
	TArray<FBiome> Biomes;

	// Whether this biome manager should allow biomes with snow.
	// Games taking place in tropical climates may not need to have snow.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
	bool bAllowSnow = true;
	
	// This determines the biome that the MapData should be in.
	// It can be overridden by the user if they would like to make their own implementation.
	// By default, it will iterate over the Biomes array and return the first biome that fulfills all requirements.
	// If none can be found, a warning is printed to the log and an empty tag is returned.
	UFUNCTION(BlueprintNativeEvent, Category="Biome")
	FGameplayTag DetermineBiome(const FMapData& MapData);
};
