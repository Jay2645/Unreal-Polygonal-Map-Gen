// Original Work Copyright (c) 2010 Amit J Patel, Modified Work Copyright (c) 2016 Jay M Stevens

#pragma once

#include "Components/ActorComponent.h"
#include "PolygonMap.h"
#include "BiomeManager.generated.h"


UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class POLYGONALMAPGENERATOR_API UBiomeManager : public UActorComponent
{
	GENERATED_BODY()
protected:
	virtual FName DetermineBiome_Implementation(const FMapData& MapData);

public:	
	// Sets default values for this component's properties
	UBiomeManager();

	static const FName BIOME_Ocean;
	static const FName BIOME_Coast;
	
	UFUNCTION(BlueprintNativeEvent, Category="Biome")
	FName DetermineBiome(const FMapData& MapData);
};
