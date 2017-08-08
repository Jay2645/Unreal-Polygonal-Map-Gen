// Original Work Copyright (c) 2010 Amit J Patel, Modified Work Copyright (c) 2016 Jay M Stevens

#pragma once

#include "Object.h"
#include "PolygonMap.h"
#include "PolygonalMapHeightmap.generated.h"

class UMoistureDistributor;

/**
 * A class which creates a "heightmap" of FMapData objects from a UPolygonMap object.
 * This heightmap can be iterated over and its values used to create a voxel-based 3D representation in
 * the game world, as a 2D grayscale image, or whatever else you can think of.
 * `CreateHeightmap()` should be called after the UPolygonMap is finished being modified, as it does not 
 * update "live" with the UPolygonMap.
 * Once the heightmap has been created, you can access its data safely using `GetMapPoint()`, or you can
 * access the raw FMapData array with `GetMapData()`.
 */
UCLASS(BlueprintType)
class POLYGONALMAPGENERATOR_API UPolygonalMapHeightmap : public UObject
{
	GENERATED_BODY()

private:
	// The raw array of FMapData structs which represent each "pixel" in the heightmap.
	// Once populated, this is a 2D array of `HeightmapSize` by `HeightmapSize`, represented in a 1D array.
	UPROPERTY()
	TArray<FMapData> HeightmapData;
	UPROPERTY()
	UMoistureDistributor* MoistureDistributor;
	UPROPERTY()
	UBiomeManager* BiomeManager;
	UPROPERTY()
	FHeightmapCreationData HeightmapProperties;

	UPROPERTY()
	float CreateHeightmapTimer;

	FIslandGeneratorDelegate OnGenerationComplete;

	UFUNCTION()
	void CheckMapPointsDone();
	UFUNCTION()
	void DoHeightmapPostProcess();
public:
	// The number of "pixels" in the heightmap.
	// Larger values create a higher-resolution heightmap, but also mean increased processing time.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Heightmap")
	int32 HeightmapSize;
	// How many of the nearest points are used when creating a heightmap from a UPolygonMap.
	// As each pixel is iterated, this sets how many of the nearest UPolygonMap points we should interpolate between.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Heightmap")
	int32 NumberOfPointsToAverage = 4;

	// Initializes the UPolyognalMapHeightmap object and populates the HeightmapData array.
	// This should be called first, before attempting to load any data from the heightmap.
	// This will create an array of FMapData objects with dimensions specified by the Size passed to this function.
	// A larger value for Size will create a higher-resolution map, but will also mean increased processing time.
	// Each FMapData object will be created by being interpolated from the nearest points in the UPolygonMap supplied to this function.
	// The number of nearest points to consider is governed by the value of the NumberOfPointsToAverage in the UPolygonalMapHeightmap class.
	// Each "pixel" in the heightmap will be supplied with its own biome, which is determined by the UBiomeManager passed to this function.
	UFUNCTION(BlueprintCallable, Category = "World Generation|Island Generation|Heightmap")
	void CreateHeightmap(UPolygonMap* PolygonMap, UBiomeManager* BiomeMngr, UMoistureDistributor* MoistureDist, const FHeightmapCreationData HeightmapCreationOptions, const FIslandGeneratorDelegate onComplete);

	// How much of the heightmap we have complete so far.
	// This is most useful if you are calculating the heightmap in the background, so it can be used as a loading bar or such.
	UFUNCTION(BlueprintPure, Category = "World Generation|Island Generation|Heightmap")
	float GetCompletionPercent() const;

	// Returns a COPY of this object's raw heightmap.
	// This can be iterated over easily, but keep in mind that any changes you make won't be made to the actual heightmap object (i.e., this).
	// Also keep in mind that the array is actually a 1D representation of a 2D array of HeightmapSize x HeightmapSize.
	UFUNCTION(BlueprintPure, Category = "World Generation|Island Generation|Heightmap")
	TArray<FMapData> GetMapData();
	// Returns the FMapData that corresponds with the given (X, Y) position on the heightmap.
	// If the position is out of bounds of the heightmap or the heightmap is not initialized, it will return a blank FMapData object.
	UFUNCTION(BlueprintPure, Category = "World Generation|Island Generation|Heightmap")
	FMapData GetMapPoint(int32 x, int32 y);

	// Sets the map point at the given (X, Y) position on the heightmap.
	// If the point is out of bounds, a warning is generated and nothing happens.
	UFUNCTION(BlueprintCallable, Category = "World Generation|Island Generation|Heightmap")
	void SetMapPoint(int32 X, int32 Y, FMapData MapData);
};