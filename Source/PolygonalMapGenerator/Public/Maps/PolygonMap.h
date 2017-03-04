// Original Work Copyright (c) 2010 Amit J Patel, Modified Work Copyright (c) 2016 Jay M Stevens

#pragma once

#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameplayTagContainer.h"
#include "PolygonMap.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogWorldGen, Log, All);
DECLARE_DYNAMIC_DELEGATE(FIslandGeneratorDelegate);

USTRUCT(BlueprintType)
struct POLYGONALMAPGENERATOR_API FMapData
{
	GENERATED_BODY()

	// Location in the map itself
	UPROPERTY(Category = "Map Graph", BlueprintReadWrite, EditAnywhere)
	FVector2D Point;

	// The height of this MapData (0.0 - 1.0)
	UPROPERTY(Category = "Map Biome", BlueprintReadWrite, EditAnywhere)
	float Elevation;
	// How much moisture this MapData receives (0.0 - 1.0)
	UPROPERTY(Category = "Map Biome", BlueprintReadWrite, EditAnywhere)
	float Moisture;

	// The Biome this MapData resides in.
	// Biomes are determined by the BiomeManager class.
	// Users can override the BiomeManager class if they would like to implement a different biome distribution.
	UPROPERTY(Category = "Map Biome", BlueprintReadWrite, EditAnywhere, meta = (Categories = "MapData.Biome"))
	FGameplayTag Biome;

	// Any special tags associated with the MapData object.
	// This can be used to tag this with things such as "volcano", "water", "player start", etc.
	// Later steps in world generation can then look at the tags and give this point special behavior based on them.
	UPROPERTY(Category = "Map Biome", BlueprintReadWrite, EditAnywhere, meta = (Categories = "MapData.MetaData"))
	FGameplayTagContainer Tags;

	FMapData()
	{
		Point = FVector2D::ZeroVector;

		Tags.AddTagFast(FGameplayTag::RequestGameplayTag(TEXT("MapData.MetaData.Border")));
		Tags.AddTagFast(FGameplayTag::RequestGameplayTag(TEXT("MapData.MetaData.Water.Saltwater")));

		Elevation = 0.0f;
		Moisture = 0.0f;
	}
};

struct FMapCorner;
struct FMapEdge;
class URiver;

USTRUCT(BlueprintType)
struct POLYGONALMAPGENERATOR_API FMapCenter
{
	GENERATED_BODY()
	// The index of this Center.
	// This can be used to look up the "main" reference of this Center in the
	// main UPolygonMap class.
	UPROPERTY(Category = "Map Graph", BlueprintReadWrite, VisibleAnywhere)
	int32 Index;

	// All metadata related to this FMapCenter object.
	UPROPERTY(Category = "Map Graph", BlueprintReadWrite, EditAnywhere)
	FMapData CenterData;

	// The index of all neighboring Centers
	UPROPERTY(Category = "Map Graph", BlueprintReadOnly, VisibleAnywhere)
	TArray<int32> Neighbors;
	// The index of all Edges that make up our borders
	UPROPERTY(Category = "Map Graph", BlueprintReadOnly, VisibleAnywhere)
	TArray<int32> Borders;
	// The index of all Corners of our polygon
	UPROPERTY(Category = "Map Graph", BlueprintReadOnly, VisibleAnywhere)
	TArray<int32> Corners;

	FMapCenter()
	{
		Index = -1;
	}

	FORCEINLINE	bool operator==(const FMapCenter& Other) const
	{
		return Other.Index == Index;
	}
};

USTRUCT(BlueprintType)
struct POLYGONALMAPGENERATOR_API FMapCorner
{
	GENERATED_BODY()
	// The index of this Corner.
	// This can be used to look up the "main" reference of this Corner in the
	// main UPolygonMap class.
	UPROPERTY(Category = "Map Graph", BlueprintReadWrite, VisibleAnywhere)
	int32 Index;

	// All metadata related to this FMapCorner object.
	UPROPERTY(Category = "Map Graph", BlueprintReadWrite, EditAnywhere)
	FMapData CornerData;

