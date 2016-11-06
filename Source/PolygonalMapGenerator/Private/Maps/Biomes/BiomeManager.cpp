// Original Work Copyright (c) 2010 Amit J Patel, Modified Work Copyright (c) 2016 Jay M Stevens

#include "PolygonalMapGeneratorPrivatePCH.h"
#include "BiomeManager.h"

const FName UBiomeManager::BIOME_Ocean = TEXT("Ocean");
const FName UBiomeManager::BIOME_Coast = TEXT("Coast");

// Sets default values for this component's properties
UBiomeManager::UBiomeManager()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	bWantsBeginPlay = true;
	PrimaryComponentTick.bCanEverTick = false;
}

FName UBiomeManager::DetermineBiome_Implementation(const FMapData& MapData)
{
	if (MapData.bIsOcean)
	{
		return BIOME_Ocean;
	}
	if (MapData.bIsCoast)
	{
		return BIOME_Coast;
	}
	return NAME_None;
}