// Original Work Copyright (c) 2010 Amit J Patel, Modified Work Copyright (c) 2016 Jay M Stevens

#pragma once

#include "GameFramework/Actor.h"
#include "PolygonMap.h"
#include "Maps/Elevations/ElevationDistributor.h"
#include "Maps/PointGenerators/PointGenerator.h"
#include "Maps/Biomes/BiomeManager.h"
#include "Maps/Moisture/MoistureDistributor.h"
#include "PolygonalMapHeightmap.h"
#include "Maps/IslandShapes/RadialIsland.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "IslandMapGenerator.generated.h"

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
	// This is the table of all possible river names.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
	UDataTable* RiverNameTable;

	// The random seed to use for the island.
	// The same seed will produce the same island.
	UPROPERTY(Category = "Island", BlueprintReadWrite, EditAnywhere)
	int32 Seed;

	// The size of the actual island, in meters.
	UPROPERTY(Category = "Island", BlueprintReadWrite, EditAnywhere)
	int32 Size;

	// The number of points to generate.
	// Higher values creates a higher-quality island, but can dramatically slow down generation time
	UPROPERTY(Category = "Points", BlueprintReadWrite, EditAnywhere)
	int32 NumberOfPoints;

	// These are the settings for converting the points we generate into Unreal world space.
	UPROPERTY(Category = "Map", BlueprintReadWrite, EditAnywhere)
	FWorldSpaceMapData PolygonMapSettings;

	//Constructor
	FIslandData()
	{
		Size = 1024;
		NumberOfPoints = 1500;

		IslandType = URadialIsland::StaticClass();
		IslandPointSelector = UPointGenerator::StaticClass();
		BiomeManager = UBiomeManager::StaticClass();
		ElevationDistributor = UElevationDistributor::StaticClass();
		MoistureDistributor = UMoistureDistributor::StaticClass();
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
	UFUNCTION(BlueprintCallable, Category = "World Generation|Island Generation")
	void SetData(FIslandData islandData);

	// Resets the map back to its default state.
	UFUNCTION(BlueprintCallable, Category = "World Generation|Island Generation")
	void ResetMap();

	// Adds all the necessary steps for Island Generation, then
	// generates an island.
	// It will call the passed delegate when the generation is done.
	UFUNCTION(BlueprintCallable, Category = "World Generation|Island Generation")
	void CreateMap(const FIslandGeneratorDelegate onComplete);

	// Returns a MapCenter at the given index.
	// If the index is invalid, an empty MapCenter will be returned.
	// This empty MapCenter will have an index of -1.
	// Make sure to use the function UpdateCenter() to update the graph
	// after the MapCenter has been modified. 
	UFUNCTION(BlueprintPure, Category = "World Generation|Island Generation|Graph")
	FMapCenter GetCenter(const int32 index) const;
	// Returns a MapCorner at the given index.
	// If the index is invalid, an empty MapCorner will be returned.
	// This empty MapCorner will have an index of -1.
	// Make sure to use the function UpdateCorner() to update the graph
	// after the MapCorner has been modified. 
	UFUNCTION(BlueprintPure, Category = "World Generation|Island Generation|Graph")
	FMapCorner GetCorner(const int32 index) const;
	// Returns a MapEdge at the given index.
	// If the index is invalid, an empty MapEdge will be returned.
	// This empty MapEdge will have an index of -1.
	// Make sure to use the function UpdateEdge() to update the graph
	// after the MapEdge has been modified. 
	UFUNCTION(BlueprintPure, Category = "World Generation|Island Generation|Graph")
	FMapEdge GetEdge(const int32 index) const;

	// Returns the MapEdge defined by two adjacent MapCenters.
	// If the edge doesn't exist, an empty MapEdge will be returned.
	// This empty MapEdge will have an index of -1.
	// Make sure to use the function UpdateEdge() to update the graph
	// after the MapEdge has been modified.
	UFUNCTION(BlueprintPure, Category = "World Generation|Island Generation|Graph")
	FMapEdge FindEdgeFromCenters(const FMapCenter& v0, const FMapCenter& v1) const;
	// Returns the MapEdge defined by two adjacent MapCorners.
	// If the edge doesn't exist, an empty MapEdge will be returned.
	// This empty MapEdge will have an index of -1.
	// Make sure to use the function UpdateEdge() to update the graph
	// after the MapEdge has been modified.
	UFUNCTION(BlueprintPure, Category = "World Generation|Island Generation|Graph")
	FMapEdge FindEdgeFromCorners(const FMapCorner& v0, const FMapCorner& v1) const;

	// Returns the number of MapCenters in our graph.
	UFUNCTION(BlueprintPure, Category = "World Generation|Island Generation|Graph")
	int32 GetCenterNum() const;
	// Returns the number of MapCorners in our graph.
	UFUNCTION(BlueprintPure, Category = "World Generation|Island Generation|Graph")
	int32 GetCornerNum() const;
	// Returns the number of MapEdges in our graph.
	UFUNCTION(BlueprintPure, Category = "World Generation|Island Generation|Graph")
	int32 GetEdgeNum() const;

	// Returns our Graph.
	UFUNCTION(BlueprintPure, Category = "World Generation|Island Generation|Graph")
	UPolygonMap* GetGraph() const;
	// Returns the heightmap associated with this island.
	// This heightmap will be blank until CreateHeightmap() completes.
	// Be sure to call CreateHeightmap() and wait for delegate completion before calling this function.
	UFUNCTION(BlueprintPure, Category = "World Generation|Island Generation|Graph")
	UPolygonalMapHeightmap* GetHeightmap() const;

	// Update a MapCenter in the graph after it has been modified.
	UFUNCTION(BlueprintCallable, Category = "World Generation|Island Generation|Graph")
	void UpdateCenter(const FMapCenter& center);
	
	// Update a MapCorner in the graph after it has been modified.
	UFUNCTION(BlueprintCallable, Category = "World Generation|Island Generation|Graph")
	void UpdateCorner(const FMapCorner& corner);
	
	// Update a MapEdge in the graph after it has been modified.
	UFUNCTION(BlueprintCallable, Category = "World Generation|Island Generation|Graph")
	void UpdateEdge(const FMapEdge& edge);

	// This adds a new generation step to the list of island generation steps.
	UFUNCTION(BlueprintCallable, Category = "World Generation|Island Generation")
	void AddGenerationStep(const FIslandGeneratorDelegate step);

	// This clears all generation steps from the island generator.
	UFUNCTION(BlueprintCallable, Category = "World Generation|Island Generation")
	void ClearAllGenerationSteps();

	// This creates a 2D heightmap representation of the current island.
	// It will take a few seconds to complete, particularly for larger values of HeightmapSize.
	// This work is multi-threaded, so the game thread will not lock up while this process completes.
	// When the function is done, it will call the OnComplete delegate.
	// The heightmap can be accessed through the GetHeightmap() function.
	UFUNCTION(BlueprintCallable, Category = "World Generation|Island Generation|Map")
	void CreateHeightmap(const int32 HeightmapSize, const FIslandGeneratorDelegate OnComplete);

	// This draws a debug voronoi representation of the island, using the values specified in
	// the IslandData's PolygonMapSettings struct.
	UFUNCTION(BlueprintCallable, Category = "World Generation|Island Generation|Debug")
	void DrawVoronoiGraph();

	// This draws a debug delaunay representation of the island, using the values specified in
	// the IslandData's PolygonMapSettings struct.
	UFUNCTION(BlueprintCallable, Category = "World Generation|Island Generation|Debug")
	void DrawDelaunayGraph();

	// Draws this map's heightmap in Unreal world space, using the values specified in the
	// IslandData's PolygonMapSettings struct.
	// Be sure to call CreateHeightmap() and wait for the delegate to complete before calling
	// this function, otherwise there may not  be any data in the heightmap array.
	UFUNCTION(BlueprintCallable, Category = "World Generation|Island Generation|Debug")
	void DrawHeightmap(float PixelSize = 100.0f);

	// Different settings that make up our island.
	// Changing these settings will produce different-looking islands.
	UPROPERTY(Category = "Island", BlueprintReadWrite, EditAnywhere)
	FIslandData IslandData;

protected:
	// Adds a list of steps that we need to generate our island.
	// Users using Blueprint should override this and NOT call the
	// parent function. They have to add steps in the Event Graph
	// due to the way Blueprint works with delegates ("Events").
	UFUNCTION(BlueprintNativeEvent, Category = "World Generation|Island Generation")
	void AddMapSteps();
	virtual void AddMapSteps_Implementation();

	// Generates a new map from our IslandData.
	// This iterates over each step in the IslandGeneratorSteps queue
	// and executes the event. Players using Blueprint can add their
	// own steps to the queue or create a new queue altogether in the
	// Blueprint Event Graph.
	UFUNCTION(BlueprintCallable, Category = "World Generation|Island Generation")
	void ExecuteNextMapStep();

	UFUNCTION(BlueprintImplementableEvent, Category = "World Generation|Island Generation|Map")
	void ResetMapEvent();

	UFUNCTION(BlueprintImplementableEvent, Category = "World Generation|Island Generation|Map")
	void InitializeMapEvent();
	// Initializes the IslandShape and PointSelector classes
	UFUNCTION(BlueprintCallable, Category = "World Generation|Island Generation|Map")
	void InitializeMapClasses();

	UFUNCTION(BlueprintImplementableEvent, Category = "World Generation|Island Generation|Map")
	void BuildGraphEvent();
	// Creates the initial points that our map will work on
	UFUNCTION(BlueprintCallable, Category = "World Generation|Island Generation|Map")
	void BuildGraph();

	UFUNCTION(BlueprintImplementableEvent, Category = "World Generation|Island Generation|Map")
	void AssignElevationEvent();
	// Determines graph elevation
	UFUNCTION(BlueprintCallable, Category = "World Generation|Island Generation|Map")
	void AssignElevation();

	UFUNCTION(BlueprintImplementableEvent, Category = "World Generation|Island Generation|Map")
	void AssignMoistureEvent();
	// Determines downslopes and moisture
	UFUNCTION(BlueprintCallable, Category = "World Generation|Island Generation|Map")
	void AssignMoisture();

	UFUNCTION(BlueprintCallable, Category = "World Generation|Island Generation|Map")
	virtual void DoPointPostProcess();
	
	UFUNCTION(BlueprintImplementableEvent, Category = "World Generation|Island Generation|Map")
	void NormalizePointsEvent();
	// Normalizes all points in their acceptable range (elevation/moisture between 0 and 1, for example).
	// Should be called when you are done playing with height/moisture.
	UFUNCTION(BlueprintCallable, Category = "World Generation|Island Generation|Map")
	void NormalizePoints();

	UFUNCTION(BlueprintImplementableEvent, Category = "World Generation|Island Generation|Map")
	void DetermineBiomesEvent();
	// Determines the biomes for all corners and centers.
	// Should be called after the bulk of map generation is complete.
	UFUNCTION(BlueprintCallable, Category = "World Generation|Island Generation|Map")
	void DetermineBiomes();

	// Called when the heightmap is done generating.
	// Stops keeping track of the time the heightmap has been running and calls our OnHeightmapComplete delegate.
	UFUNCTION()
	void OnHeightmapFinished();
private:
	// The map's heightmap, generated after map generation is finished.
	UPROPERTY()
	UPolygonalMapHeightmap* MapHeightmap;

	// The random generator to use when generating the map.
	UPROPERTY()
	FRandomStream RandomGenerator;
	// What time the map started being generated.
	UPROPERTY()
	float MapStartGenerationTime;
	// What time this current step started.
	UPROPERTY()
	float CurrentGenerationTime;

	// Is the current map generation step done?
	UPROPERTY()
	bool bCurrentStepIsDone;
	// Have we generated a heightmap yet?
	UPROPERTY()
	bool bHasGeneratedHeightmap;

	// A list of all the heightmap steps we need to take.
	TQueue<FIslandGeneratorDelegate> IslandGeneratorSteps;

	// A delegate called when initial map generation is done.
	FIslandGeneratorDelegate OnGenerationComplete;
	// A delegate called when heightmap generation is done.
	FIslandGeneratorDelegate OnHeightmapComplete;
public:
	// The graph containing all our map's polygons.
	// This is the "main" representation of our map, and the source of all our raw data.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Island Generation")
	UPolygonMap* MapGraph;
	// The instance of our IslandShape object.
	// IslandShapes determine the shape of the island.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Island Generation")
	UIslandShape* IslandShape;
	// The instance of our PointGenerator object.
	// The PointGenerator determines the manner in which our points are selected.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Island Generation")
	UPointGenerator* PointSelector;
	// The instance of our ElevationDistributor object.
	// This controls how elevation is distributed.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Island Generation")
	UElevationDistributor* ElevationDistributor;
	// The instance of our MoistureDistributor object.
	// This controls how moisture is distributed.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Island Generation")
	UMoistureDistributor* MoistureDistributor;
	// The instance of our BiomeManager object.
	// This controls what biomes get assigned to points.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Island Generation")
	UBiomeManager* BiomeManager;
};