	// The index of all Centers that touch this Corner
	UPROPERTY(Category = "Map Graph", BlueprintReadOnly, VisibleAnywhere)
	TArray<int32> Touches;
	// The index of all Edges that have one end terminating at this Corner
	UPROPERTY(Category = "Map Graph", BlueprintReadOnly, VisibleAnywhere)
	TArray<int32> Protrudes;
	// The index of all Corners adjacent to this Corner
	UPROPERTY(Category = "Map Graph", BlueprintReadOnly, VisibleAnywhere)
	TArray<int32> Adjacent;

	// Pointer to adjacent Corner most downhill from us, or -1
	UPROPERTY(Category = "Watershed", BlueprintReadWrite, EditAnywhere)
	int32 Downslope;
	// Pointer to coastal Corner, or -1 for null
	UPROPERTY(Category = "Watershed", BlueprintReadWrite, EditAnywhere)
	int32 Watershed;
	// The size of the watershed
	UPROPERTY(Category = "Watershed", BlueprintReadWrite, EditAnywhere)
	int32 WatershedSize;
	// The size of the river flowing through this corner.
	// If there is no river, this is 0.
	UPROPERTY(Category = "River", BlueprintReadWrite, EditAnywhere)
	int32 RiverSize;
	// This index refers to where in the river this point resides.
	// It refers to an array of FMapCorners in the URiver object.
	// If there is no river, this is -1.
	UPROPERTY(Category = "River", BlueprintReadWrite, EditAnywhere)
	int32 RiverIndex;
	// A pointer to any River which flows through this corner.
	UPROPERTY(Category = "River", BlueprintReadWrite, EditAnywhere)
	URiver* River;

	FMapCorner()
	{
		Index = -1;
		RiverSize = 0;
		Downslope = -1;
		Watershed = -1;
		RiverIndex = -1;
		WatershedSize = 0;
		River = NULL;
	}

	FORCEINLINE bool operator <(const FMapCorner& c) const
	{
		return CornerData.Elevation < c.CornerData.Elevation;
	}

	FORCEINLINE	bool operator==(const FMapCorner& Other) const
	{
		return Other.Index == Index;
	}
};

USTRUCT(BlueprintType)
struct POLYGONALMAPGENERATOR_API FMapEdge
{
	GENERATED_BODY()
	// The index of this Edge
	UPROPERTY(Category = "Map Graph", BlueprintReadWrite, VisibleAnywhere)
	int32 Index;

	// The Delaunay Edge points to a center.
	// Right now, this is just an index of that center, rather than an actual reference to it.
	// I'm not happy with this implementation, so it may be changed in the future.
	UPROPERTY(Category = "Map Graph", BlueprintReadOnly, VisibleAnywhere)
	int32 DelaunayEdge0;
	// The Delaunay Edge points to a center.
	// Right now, this is just an index of that center, rather than an actual reference to it.
	// I'm not happy with this implementation, so it may be changed in the future.
	UPROPERTY(Category = "Map Graph", BlueprintReadOnly, VisibleAnywhere)
	int32 DelaunayEdge1;

	// The Voronoi Edge points to a corner.
	// Right now, this is just an index of that corner, rather than an actual reference to it.
	// I'm not happy with this implementation, so it may be changed in the future.
	UPROPERTY(Category = "Map Graph", BlueprintReadOnly, VisibleAnywhere)
	int32 VoronoiEdge0;
	// The Voronoi Edge points to a corner.
	// Right now, this is just an index of that corner, rather than an actual reference to it.
	// I'm not happy with this implementation, so it may be changed in the future.
	UPROPERTY(Category = "Map Graph", BlueprintReadOnly, VisibleAnywhere)
	int32 VoronoiEdge1;

	// The midway point between our two voronoi edges
	UPROPERTY(Category = "Map Graph", BlueprintReadOnly, VisibleAnywhere)
	FVector2D Midpoint;

	// The volume of the river flowing through this edge, or 0
	UPROPERTY(Category = "Map Biome", BlueprintReadWrite, EditAnywhere)
	int32 RiverVolume;

