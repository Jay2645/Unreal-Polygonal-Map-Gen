// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Array.h"
#include "Maps/Biomes/BiomeManager.h"
#include "Maps/PolygonMap.h"
#include "Async/TaskGraphInterfaces.h"

struct FMapData;
class UPolygonalMapHeightmap;

/**
 * 
 */
class FHeightmapPointTask
{

public:
	FHeightmapPointTask(int32 XCoord, int32 YCoord, int32 NumberOfPoints)
	{
		X = XCoord;
		Y = YCoord;
		NumberOfPointsToAverage = NumberOfPoints;
	}

	int32 X;
	int32 Y;
	int32 NumberOfPointsToAverage;

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