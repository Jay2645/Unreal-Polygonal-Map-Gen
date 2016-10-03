// Original Work Copyright (c) 2010 Amit J Patel, Modified Work Copyright (c) 2016 Jay M Stevens

#pragma once

#include "Object.h"
#include "PolygonMap.h"
#include "PolygonalMapGen/Private/Map/PointGenerator/PointGenerator.h"
#include "PolygonalMapGen/Private/Map/IslandShapes/IslandShape.h"
#include "IslandMapGenerator.generated.h"

USTRUCT(BlueprintType)
struct FIslandData
{
	GENERATED_BODY()

	// The type of island to make. Different generators will make differently-shaped islands.
	UPROPERTY(Category = "Island", BlueprintReadWrite, EditAnywhere)
	TSubclassOf<UIslandShape> IslandType;

	// The random seed to use for the island
	UPROPERTY(Category = "Island", BlueprintReadWrite, EditAnywhere)
	int32 Seed;

	// Size of the actual island
	UPROPERTY(Category = "Island", BlueprintReadWrite, EditAnywhere)
	int32 Size;

	// Increases the mountain area
	UPROPERTY(Category = "Island", BlueprintReadWrite, EditAnywhere)
	float ScaleFactor;

	UPROPERTY(Category = "Points", BlueprintReadWrite, EditAnywhere)
	TSubclassOf<UPointGenerator> IslandPointSelector;

	// The number of points to generate
	UPROPERTY(Category = "Points", BlueprintReadWrite, EditAnywhere)
	int32 NumberOfPoints;

	// 0 to 1, fraction of water corners for water polygon
	UPROPERTY(Category = "Water", BlueprintReadWrite, EditAnywhere)
	float LakeThreshold;

	UWorld* GameWorld;

	//Constructor
	FIslandData()
	{
		LakeThreshold = 0.3;
		Size = 600;
		NumberOfPoints = 150;
		ScaleFactor = 1.1f;
	}
};

/**
 * The IslandMapGenerator creates a graph and then assigns
 * elevation and moisture based on various rules set in
 * IslandData.
 */
UCLASS(Blueprintable)
class AIslandMapGenerator : public AActor
{
	GENERATED_BODY()
public:
	AIslandMapGenerator() {};
	~AIslandMapGenerator() {};

	// Sets the IslandData that will be used for Island Generation.
	UFUNCTION(BlueprintCallable, Category="Island Generation")
	void SetData(FIslandData islandData);

	// Resets the map back to its default state.
	UFUNCTION(BlueprintCallable, Category = "Island Generation")
	void ResetMap();

	// Generates a new map from our IslandData.
	UFUNCTION(BlueprintNativeEvent, Category = "Island Generation")
	void GenerateMap();

	void GenerateMap_Implementation();

	// Returns a MapCenter at the given index.
	// If the index is invalid, an empty MapCenter will be returned.
	// This empty MapCenter will have an index of -1.
	// Make sure to use the function UpdateCenter() to update the graph
	// after the MapCenter has been modified. 
	UFUNCTION(BlueprintPure, Category = "Island Generation|Graph")
	FMapCenter& GetCenter(const int32 index);
	// Returns a MapCorner at the given index.
	// If the index is invalid, an empty MapCorner will be returned.
	// This empty MapCorner will have an index of -1.
	// Make sure to use the function UpdateCorner() to update the graph
	// after the MapCorner has been modified. 
	UFUNCTION(BlueprintPure, Category = "Island Generation|Graph")
	FMapCorner& GetCorner(const int32 index);
	// Returns a MapEdge at the given index.
	// If the index is invalid, an empty MapEdge will be returned.
	// This empty MapEdge will have an index of -1.
	// Make sure to use the function UpdateEdge() to update the graph
	// after the MapEdge has been modified. 
	UFUNCTION(BlueprintPure, Category = "Island Generation|Graph")
	FMapEdge& GetEdge(const int32 index);
	