	//Constructor
	FMapEdge()
	{
		Index = -1;
		DelaunayEdge0 = -1;
		DelaunayEdge1 = -1;
		VoronoiEdge0 = -1;
		VoronoiEdge1 = -1;

		RiverVolume = 0;
	}

	FORCEINLINE	bool operator==(const FMapEdge& Other) const
	{
		return Other.Index == Index;
	}
};

/**
* This class governs how the 2D FMapData objects act in "world space".
* This is used showing map debug, as well as converting from "Polygon Map" space to 3D Unreal world space.
*/
USTRUCT(BlueprintType)
struct POLYGONALMAPGENERATOR_API FWorldSpaceMapData
{
	GENERATED_BODY()

	// The size each debug point is rendered
	UPROPERTY(Category = "Map", BlueprintReadWrite, EditAnywhere)
	int32 PointSize;
	// The Z offset of the bottom of the ocean -- what's the lowest
	// point on the entire map?
	UPROPERTY(Category = "Map", BlueprintReadWrite, EditAnywhere)
	float ElevationOffset;
	// How much to scale the map on the XY scale.
	UPROPERTY(Category = "Map", BlueprintReadWrite, EditAnywhere)
	float XYScaleFactor;
	// How tall the map should be -- how much higher is the highest
	// point in the map from the lowest point?
	UPROPERTY(Category = "Map", BlueprintReadWrite, EditAnywhere)
	float ElevationScale;

	FWorldSpaceMapData()
	{
		PointSize = 16;
		ElevationOffset = 0.0f;
		XYScaleFactor = 100.0f;
		ElevationScale = 3200.0f;
	}
};

/**
* The PolygonMap is a class which uses a Voronoi diagram to collect data about a graph of
* points on the XY plane.
* These points are separated into map 'centers' (the points of the Delaunay 'dual graph'),
* map 'corners' (the points of the Voronoi graph), and map 'edges' (which make up the
* connections between Voronoi graphs and Delaunay graphs).
* Map centers and map corners both have metadata associated with them in the form of a
* 'MapData' class, which is used in the actual MapGenerator.
*/
UCLASS(Blueprintable)
class POLYGONALMAPGENERATOR_API UPolygonMap : public UActorComponent
{
	GENERATED_BODY()
	friend class AIslandMapGenerator;

public:
	// Creates the graph's initial points
	UFUNCTION(BlueprintCallable, Category = "Island Generation|Graph")
	void CreatePoints(UPointGenerator* PointSelector, const int32& numberOfPoints);

	// Build graph data structure in 'edges', 'centers', 'corners',
	// based on information in the Voronoi results: point.neighbors
	// will be a list of neighboring points of the same type (corner
	// or center); point.edges will be a list of edges that include
	// that point. Each edge connects to four points: the Voronoi edge
	// edge.{voronoiEdge0,voronoiEdge1} and its dual Delaunay triangle 
	// edge edge.{delaunayEdge0,delaunayEdge1}.0 For boundary
	// polygons, the Delaunay edge will have one null point, and the 
	// Voronoi edge may be null.
	UFUNCTION(BlueprintCallable, Category = "Island Generation|Graph")
	void BuildGraph(const int32& mapSize, const FWorldSpaceMapData& data);

	// Creates an FMapCenter from the given point.
	UFUNCTION(BlueprintCallable, Category = "Island Generation|Graph")
	FMapCenter MakeCenter(const FVector2D& point);
	// Creates an FMapCorner from the given point.
	UFUNCTION(BlueprintCallable, Category = "Island Generation|Graph")
	FMapCorner MakeCorner(const FVector2D& point);

	// Although Lloyd relaxation improves the uniformity of polygon
	// sizes, it doesn't help with the edge lengths. Short edges can
	// be bad for some games, and lead to weird artifacts on
	// rivers. We can easily lengthen short edges by moving the
	// corners, but **we lose the Voronoi property**.  The corners are
	// moved to the average of the polygon centers around them. Short
	// edges become longer. Long edges tend to become shorter. The
	// polygons tend to be more uniform after this step.
	UFUNCTION(BlueprintCallable, Category = "Island Generation|Graph")
	void ImproveCorners();

