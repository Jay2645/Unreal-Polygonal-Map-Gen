// Original Work Copyright (c) 2010 Amit J Patel, Modified Work Copyright (c) 2016 Jay M Stevens

#pragma once

#include "GameFramework/Actor.h"
#include "PolygonMap.h"
#include "Maps/Elevations/ElevationDistributor.h"
#include "Maps/PointGenerators/PointGenerator.h"
#include "Maps/Biomes/BiomeManager.h"
#include "Maps/Moisture/MoistureDistributor.h"
#include "PolygonalMapHeightmap.h"
#include "Maps/IslandShapes/IslandShape.h"
#include "IslandMapGenerator.generated.h"

DECLARE_DYNAMIC_DELEGATE(FIslandGeneratorDelegate);

/*
* The IslandData struct manages all the attributes used to generate an island.
*/
USTRUCT(BlueprintType)
struct POLYGONALMAPGENERATOR_API FIslandData
{
	GENERATED_BODY()

	// The type of island to make.
	// Different generators will make differently-shaped islands.
	// By default, all points will be considered water.
	UPROPERTY(Category = "Island", BlueprintReadWrite, EditAnywhere)
	TSubclassOf<UIslandShape> IslandType;

	// What IslandPointGenerator to use.
	// By default, all points will be at 0,0.
	// Generators can do things like make random points, for a "natural"-looking map, make square points for a grid-shaped map, etc.
	UPROPERTY(Category = "Points", BlueprintReadWrite, EditAnywhere)
	TSubclassOf<UPointGenerator> IslandPointSelector;

	// What BiomeManager to use.
	// By default, only MapData objects with the ocean or coast properties will get biomes.
	// Users should create their own class to determine which biomes they want in their game.
	UPROPERTY(Category = "Biomes", BlueprintReadWrite, EditAnywhere)
	TSubclassOf<UBiomeManager> BiomeManager;

	// What ElevationDistributor to use.
	// By default, this will create an island with the elevation mostly focused in the middle.
	// This will create landmasses with a giant mountain in the center.
	UPROPERTY(Category = "Biomes", BlueprintReadWrite, EditAnywhere)
	TSubclassOf<UElevationDistributor> ElevationDistributor;


	// What ElevationDistributor to use.
	UPROPERTY(Category = "Biomes", BlueprintReadWrite, EditAnywhere)
	TSubclassOf<UMoistureDistributor> MoistureDistributor;

	// The random seed to use for the island.
	UPROPERTY(Category = "Island", BlueprintReadWrite, EditAnywhere)
	int32 Seed;

	// The size of the actual island, in meters.
	UPROPERTY(Category = "Island", BlueprintReadWrite, EditAnywhere)
	int32 Size;

	UPROPERTY(Category = "Island", BlueprintReadWrite, EditAnywhere)
	float ScaleFactor;

	// The number of points to generate
	UPROPERTY(Category = "Points", BlueprintReadWrite, EditAnywhere)
	int32 NumberOfPoints;

	// 0 to 1, fraction of water corners for water polygon
	UPROPERTY(Category = "Water", BlueprintReadWrite, EditAnywhere)
	float LakeThreshold;

	// This is the settings for converting the points we generate into Unreal world space.
	UPROPERTY(Category = "Map", BlueprintReadWrite, EditAnywhere)
	FWorldSpaceMapData PolygonMapSettings;

	//Constructor
	FIslandData()
	{
		LakeThreshold = 0.3;
		Size = 1024;
		NumberOfPoints = 1500;
		ScaleFactor = 1.1f;

		IslandType = UIslandShape::StaticClass();
		IslandPointSelector = UPointGenerator::StaticClass();
		BiomeManager = UBiomeManager::StaticClass();
		ElevationDistributor = UElevationDistributor::StaticClass();
	}
};

/**
* The IslandMapGenerator creates a graph and then assigns
* elevation and moisture based on various rules set in
* IslandData, using subclasses.
*/
UCLASS(Blueprintable)
class POLYGONALMAPGENERATOR_API AIslandMapGenerator : public AActor
{
	GENERATED_BODY()
public:

