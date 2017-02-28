// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "PolygonMap.h"
#include "River.h"
#include "MoistureDistributor.generated.h"

/**
 * The MoistureDistributor class distributes moisture across a map.
 * It is also in charge of calculating stream flow direction, rivers, and watersheds.
 */
UCLASS(Blueprintable)
class POLYGONALMAPGENERATOR_API UMoistureDistributor : public UActorComponent
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lakes", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float LakeThreshold = 0.9f;
	// The maximum number of rivers in this map.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rivers")
	int32 RiverCount = 50;
	// The minimum height a river will begin at.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rivers", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinRiverStartElevation = 0.1f;
	// The maximum height a river will begin at.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rivers", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MaxRiverStartElevation = 0.9f;
	// The bias a river has towards running downstream.
	// The higher the number, the more likely the river will go downstream.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rivers", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float RiverDownstreamBias = 0.5f;
	// The bias a river has for wandering into somewhere with standing water (like a lake).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rivers", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float StandingWaterBias = 0.75f;
	// How much more likely a river is to join another river if there is one.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rivers", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float RiverJoinBias = 0.825f;
	// The chance that rivers bordering each other will combine to create a lake.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rivers", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float RiverLakeConversionFactor = 0.85f;
	// The minimum number of points needed for a river to be valid
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rivers")
	uint8 MinRiverSectionCount = 5;
	// How wide a river is by default.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rivers")
	uint8 StartingRiverSize = 1;
	// The maximum width a river can grow to.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rivers")
	uint8 MaxRiverSize = 4;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Rivers")
	TArray<URiver*> Rivers;

	UFUNCTION(BlueprintCallable, Category = "Island Generation|Map")
	void SetGraph(UPolygonMap* Graph, int32 Size);


	// Determine polygon and corner type: ocean, coast, land.
	UFUNCTION(BlueprintCallable, Category = "Island Generation|Map")
	virtual void AssignOceanCoastAndLand();

	// Calculate the watershed of every land point. The watershed is
	// the last downstream land point in the downslope graph. TODO:
	// watersheds are currently calculated on corners, but it'd be
	// more useful to compute them on polygon centers so that every
	// polygon can be marked as being in one watershed.
	UFUNCTION(BlueprintCallable, Category = "Island Generation|Map")
	void CalculateWatersheds();
	// Create rivers along edges. Pick a random corner point, then
	// move downslope. Mark the edges and corners as rivers.
	UFUNCTION(BlueprintCallable, Category = "Island Generation|Map")
	void CreateRivers(FRandomStream& RandomGenerator);
	// Calculate moisture. Freshwater sources spread moisture: rivers
	// and lakes (not oceans). Saltwater sources have moisture but do
	// not spread it (we set it at the end, after propagation).
	UFUNCTION(BlueprintCallable, Category = "Island Generation|Map")
	void AssignCornerMoisture();
	// Redistribute moisture to cover the entire range evenly 
	// from 0.0 to 1.0.
	UFUNCTION(BlueprintCallable, Category = "Island Generation|Map")
	void RedistributeMoisture(TArray<int32> LandCorners);
	// Assign polygon moisture as the average of the corner moisture.
	UFUNCTION(BlueprintCallable, Category = "Island Generation|Map")
	void AssignPolygonMoisture();

protected:
	UPolygonMap* MapGraph;
	int32 MapSize;
};