	// Returns a MapCenter at the given index.
	// If the index is invalid, an empty MapCenter will be returned.
	// This empty MapCenter will have an index of -1.
	// Make sure to use the function UpdateCenter() to update the graph
	// after the MapCenter has been modified. 
	UFUNCTION(BlueprintPure, Category = "Island Generation|Graph")
	FMapCenter GetCenter(const int32& index) const;
	// Returns a MapCorner at the given index.
	// If the index is invalid, an empty MapCorner will be returned.
	// This empty MapCorner will have an index of -1.
	// Make sure to use the function UpdateCorner() to update the graph
	// after the MapCorner has been modified. 
	UFUNCTION(BlueprintPure, Category = "Island Generation|Graph")
	FMapCorner GetCorner(const int32& index) const;
	// Returns a MapEdge at the given index.
	// If the index is invalid, an empty MapEdge will be returned.
	// This empty MapEdge will have an index of -1.
	// Make sure to use the function UpdateEdge() to update the graph
	// after the MapEdge has been modified. 
	UFUNCTION(BlueprintPure, Category = "Island Generation|Graph")
	FMapEdge GetEdge(const int32& index) const;

	// Update a MapCenter in the graph after it has been modified.
	// Due to the way Unreal handles TArrays, we can only return
	// a copy of an element, not the element itself.
	// I don't like this implementation, and it may be changed in the future.
	UFUNCTION(BlueprintCallable, Category = "Island Generation|Graph")
	void UpdateCenter(const FMapCenter& center);
	// Update a MapCorner in the graph after it has been modified.
	// Due to the way Unreal handles TArrays, we can only return
	// a copy of an element, not the element itself.
	// I don't like this implementation, and it may be changed in the future.
	UFUNCTION(BlueprintCallable, Category = "Island Generation|Graph")
	void UpdateCorner(const FMapCorner& corner);
	// Update a MapEdge in the graph after it has been modified.
	// Due to the way Unreal handles TArrays, we can only return
	// a copy of an element, not the element itself.
	// I don't like this implementation, and it may be changed in the future.
	UFUNCTION(BlueprintCallable, Category = "Island Generation|Graph")
	void UpdateEdge(const FMapEdge& edge);

	// Returns the number of MapCenters in our graph.
	UFUNCTION(BlueprintPure, Category = "Island Generation|Graph")
	int32 GetCenterNum() const;
	// Returns the number of MapCorners in our graph.
	UFUNCTION(BlueprintPure, Category = "Island Generation|Graph")
	int32 GetCornerNum() const;
	// Returns the number of MapEdges in our graph.
	UFUNCTION(BlueprintPure, Category = "Island Generation|Graph")
	int32 GetEdgeNum() const;

	// Returns the size of one side of our graph.
	// The full graph is a square with each side of this length.
	UFUNCTION(BlueprintPure, Category = "Island Generation|Graph")
	int32 GetGraphSize() const;


	// Returns ALL MapData points in our graph.
	// This is like returning a collection of all MapCenters and MapCorners.
	// Note that you cannot determine from a MapData object alone whether it
	// came from a MapCenter or a MapCorner, and that because of this, the
	// MapData object's "Index" property now means nothing. 2 different
	// MapData objects may share the same indicies.
	UFUNCTION(BlueprintPure, Category = "Island Generation|MapData")
	TArray<FMapData>& GetAllMapData();

	// Gets a list of all MapCorners which are not marked as ocean.
	UFUNCTION(BlueprintPure, Category = "Island Generation|Graph")
	TArray<int32> FindLandCorners() const;

	// Returns the MapEdge defined by two adjacent MapCenters.
	// If the edge doesn't exist, an empty MapEdge will be returned.
	// This empty MapEdge will have an index of -1.
	// Make sure to use the function UpdateEdge() to update the graph
	// after the MapEdge has been modified.
	// Due to the way Unreal handles TArrays, we can only return
	// a copy of an element, not the element itself.
	// I don't like this implementation, and it may be changed in the future.
	UFUNCTION(BlueprintPure, Category = "Island Generation|Graph")
	FMapEdge FindEdgeFromCenters(const FMapCenter& v0, const FMapCenter& v1) const;