	AIslandMapGenerator();
	~AIslandMapGenerator() {};

	virtual void Tick(float deltaSeconds) override;
	// Sets the IslandData that will be used for Island Generation.
	UFUNCTION(BlueprintCallable, Category = "Island Generation")
		void SetData(FIslandData islandData);

	// Resets the map back to its default state.
	UFUNCTION(BlueprintCallable, Category = "Island Generation")
		void ResetMap();

	// Adds all the necessary steps for Island Generation, then
	// generates an island.
	// It will call the passed delegate when the generation is done.
	UFUNCTION(BlueprintCallable, Category = "Island Generation")
	void CreateMap(const FIslandGeneratorDelegate onComplete);

	// Returns a MapCenter at the given index.
	// If the index is invalid, an empty MapCenter will be returned.
	// This empty MapCenter will have an index of -1.
	// Make sure to use the function UpdateCenter() to update the graph
	// after the MapCenter has been modified. 
	UFUNCTION(BlueprintPure, Category = "Island Generation|Graph")
	FMapCenter GetCenter(const int32 index) const;
	// Returns a MapCorner at the given index.
	// If the index is invalid, an empty MapCorner will be returned.
	// This empty MapCorner will have an index of -1.
	// Make sure to use the function UpdateCorner() to update the graph
	// after the MapCorner has been modified. 
	UFUNCTION(BlueprintPure, Category = "Island Generation|Graph")
	FMapCorner GetCorner(const int32 index) const;
	// Returns a MapEdge at the given index.
	// If the index is invalid, an empty MapEdge will be returned.
	// This empty MapEdge will have an index of -1.
	// Make sure to use the function UpdateEdge() to update the graph
	// after the MapEdge has been modified. 
	UFUNCTION(BlueprintPure, Category = "Island Generation|Graph")
	FMapEdge GetEdge(const int32 index) const;

	// Returns the MapEdge defined by two adjacent MapCenters.
	// If the edge doesn't exist, an empty MapEdge will be returned.
	// This empty MapEdge will have an index of -1.
	// Make sure to use the function UpdateEdge() to update the graph
	// after the MapEdge has been modified.
	UFUNCTION(BlueprintPure, Category = "Island Generation|Graph")
	FMapEdge FindEdgeFromCenters(const FMapCenter& v0, const FMapCenter& v1) const;
	// Returns the MapEdge defined by two adjacent MapCorners.
	// If the edge doesn't exist, an empty MapEdge will be returned.
	// This empty MapEdge will have an index of -1.
	// Make sure to use the function UpdateEdge() to update the graph
	// after the MapEdge has been modified.
	UFUNCTION(BlueprintPure, Category = "Island Generation|Graph")
	FMapEdge FindEdgeFromCorners(const FMapCorner& v0, const FMapCorner& v1) const;

	// Returns the number of MapCenters in our graph.
	UFUNCTION(BlueprintPure, Category = "Island Generation|Graph")
	int32 GetCenterNum() const;
	// Returns the number of MapCorners in our graph.
	UFUNCTION(BlueprintPure, Category = "Island Generation|Graph")
	int32 GetCornerNum() const;
	// Returns the number of MapEdges in our graph.
	UFUNCTION(BlueprintPure, Category = "Island Generation|Graph")
	int32 GetEdgeNum() const;

	// Returns our Graph.
	UFUNCTION(BlueprintPure, Category = "Island Generation|Graph")
	UPolygonMap* GetGraph() const;
	UFUNCTION(BlueprintPure, Category = "Island Generation|Graph")
	UPolygonalMapHeightmap* GetHeightmap() const;

	// Update a MapCenter in the graph after it has been modified.
	UFUNCTION(BlueprintCallable, Category = "Island Generation|Graph")
	void UpdateCenter(const FMapCenter& center);
	// Update a MapCorner in the graph after it has been modified.
	UFUNCTION(BlueprintCallable, Category = "Island Generation|Graph")
	void UpdateCorner(const FMapCorner& corner);
	// Update a MapEdge in the graph after it has been modified.
	UFUNCTION(BlueprintCallable, Category = "Island Generation|Graph")
	void UpdateEdge(const FMapEdge& edge);

