// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Maps/Elevations/ElevationDistributor.h"
#include "Noise/PolygonalMapGenNoise.h"
#include "SimplexNoiseElevationDistributor.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class POLYGONALMAPGENERATOR_API USimplexNoiseElevationDistributor : public UElevationDistributor
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
	FNoiseSettings NoiseSettings;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
	float NoiseScale = 0.0033f;

	// The elevation below which a point is considered to be water.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
	float OceanThreshold = 0.05f;

	// How many times the gradient is applied.
	// More iterations smooths out the edges of the map and accentuates the center.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
	int32 GradientIterations = 1;

	USimplexNoiseElevationDistributor();

	// Determine the elevations and water at Voronoi corners.
	UFUNCTION(BlueprintCallable, Category = "Island Generation|Map")
	virtual void AssignCornerElevations(UIslandShape* islandShape, bool bneedsMoreRandomness, FRandomStream& randomGenerator) override;
	
	//UFUNCTION(BlueprintCallable, Category = "Island Generation|Map")
		//virtual void AssignOceanCoastAndLand(float lakeThreshold) override;

	UFUNCTION(BlueprintCallable, Category = "Island Generation|Map")
	virtual void RedistributeElevations(TArray<int32> landCorners) override;

	UFUNCTION(BlueprintPure, Category = "Island Generation|Map")
	virtual float GetGradient(float XCoord, float YCoord, int32 IslandSize) const;

	// Flatten areas which are considered ocean corners
	//UFUNCTION(BlueprintCallable, Category = "Island Generation|Map")
		//virtual void FlattenWaterElevations() override;
};
