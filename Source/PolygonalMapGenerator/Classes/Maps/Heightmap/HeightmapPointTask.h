// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Array.h"
#include "Maps/Biomes/BiomeManager.h"
#include "Maps/PolygonMap.h"
#include "Async/TaskGraphInterfaces.h"

struct FMapData;
class UPolygonalMapHeightmap;

enum EPointSelectionMode : uint8
{
	// Each pixel is a reflection of the entire polygon with no blending.
	// This creates a "stepped" look and is very fast.
	UsePolygon,
	// This uses the cached MapData array and interpolates between the nearest point.
	// This provides decent results, but is slow.
	Interpolated,
	// The same as using interpolated, but biomes are just fetched from the parent polygon.
	// This provides consistent results (free from errors generated from interpolation),
	// but polygon edges are very distinct
	InterpolatedWithPolygonBiome
};

/**
 * 
 */
class FHeightmapPointTask
{

public:
	FHeightmapPointTask(int32 XCoord, int32 YCoord, int32 NumberOfPoints, EPointSelectionMode SelectionMode)
	{
		X = XCoord;
		Y = YCoord;
		NumberOfPointsToAverage = NumberOfPoints;
		PointSelectionMode = SelectionMode;
	}

	int32 X;
	int32 Y;
	int32 NumberOfPointsToAverage;
	EPointSelectionMode PointSelectionMode;

	/** return the name of the task **/
	static const TCHAR* GetTaskName()
	{
		return TEXT("FHeightmapPointTask");
	}
	FORCEINLINE static TStatId GetStatId()
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FHeightmapPointTask, STATGROUP_TaskGraphTasks);
	}
	/** return the thread for this task **/
	static ENamedThreads::Type GetDesiredThread()
	{
		return ENamedThreads::AnyThread;
	}

	static ESubsequentsMode::Type GetSubsequentsMode()
	{
		return ESubsequentsMode::FireAndForget;
	}

	// Creates a map point at the given pixel position and adds it to the end of the HeightmapData array.
	FMapData MakeMapPoint(FVector2D PixelPosition, TArray<FMapData> MapData, UBiomeManager* BiomeManager);

	// Do the task
	void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent);
};

class FHeightmapPointGenerator
{
public:
	// The heightmap generator
	static UPolygonalMapHeightmap* MapHeightmap;
	static UPolygonMap* MapGraph;
	static UBiomeManager* BiomeManager;

	// Results of the threads
	static TArray<FMapData> HeightmapData;

	// This is the array of thread completions, used to determine if all threads are done
	static FGraphEventArray CompletionEvents;

	static int32 CompletedThreads;
	static int32 TotalNumberOfThreads;
	static FIslandGeneratorDelegate OnAllPointsComplete;

	// Are all tasks complete?
	static bool TasksAreComplete();

	// Initiation point
	static void GenerateHeightmapPoints(const int32 HeightmapSize, int32 NumberOfPointsToAverage, UPolygonalMapHeightmap* HeightmapGenerator, UPolygonMap* Graph, UBiomeManager* BiomeMgr, const FIslandGeneratorDelegate OnComplete);

	static void CheckComplete();
};