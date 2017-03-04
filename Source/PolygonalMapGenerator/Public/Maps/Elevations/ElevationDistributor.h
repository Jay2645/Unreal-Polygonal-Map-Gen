// Original Work Copyright (c) 2010 Amit J Patel, Modified Work Copyright (c) 2016 Jay M Stevens

#pragma once

#include "Components/ActorComponent.h"
#include "PolygonMap.h"
#include "ElevationDistributor.generated.h"

/**
* The ElevationDistributor class is a class which takes a collection
* of graph nodes on the XY plane and gives the nodes Z values.
*/
UCLASS(Blueprintable)
class POLYGONALMAPGENERATOR_API UElevationDistributor : public UActorComponent
{
	GENERATED_BODY()

public:
	// Change the polygon graph for this ElevationDistributor.
	UFUNCTION(BlueprintCallable, Category = "Island Generation|Graph")
	void SetGraph(UPolygonMap* graph);

	// Determine the elevations and water at Voronoi corners.
	UFUNCTION(BlueprintCallable, Category = "Island Generation|Map")
	virtual void AssignCornerElevations(UIslandShape* islandShape, bool bneedsMoreRandomness, FRandomStream& randomGenerator);

	// Calculate downslope pointers.  At every point, we point to the
	// point downstream from it, or to itself.  This is used for
	// generating rivers and watersheds.
	UFUNCTION(BlueprintCallable, Category = "Island Generation|Map")
	virtual void CalculateDownslopes();

	// Rescale elevations so that the highest is 1.0, and they're
	// distributed well. We want lower elevations to be more common
	// than higher elevations, in proportions approximately matching
	// concentric rings. That is, the lowest elevation is the
	// largest ring around the island, and therefore should more
	// land area than the highest elevation, which is the very
	// center of a perfectly circular island.
	UFUNCTION(BlueprintCallable, Category = "Island Generation|Map")
	virtual void RedistributeElevations(TArray<int32> landCorners);

	// Flatten areas which are considered ocean corners
	UFUNCTION(BlueprintCallable, Category = "Island Generation|Map")
	virtual void FlattenWaterElevations();

	// Assign polygon elevations as the average of their corners
	UFUNCTION(BlueprintCallable, Category = "Island Generation|Map")
	virtual void AssignPolygonElevations();

protected:
	UPolygonMap* MapGraph;
};