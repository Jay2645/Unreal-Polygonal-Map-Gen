// Fill out your copyright notice in the Description page of Project Settings.

#include "PolygonalMapGeneratorPrivatePCH.h"
#include "MapDataHelper.h"
#include "DrawDebugHelpers.h"
#include "PolygonMap.h"
#include "MapDebugVisualizer.h"

void UMapDebugVisualizer::DrawDebugPixelGrid(AActor* Actor, const FWorldSpaceMapData& MapData, const TArray<FMapData>& HeightmapData, int32 HeightmapSize, float PixelSize, float PixelHeightMultiplier)
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

			float pixelHeight = FMath::FloorToInt(mapData.Elevation * ((MapData.ElevationScale / 100.0f) * PixelHeightMultiplier));

			FVector v0 = offset + FVector(x * PixelSize, y * PixelSize, pixelHeight);
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

void UMapDebugVisualizer::DrawDebugVoronoiGrid(AActor* Actor, const FWorldSpaceMapData& MapData, UPolygonMap* PolygonMap)
{
	UWorld* world = Actor->GetWorld();
	if (world == NULL)
	{
		UE_LOG(LogWorldGen, Error, TEXT("World was null!"));
		return;
	}

	TArray<FMapCorner> Corners = PolygonMap->GetCopyOfMapCornerArray();

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

		FVector worldLocation = PolygonMap->ConvertGraphPointToWorldSpace(mapData);
		DrawDebugSphere(world, worldLocation, MapData.PointSize, 4, color, true);
	}

	TArray<FMapEdge> Edges = PolygonMap->GetCopyOfMapEdgeArray();
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
		FVector worldVertex0 = PolygonMap->ConvertGraphPointToWorldSpace(v0Data);

		worldIndex1 = Edges[i].VoronoiEdge1;
		if (worldIndex1 < 0)
		{
			continue;
		}
		FMapData v1Data = Corners[worldIndex1].CornerData;
		FVector worldVertex1 = PolygonMap->ConvertGraphPointToWorldSpace(v1Data);

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

void UMapDebugVisualizer::DrawDebugDelaunayGrid(AActor* Actor, const FWorldSpaceMapData& MapData, UPolygonMap* PolygonMap)
{
	UWorld* world = Actor->GetWorld();
	if (world == NULL)
	{
		UE_LOG(LogWorldGen, Error, TEXT("World was null!"));
		return;
	}

	TArray<FMapCenter> Centers = PolygonMap->GetCopyOfMapCenterArray();
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

		FVector worldLocation = PolygonMap->ConvertGraphPointToWorldSpace(mapData);
		DrawDebugSphere(world, worldLocation, MapData.PointSize, 4, color, true);
	}


	TArray<FMapEdge> Edges = PolygonMap->GetCopyOfMapEdgeArray();
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
		FVector worldVertex0 = PolygonMap->ConvertGraphPointToWorldSpace(d0Data);

		worldIndex1 = Edges[i].DelaunayEdge1;
		if (worldIndex1 < 0)
		{
			UE_LOG(LogWorldGen, Error, TEXT("No Delaunay Edge 1 for %d!"), i);
			continue;
		}
		FMapData d1Data = Centers[worldIndex1].CenterData;
		FVector worldVertex1 = PolygonMap->ConvertGraphPointToWorldSpace(d1Data);

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

void UMapDebugVisualizer::DrawTriangle(AActor* Actor, FVector PointA, FVector PointB, FVector PointC)
{
	UWorld* world = Actor->GetWorld();
	if (world == NULL)
	{
		UE_LOG(LogWorldGen, Error, TEXT("World was null!"));
		return;
	}
	DrawDebugLine(world, PointA, PointB, FColor::White);
	DrawDebugLine(world, PointB, PointC, FColor::White);
	DrawDebugLine(world, PointC, PointA, FColor::White);
}

