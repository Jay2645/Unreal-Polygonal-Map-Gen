// Original Work Copyright (c) 2010 Amit J Patel, Modified Work Copyright (c) 2016 Jay M Stevens

#pragma once

#include "Components/ActorComponent.h"
#include "PolygonMap.h"
#include "GameplayTagContainer.h"
#include "BiomeManager.generated.h"

USTRUCT(BlueprintType)
struct FBiomeSettings
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Elevation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinElevation;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Elevation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MaxElevation;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Moisture", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinMoisture;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Moisture", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MaxMoisture;

	FBiomeSettings()
	{
		MinElevation = 0.0f;
		MaxElevation = 1.0f;

		MinMoisture = 0.0f;
		MaxMoisture = 1.0f;
	}
};

USTRUCT(BlueprintType)
struct FBiome
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Map Biome", meta = (Categories = "MapData.Biome"))
	FGameplayTag BiomeTag;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Map Biome")
	TArray<FBiomeSettings> BiomeSettings;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Map Biome", meta = (Categories = "MapData.MetaData"))
	FGameplayTagContainer RequiredTags;
};

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class POLYGONALMAPGENERATOR_API UBiomeManager : public UActorComponent
{
	GENERATED_BODY()
protected:
	virtual FGameplayTag DetermineBiome_Implementation(const FMapData& MapData);

public:	
	// Sets default values for this component's properties
	UBiomeManager();

	static const FName BIOME_OceanTag;
	static const FName BIOME_CoastTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
	TArray<FBiome> Biomes;

	// Whether this biome manager should allow biomes with snow.
	// Games taking place in tropical climates may not need to have snow.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
	bool bAllowSnow = true;
	
	UFUNCTION(BlueprintNativeEvent, Category="Biome")
	FGameplayTag DetermineBiome(const FMapData& MapData);
};
