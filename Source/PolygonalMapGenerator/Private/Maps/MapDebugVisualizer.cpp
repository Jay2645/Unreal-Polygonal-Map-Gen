// Fill out your copyright notice in the Description page of Project Settings.

#include "PolygonalMapGeneratorPrivatePCH.h"
#include "MapDataHelper.h"
#include "DrawDebugHelpers.h"
#include "PolygonMap.h"
#include "MapDebugVisualizer.h"

void UMapDebugVisualizer::DrawDebugPixelGrid(AActor* Actor, const FWorldSpaceMapData& MapData, const TArray<FMapData>& HeightmapData, int32 HeightmapSize, float PixelSize)
{
	UWorld* world = Actor->GetWorld();
	if (world == NULL)
	{
		UE_LOG(LogWorldGen, Error, TEXT("World was null!"));
		return;
	}
	FVector offset = FVector(0.0f, 0.0f, 0.0f);

	for (int32 x = 0; x < HeightmapSize; x++)
	{
		for (int32 y = 0; y < HeightmapSize; y++)
		{
			FMapData mapData = HeightmapData[x + (y * HeightmapSize)];

			FColor color = FColor(255, 255, 255);
			if (UMapDataHelper::IsOcean(mapData))
			{
				color = FColor(0, 27, 71);
			}
			else if (UMapDataHelper::IsCoast(mapData))
			{
				color = FColor(236, 242, 135);
			}
			else if (UMapDataHelper::IsWater(mapData))
			{
				color = FColor(87, 133, 209);
			}
			else if (UMapDataHelper::IsRiver(mapData))
			{
				color = FColor(147, 198, 255);
			}

			FVector v0 = offset + FVector(mapData.Point.X * PixelSize, mapData.Point.Y * PixelSize, FMath::FloorToInt(mapData.Elevation * (MapData.ElevationScale / 100.0f)) * PixelSize + 1.0f);
			FVector v1 = FVector(v0.X, v0.Y + PixelSize, v0.Z);
			FVector v2 = FVector(v0.X + PixelSize, v0.Y, v0.Z);
			FVector v3 = FVector(v2.X, v1.Y, v0.Z);
			//DrawDebugSphere(world, v0, 100, 4, color, true);
			DrawDebugLine(world, v0, v1, color, true);
			DrawDebugLine(world, v0, v2, color, true);
			DrawDebugLine(world, v3, v2, color, true);
			DrawDebugLine(world, v3, v1, color, true);
		}
	}
}

void UMapDebugVisualizer::DrawDebugPixelRivers(AActor* Actor, const FWorldSpaceMapData& MapData, const TArray<FMapData>& HeightmapData, int32 HeightmapSize, float PixelSize)
{
	UWorld* world = Actor->GetWorld();
	if (world == NULL)
	{
		UE_LOG(LogWorldGen, Error, TEXT("World was null!"));
		return;
	}
	FVector offset = FVector(0.0f, 0.0f, 0.0f);

	for (int32 x = 0; x < HeightmapSize; x++)
	{ 
		for (int32 y = 0; y < HeightmapSize; y++)
		{
			FMapData mapData = HeightmapData[x + (y * HeightmapSize)];
			if (!UMapDataHelper::IsRiver(mapData))
			{
				continue;
			}

			FColor color = FColor(147, 198, 255);

			FVector v0 = offset + FVector(mapData.Point.X * PixelSize, mapData.Point.Y * PixelSize, FMath::FloorToInt(mapData.Elevation * (MapData.ElevationScale / 100.0f)) * PixelSize + 1.0f);
			FVector v1 = FVector(v0.X, v0.Y + PixelSize, v0.Z);
			FVector v2 = FVector(v0.X + PixelSize, v0.Y, v0.Z);
			FVector v3 = FVector(v2.X, v1.Y, v0.Z);
			//DrawDebugSphere(world, v0, 100, 4, color, true);
			DrawDebugLine(world, v0, v1, color, true);
			DrawDebugLine(world, v0, v2, color, true);
			DrawDebugLine(world, v3, v2, color, true);
			DrawDebugLine(world, v3, v1, color, true);
		}
	}
}