void UMapDebugVisualizer::DrawRivers(AActor* Actor, const FWorldSpaceMapData& MapData, UPolygonMap* MapGraph, const TArray<URiver*>& Rivers)
{
	UWorld* world = Actor->GetWorld();
	if (world == NULL)
	{
		UE_LOG(LogWorldGen, Error, TEXT("World was null!"));
		return;
	}

	FColor color = FColor(147, 198, 255);
	for (int r = 0; r < Rivers.Num(); r++)
	{
		URiver* river = Rivers[r];
		if (river->RiverCorners.Num() == 0)
		{
			continue;
		}
		FMapEdge lastEdge;
		FMapEdge currentEdge;
		FMapEdge nextEdge;
		for (int i = 0; i < river->RiverCorners.Num() - 2; i++)
		{
			FMapCorner v0 = river->RiverCorners[i];
			FMapCorner v1 = river->RiverCorners[i + 1];
			FMapCorner v2 = river->RiverCorners[i + 2];
			lastEdge = currentEdge;
			currentEdge = MapGraph->FindEdgeFromCorners(v0, v1);
			nextEdge = MapGraph->FindEdgeFromCorners(v1, v2);
			if (lastEdge.Index != -1 && nextEdge.Index != -1)
			{
				FVector worldLocation0 = MapGraph->ConvertGraphPointToWorldSpace(v0.CornerData);
				worldLocation0.Z = 0.0f;
				FVector worldLocation1 = MapGraph->ConvertGraphPointToWorldSpace(v1.CornerData);
				worldLocation1.Z = 0.0f;
				DrawDebugSphere(world, worldLocation0, MapData.PointSize, 4, color, true);
				DrawDebugSphere(world, worldLocation1, MapData.PointSize, 4, color, true);
				DrawBeizerCurve(Actor, MapData, lastEdge.Midpoint, v0.CornerData.Point, nextEdge.Midpoint, v1.CornerData.Point, color, MapGraph->GetGraphSize());
			}
		}
	}

	/*for (int i = 0; i < Corners.Num(); i++)
	{
		FMapData mapData = Corners[i].CornerData;

		FColor color = FColor(147, 198, 255);

		if (Corners[i].RiverSize > 0)
		{
			FVector worldLocation = UPolygonMap::ConvertGraphPointToWorldSpace(mapData, MapData, MapSize);
			DrawDebugSphere(world, worldLocation, MapData.PointSize, 4, color, true);
		}
	}

	for (int i = 0; i < Edges.Num(); i++)
	{
		if (Edges[i].RiverVolume > 0)
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

			FColor color = FColor(147, 198, 255);
			DrawDebugLine(world, worldVertex0, worldVertex1, color, true);
		}
	}*/

}

void UMapDebugVisualizer::DrawBeizerCurve(AActor* Actor, const FWorldSpaceMapData& WorldData, FVector2D v0, FVector2D control0, FVector2D v1, FVector2D control1, FColor color, int32 MapSize)
{
	UWorld* world = Actor->GetWorld();
	if (world == NULL)
	{
		UE_LOG(LogWorldGen, Error, TEXT("World was null!"));
		return;
	}
	FVector2D q0 = CalculateBezierPoint(0, v0, control0, control1, v1);

	float elevationOffset = WorldData.ElevationOffset;
	float xyScale = WorldData.XYScaleFactor / MapSize;
	float elevationScale = WorldData.ElevationScale;

	for (int i = 1; i <= 25; i++)
	{
		float t = i / 25.0f;
		FVector2D q1 = CalculateBezierPoint(t, v0, control0, control1, v1);

		FVector vertex0 = FVector::ZeroVector;
		vertex0.X = q0.X * MapSize * xyScale;
		vertex0.Y = q0.Y * MapSize * xyScale;
		FVector vertex1 = FVector::ZeroVector;
		vertex1.X = q1.X * MapSize * xyScale;
		vertex1.Y = q1.Y * MapSize * xyScale;

		DrawDebugLine(world, vertex0, vertex1, color, true);
		q0 = q1;
	}
}

FVector2D UMapDebugVisualizer::CalculateBezierPoint(float t, FVector2D p0, FVector2D p1, FVector2D p2, FVector2D p3)
{
	float u = 1 - t;
	float tt = t * t;
	float uu = u * u;
	float uuu = uu * u;
	float ttt = tt * t;

	FVector2D p = uuu * p0; //first term

	p += 3 * uu * t * p1; //second term
	p += 3 * u * tt * p2; //third term
	p += ttt * p3; //fourth term

	return p;
}