	UFUNCTION(BlueprintCallable, Category = "Island Generation")
	void AddGenerationStep(const FIslandGeneratorDelegate step);

	UFUNCTION(BlueprintCallable, Category = "Island Generation")
	void ClearAllGenerationSteps();

	UFUNCTION(BlueprintCallable, Category = "Island Generation|Debug")
	void DrawVoronoiGraph();

	UFUNCTION(BlueprintCallable, Category = "Island Generation|Debug")
	void DrawDelaunayGraph();

	UFUNCTION(BlueprintCallable, Category = "Island Generation|Debug")
	void DrawHeightmap(float PixelSize = 100.0f);

	UPROPERTY(Category = "Island", BlueprintReadWrite, EditAnywhere)
	FIslandData IslandData;

protected:
	// Adds a list of steps that we need to generate our island.
	// Users using Blueprint should override this and NOT call the
	// parent function. They have to add steps in the Event Graph
	// due to the way Blueprint works with delegates ("Events").
	UFUNCTION(BlueprintNativeEvent, Category = "Island Generation")
	void AddMapSteps();
	virtual void AddMapSteps_Implementation();

	// Generates a new map from our IslandData.
	// This iterates over each step in the IslandGeneratorSteps queue
	// and executes the event. Players using Blueprint can add their
	// own steps to the queue or create a new queue altogether in the
	// Blueprint Event Graph.
	UFUNCTION(BlueprintCallable, Category = "Island Generation")
	void GenerateMap();

	UFUNCTION(BlueprintImplementableEvent, Category = "Island Generation|Map")
	void ResetMapEvent();

	UFUNCTION(BlueprintImplementableEvent, Category = "Island Generation|Map")
	void InitializeMapEvent();
	// Initializes the IslandShape and PointSelector classes
	UFUNCTION(BlueprintCallable, Category = "Island Generation|Map")
	void InitializeMap();

	UFUNCTION(BlueprintImplementableEvent, Category = "Island Generation|Map")
	void BuildGraphEvent();
	// Creates the initial points that our map will work on
	UFUNCTION(BlueprintCallable, Category = "Island Generation|Map")
	void BuildGraph();

	UFUNCTION(BlueprintImplementableEvent, Category = "Island Generation|Map")
	void AssignElevationEvent();
	// Determines graph elevation
	UFUNCTION(BlueprintCallable, Category = "Island Generation|Map")
	void AssignElevation();

	UFUNCTION(BlueprintImplementableEvent, Category = "Island Generation|Map")
	void AssignMoistureEvent();
	// Determines downslopes and moisture
	UFUNCTION(BlueprintCallable, Category = "Island Generation|Map")
	void AssignMoisture();

	UFUNCTION(BlueprintCallable, Category = "Island Generation|Map")
	virtual void DoPointPostProcess();
	UFUNCTION(BlueprintImplementableEvent, Category = "Island Generation|Map")
	void FinalizeAllPointsEvent();
	// Does final processing on the graph
	UFUNCTION(BlueprintCallable, Category = "Island Generation|Map")
	void FinalizeAllPoints();

private:
	UPROPERTY()
	UPolygonMap* MapGraph;
	UPROPERTY()
	UPolygonalMapHeightmap* MapHeightmap;

	UPROPERTY()
	FRandomStream RandomGenerator;

	TQueue<FIslandGeneratorDelegate> IslandGeneratorSteps;

	FIslandGeneratorDelegate OnGenerationComplete;
public:
	// The instance of our IslandShape object.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Island Generation")
	UIslandShape* IslandShape;
	// The instance of our PointGenerator object.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Island Generation")
	UPointGenerator* PointSelector;
	// The instance of our ElevationDistributor object.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Island Generation")
	UElevationDistributor* ElevationDistributor;
	// The instance of our MoistureDistributor object.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Island Generation")
	UMoistureDistributor* MoistureDistributor;
	// The instance of our BiomeManager object.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Island Generation")
	UBiomeManager* BiomeManager;
};