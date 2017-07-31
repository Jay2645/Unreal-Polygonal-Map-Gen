// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "PolygonMap.h"
#include "Moisture/River.h"
#include "MapDebugVisualizer.generated.h"

/**
 * A helper class for visualizing our map.
 * Intended to be used for debugging purposes.
 */
UCLASS(BlueprintType)
class POLYGONALMAPGENERATOR_API UMapDebugVisualizer : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()	
public:
	// Draws a debug grid showing the position and height of each FMapData "pixel" in this heightmap.
	// Keep in mind that larger values of HeightmapSize will have a SEVERE performance impact.
	// Try to only use this with a limited number of pixels in the heightmap.
	UFUNCTION(BlueprintCallable, Category = "World Generation|Island Generation|Debug")
	static void DrawDebugPixelGrid(AActor* Actor, const FWorldSpaceMapData& MapData, const TArray<FMapData>& HeightmapData, int32 HeightmapSize, float PixelSize);
	
	// Same as DrawDebugPixelGrid(), but only draws the parts of the debug grid which contain a river.
	UFUNCTION(BlueprintCallable, Category = "World Generation|Island Generation|Debug")
	static void DrawDebugPixelRivers(AActor* Actor, const FWorldSpaceMapData& MapData, const TArray<FMapData>& HeightmapData, int32 HeightmapSize, float PixelSize);
	
	// Draws a 3D version of the underlying Delaunay graph in world space.
	// This graph is a collection of all the map's centers and the
	// Delaunay edges which connect them.
	UFUNCTION(BlueprintCallable, Category = "World Generation|Island Generation|Debug")
	static void DrawDebugVoronoiGrid(AActor* Actor, const FWorldSpaceMapData& MapData, UPolygonMap* PolygonMap);

	// Draws a 3D version of the underlying Voronoi graph in world space.
	// This graph is a collection of all the map's corners and the
	// Voronoi edges which connect them.
	UFUNCTION(BlueprintCallable, Category = "World Generation|Island Generation|Debug")
	static void DrawDebugDelaunayGrid(AActor* Actor, const FWorldSpaceMapData& MapData, UPolygonMap* PolygonMap);

	// Draws a triangle using the 3 given points.
	UFUNCTION(BlueprintCallable, Category = "World Generation|Island Generation|Debug")
	static void DrawTriangle(AActor* Actor, FVector PointA, FVector PointB, FVector PointC);

	// Draws all the rivers in our map, as smooth lines.
	UFUNCTION(BlueprintCallable, Category = "World Generation|Island Generation|Debug")
	static void DrawRivers(AActor* Actor, const FWorldSpaceMapData& MapData, UPolygonMap* MapGraph, const TArray<URiver*>& Rivers);
	// Draws a smooth curve in world space.
	UFUNCTION(BlueprintCallable, Category = "World Generation|Island Generation|Debug")
	static void DrawBeizerCurve(AActor* Actor, const FWorldSpaceMapData& WorldData, FVector2D v0, FVector2D control0, FVector2D v1, FVector2D control1, FColor color, int32 MapSize);

private:
	UFUNCTION(BlueprintPure, Category = "World Generation|Island Generation|Debug")
	static FVector2D CalculateBezierPoint(float t, FVector2D p0, FVector2D p1, FVector2D p2, FVector2D p3);
};
