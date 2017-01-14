// Original Work Copyright (c) 2010 Amit J Patel, Modified Work Copyright (c) 2016 Jay M Stevens

#pragma once

#include "Object.h"
#include "PolygonMap.h"
#include "PolygonalMapHeightmap.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class POLYGONALMAPGENERATOR_API UPolygonalMapHeightmap : public UObject
{
	GENERATED_BODY()

private:
	UPROPERTY()
	TArray<FMapData> HeightmapData;

	FMapData MakeMapPoint(FVector2D PixelPosition, TArray<FMapData> MapData, UBiomeManager* BiomeManager);

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Heightmap")
		int32 HeightmapSize;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Heightmap")
	int32 NumberOfPointsToAverage = 4;

	UFUNCTION(BlueprintCallable, Category = "Map Generation|Heightmap")
		void CreateHeightmap(UPolygonMap* PolygonMap, UBiomeManager* BiomeManager, int32 Size);

	UFUNCTION(BlueprintPure, Category = "Map Generation|Heightmap")
		FMapData GetMapPoint(int32 x, int32 y);

	UFUNCTION(BlueprintCallable, Category = "Map Generation|Heightmap|Debug")
	void DrawDebugPixelGrid(UWorld* world, float PixelSize);
};