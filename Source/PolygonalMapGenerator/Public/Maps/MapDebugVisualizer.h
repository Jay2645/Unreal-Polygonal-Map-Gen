// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "PolygonMap.h"
#include "MapDebugVisualizer.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class POLYGONALMAPGENERATOR_API UMapDebugVisualizer : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()	
public:
	// Draws a debug grid showing the position and height of each FMapData "pixel" in this heightmap.
	// Keep in mind that larger values of HeightmapSize will have a SEVERE performance impact.
	// Try to only use this with a limited number of pixels in the heightmap.
	UFUNCTION(BlueprintCallable, Category = "Map Generation|Debug")
	static void DrawDebugPixelGrid(AActor* Actor, const FWorldSpaceMapData& MapData, const TArray<FMapData>& HeightmapData, int32 HeightmapSize, float PixelSize);
	
	UFUNCTION(BlueprintCallable, Category = "Map Generation|Debug")
	static void DrawDebugPixelRivers(AActor* Actor, const FWorldSpaceMapData& MapData, const TArray<FMapData>& HeightmapData, int32 HeightmapSize, float PixelSize);
	
	// Draws a 3D version of the underlying Delaunay graph in world space.
	// This graph is a collection of all the map's centers and the
	// Delaunay edges which connect them.
	UFUNCTION(BlueprintCallable, Category = "Map Generation|Debug")
	static void DrawDebugVoronoiGrid(AActor* Actor, const FWorldSpaceMapData& MapData, const TArray<FMapCorner>& Corners, const TArray<FMapEdge>& Edges, int32 MapSize);

	// Draws a 3D version of the underlying Voronoi graph in world space.
	// This graph is a collection of all the map's corners and the
	// Voronoi edges which connect them.
	UFUNCTION(BlueprintCallable, Category = "Map Generation|Debug")
	static void DrawDebugDelaunayGrid(AActor* Actor, const FWorldSpaceMapData& MapData, const TArray<FMapCenter>& Centers, const TArray<FMapEdge>& Edges, int32 MapSize);
};