	// Returns the MapEdge defined by two adjacent MapCorners.
	// If the edge doesn't exist, an empty MapEdge will be returned.
	// This empty MapEdge will have an index of -1.
	// Make sure to use the function UpdateEdge() to update the graph
	// after the MapEdge has been modified.
	// Due to the way Unreal handles TArrays, we can only return
	// a copy of an element, not the element itself.
	// I don't like this implementation, and it may be changed in the future.
	UFUNCTION(BlueprintPure, Category = "Island Generation|Graph")
	FMapEdge FindEdgeFromCorners(const FMapCorner& v0, const FMapCorner& v1) const;

	// Returns the MapCenter defined by two adjacent MapCorners.
	// If the center polygon doesn't exist, an empty MapCenter will be returned.
	// This empty MapCenter will have an index of -1.
	// Make sure to use the function UpdateCenter() to update the graph
	// after the MapCenter has been modified.
	// Due to the way Unreal handles TArrays, we can only return
	// a copy of an element, not the element itself.
	// I don't like this implementation, and it may be changed in the future.
	UFUNCTION(BlueprintPure, Category = "Island Generation|Graph")
	FMapCenter FindCenterFromCorners(FMapCorner CornerA, FMapCorner CornerB) const;

	// Compiles all map data into the CachedMapData array.
	// This is used by the PixelMap to create pixels.
	UFUNCTION(BlueprintCallable, Category = "Island Generation|Graph")
	void CompileMapData();

	// Converts a MapData object into the point it represents in 3D world space.
	// This can be used to mark where rivers are, to place props, etc.
	UFUNCTION(BlueprintPure, Category = "Island Generation")
	static FVector ConvertGraphPointToWorldSpace(const FMapData& MapData, const FWorldSpaceMapData& WorldData, int32 MapSize);

	// This takes a 2D point and returns a copy of the polygon encompassing that point.
	// If no polygon can be found, an empty MapCenter will be returned.
	// This empty MapCenter will have an index of -1.
	// Make sure to use the function UpdateCenter() to update the graph
	// after the MapCenter has been modified.
	// Due to the way Unreal handles TArrays, we can only return
	// a copy of an element, not the element itself.
	// I don't like this implementation, and it may be changed in the future.
	UFUNCTION(BlueprintPure, Category = "Island Generation|Graph")
	FMapCenter FindPolygonLocalSpace(const FVector2D& Point) const;

	// Checks if a given MapCenter polygon contains a 2D point.
	// If it does, this function returns true. Otherwise, it returns false.
	UFUNCTION(BlueprintPure, Category = "Island Generation|Graph")
	bool PolygonContainsPoint(const FVector2D& Point, const FMapCenter& Center) const;

	/// Graph Data
	// The points in our graph
	// I don't like this implementation, and it may be changed in the future.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Graph")
	TArray<FVector2D> Points;
	// The Centers in our graph
	// I don't like this implementation, and it may be changed in the future.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Graph")
	TArray<FMapCenter> Centers;
	// The Corners in our graph
	// I don't like this implementation, and it may be changed in the future.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Graph")
	TArray<FMapCorner> Corners;
	// The Edges of our graph
	// I don't like this implementation, and it may be changed in the future.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Graph")
	TArray<FMapEdge> Edges;

private:
	// This variable generates our initial points
	UPROPERTY()
	UPointGenerator* PointSelector;
	// The size of an edge in the 2D map
	UPROPERTY()
	int32 MapSize;

	// A conversion from local space to world space
	UPROPERTY()
	FWorldSpaceMapData MapData;

	// ALL MapData from both MapCenters and MapCorners.
	// Must be compiled first.
	UPROPERTY()
	TArray<FMapData> CachedMapData;
	// A map of centers connected to their location
	UPROPERTY()
	TMap<FVector2D, int32> CenterLookup;
	// A map of corners connected to their location
	UPROPERTY()
	TMap<FVector2D, int32> CornerLookup;

	// Returns true if 2 segments are intersecting, otherwise returns false.
	UFUNCTION()
	bool SegementsIntersect(const FMapEdge& Edge, const FVector2D& StartPoint, const FVector2D& EndPoint) const;
};