void UMapDebugVisualizer::DrawDebugVoronoiGrid(AActor* Actor, const FWorldSpaceMapData& MapData, const TArray<FMapCorner>& Corners, const TArray<FMapEdge>& Edges, int32 MapSize)
{
	UWorld* world = Actor->GetWorld();
	if (world == NULL)
	{
		UE_LOG(LogWorldGen, Error, TEXT("World was null!"));
		return;
	}

	for (int i = 0; i < Corners.Num(); i++)
	{
		FMapData mapData = Corners[i].CornerData;

		FColor color = FColor(255, 255, 255);

		if (UMapDataHelper::IsBorder(mapData))
		{
			color = FColor(221, 24, 24);
		}
		else if (Corners[i].RiverSize > 0)
		{
			color = FColor(147, 198, 255);
		}
		if (UMapDataHelper::IsOcean(mapData))
		{
			color = FColor(0, 27, 71);
		}
		else if (UMapDataHelper::IsCoast(mapData))
		{
			color = FColor(236, 242, 135);
		}
		else if (UMapDataHelper::IsWater(mapData))
		{
			color = FColor(87, 133, 209);
		}

		FVector worldLocation = UPolygonMap::ConvertGraphPointToWorldSpace(mapData, MapData, MapSize);
		DrawDebugSphere(world, worldLocation, MapData.PointSize, 4, color, true);
	}

	for (int i = 0; i < Edges.Num(); i++)
	{
		int32 worldIndex0;
		int32 worldIndex1;

		worldIndex0 = Edges[i].VoronoiEdge0;
		if (worldIndex0 < 0)
		{
			continue;
		}
		FMapData v0Data = Corners[worldIndex0].CornerData;
		FVector worldVertex0 = UPolygonMap::ConvertGraphPointToWorldSpace(v0Data, MapData, MapSize);

		worldIndex1 = Edges[i].VoronoiEdge1;
		if (worldIndex1 < 0)
		{
			continue;
		}
		FMapData v1Data = Corners[worldIndex1].CornerData;
		FVector worldVertex1 = UPolygonMap::ConvertGraphPointToWorldSpace(v1Data, MapData, MapSize);

		FColor color = FColor(255, 255, 255);
		if (UMapDataHelper::IsBorder(v0Data) || UMapDataHelper::IsBorder(v1Data))
		{
			color = FColor(221, 24, 24);
		}
		else if (Edges[i].RiverVolume > 0)
		{
			color = FColor(147, 198, 255);
		}
		else if (UMapDataHelper::IsOcean(v0Data) || UMapDataHelper::IsOcean(v1Data))
		{
			color = FColor(0, 27, 71);
		}
		else if (UMapDataHelper::IsWater(v0Data) || UMapDataHelper::IsWater(v1Data))
		{
			color = FColor(87, 133, 209);
		}
		else if (UMapDataHelper::IsCoast(v0Data) || UMapDataHelper::IsCoast(v1Data))
		{
			color = FColor(236, 242, 135);
		}

		DrawDebugLine(world, worldVertex0, worldVertex1, color, true);
	}
}

void UMapDebugVisualizer::DrawDebugDelaunayGrid(AActor* Actor, const FWorldSpaceMapData& MapData, const TArray<FMapCenter>& Centers, const TArray<FMapEdge>& Edges, int32 MapSize)
{
	UWorld* world = Actor->GetWorld();
	if (world == NULL)
	{
		UE_LOG(LogWorldGen, Error, TEXT("World was null!"));
		return;
	}

	// Draw centers
	for (int i = 0; i < Centers.Num(); i++)
	{
		FMapData mapData = Centers[i].CenterData;

		FColor color = FColor(255, 255, 255);
		/*if (Corners[i].bIsBorder)
		{
		color = FColor(255, 0, 0);
		}
		else */if (UMapDataHelper::IsOcean(mapData))
		{
			color = FColor(0, 0, 255);
		}
		else if (UMapDataHelper::IsCoast(mapData))
		{
			color = FColor(255, 255, 0);
		}
		else if (UMapDataHelper::IsWater(mapData))
		{
			color = FColor(0, 255, 255);
		}

		FVector worldLocation = UPolygonMap::ConvertGraphPointToWorldSpace(mapData, MapData, MapSize);
		DrawDebugSphere(world, worldLocation, MapData.PointSize, 4, color, true);
	}


	for (int i = 0; i < Edges.Num(); i++)
	{
		int32 worldIndex0;
		int32 worldIndex1;

		worldIndex0 = Edges[i].DelaunayEdge0;
		if (worldIndex0 < 0)
		{
			UE_LOG(LogWorldGen, Error, TEXT("No Delaunay Edge 0 for %d!"), i);
			continue;
		}
		FMapData d0Data = Centers[worldIndex0].CenterData;
		FVector worldVertex0 = UPolygonMap::ConvertGraphPointToWorldSpace(d0Data, MapData, MapSize);

		worldIndex1 = Edges[i].DelaunayEdge1;
		if (worldIndex1 < 0)
		{
			UE_LOG(LogWorldGen, Error, TEXT("No Delaunay Edge 1 for %d!"), i);
			continue;
		}
		FMapData d1Data = Centers[worldIndex1].CenterData;
		FVector worldVertex1 = UPolygonMap::ConvertGraphPointToWorldSpace(d1Data, MapData, MapSize);

		FColor color = FColor(255, 255, 255);
		/*if (Centers[worldIndex0].bIsBorder && Centers[worldIndex1].bIsBorder)
		{
		color = FColor(255, 0, 0);
		}
		else */if (UMapDataHelper::IsOcean(d0Data) && UMapDataHelper::IsOcean(d1Data))
		{
			color = FColor(0, 0, 255);
		}
		else if (UMapDataHelper::IsWater(d0Data) && UMapDataHelper::IsWater(d1Data))
		{
			color = FColor(0, 255, 0);
		}
		else if (UMapDataHelper::IsCoast(d0Data) && UMapDataHelper::IsCoast(d1Data))
		{
			color = FColor(255, 255, 0);
		}
		else if (UMapDataHelper::IsBorder(d0Data) || UMapDataHelper::IsBorder(d1Data))
		{
			color = FColor(128, 10, 10);
		}
		else if (UMapDataHelper::IsOcean(d0Data) || UMapDataHelper::IsOcean(d1Data))
		{
			color = FColor(10, 10, 128);
		}
		else if (UMapDataHelper::IsWater(d0Data) || UMapDataHelper::IsWater(d1Data))
		{
			color = FColor(10, 128, 10);
		}
		else if (UMapDataHelper::IsCoast(d0Data) || UMapDataHelper::IsCoast(d1Data))
		{
			color = FColor(128, 128, 10);
		}

		DrawDebugLine(world, worldVertex0, worldVertex1, color, true);
	}
}