	// Returns the MapEdge defined by two adjacent MapCenters.
	// If the edge doesn't exist, an empty MapEdge will be returned.
	// This empty MapEdge will have an index of -1.
	// Make sure to use the function UpdateEdge() to update the graph
	// after the MapEdge has been modified.
	UFUNCTION(BlueprintPure, Category = "Island Generation|Graph")
	FMapEdge& FindEdgeFromCenters(const FMapCenter& v0, const FMapCenter& v1);
	// Returns the MapEdge defined by two adjacent MapCorners.
	// If the edge doesn't exist, an empty MapEdge will be returned.
	// This empty MapEdge will have an index of -1.
	// Make sure to use the function UpdateEdge() to update the graph
	// after the MapEdge has been modified.
	UFUNCTION(BlueprintPure, Category = "Island Generation|Graph")
	FMapEdge& FindEdgeFromCorners(const FMapCorner& v0, const FMapCorner& v1);

	// Returns the number of MapCenters in our graph.
	UFUNCTION(BlueprintPure, Category = "Island Generation|Graph")
	int32 GetCenterNum();
	// Returns the number of MapCorners in our graph.
	UFUNCTION(BlueprintPure, Category = "Island Generation|Graph")
	int32 GetCornerNum();
	// Returns the number of MapEdges in our graph.
	UFUNCTION(BlueprintPure, Category = "Island Generation|Graph")
	int32 GetEdgeNum();

	// Returns our Graph.
	UFUNCTION(BlueprintPure, Category = "Island Generation|Graph")
	UPolygonMap* GetGraph();

	// Update a MapCenter in the graph after it has been modified.
	UFUNCTION(BlueprintCallable, Category = "Island Generation|Graph")
	void UpdateCenter(const FMapCenter& center);
	// Update a MapCorner in the graph after it has been modified.
	UFUNCTION(BlueprintCallable, Category = "Island Generation|Graph")
	void UpdateCorner(const FMapCorner& corner);
	// Update a MapEdge in the graph after it has been modified.
	UFUNCTION(BlueprintCallable, Category = "Island Generation|Graph")
	void UpdateEdge(const FMapEdge& edge);

	UPROPERTY(Category = "Island", BlueprintReadWrite, EditAnywhere)
	FIslandData IslandData;
private:
	// Initializes the IslandShape and PointSelector classes
	void InitializeMap();

	void BuildGraph();

	// Determines graph elevation
	void AssignElevation();

	// Determines downslopes and moisture
	void AssignMoisture();

	// Determine the elevations and water at Voronoi corners.
	void AssignCornerElevations();

	// Determine polygon and corner type: ocean, coast, land.
	void AssignOceanCoastAndLand();

	// Rescale elevations so that the highest is 1.0, and they're
	// distributed well. We want lower elevations to be more common
	// than higher elevations, in proportions approximately matching
	// concentric rings. That is, the lowest elevation is the
	// largest ring around the island, and therefore should more
	// land area than the highest elevation, which is the very
	// center of a perfectly circular island.
	void RedistributeElevations(TArray<int32> landCorners);

	// Flatten areas which are considered ocean corners
	void FlattenWaterElevations();

	// Assign polygon elevations as the average of their corners
	void AssignPolygonElevations();

	// Calculate downslope pointers.  At every point, we point to the
	// point downstream from it, or to itself.  This is used for
	// generating rivers and watersheds.
	void CalculateDownslopes();

	// Calculate the watershed of every land point. The watershed is
	// the last downstream land point in the downslope graph. TODO:
	// watersheds are currently calculated on corners, but it'd be
	// more useful to compute them on polygon centers so that every
	// polygon can be marked as being in one watershed.
	void CalculateWatersheds();

	// Create rivers along edges. Pick a random corner point, then
	// move downslope. Mark the edges and corners as rivers.
	void CreateRivers();

	// Calculate moisture. Freshwater sources spread moisture: rivers
	// and lakes (not oceans). Saltwater sources have moisture but do
	// not spread it (we set it at the end, after propagation).
	void AssignCornerMoisture();

	// Redistribute moisture to cover the entire range evenly 
	// from 0.0 to 1.0.
	void RedistributeMoisture(TArray<int32> landCorners);

	// Assign polygon moisture as the average of the corner moisture.
	void AssignPolygonMoisture();

	void FinalizeAllPoints();

	// Create an array of corners that are on land only, for use by
	// algorithms that work only on land.

	//void DrawDebugPixelGrid();

	UPROPERTY()
		UIslandShape* IslandShape;
	UPROPERTY()
		UPointGenerator* PointSelector;
	UPROPERTY()
		UPolygonMap* MapGraph;

	UPROPERTY()
		FRandomStream RandomGenerator;
};