// Original Work Copyright (c) 2010 Amit J Patel, Modified Work Copyright (c) 2016 Jay M Stevens

#include "PolygonalMapGeneratorPrivatePCH.h"
#include "Maps/PolygonMap.h"
#include "MapDebugVisualizer.h"
#include "Maps/MapDataHelper.h"
#include "Classes/Diagrams/Voronoi.h"

DEFINE_LOG_CATEGORY(LogWorldGen);

void UPolygonMap::CreatePoints(UPointGenerator* pointSelector, const int32& numberOfPoints)
{
	if (pointSelector == NULL)
	{
		return;
	}
	PointSelector = pointSelector;
	Points = PointSelector->GeneratePoints(numberOfPoints);
}

void UPolygonMap::BuildGraph(const int32& mapSize, const FWorldSpaceMapData& data)
{
	MapSize = mapSize;
	MapData = data;
	Voronoi voronoi(Points);
	for (int i = 0; i < voronoi.sites.Num(); i++)
	{
		VSite site = voronoi.sites[i];

		// Create centers, corners, and edges
		for (auto siteEdge : site.edges)
		{
			// Voronoi Edge (corner to corner)
			FVector4 vEdge = siteEdge.vEdge;
			// Delaunay Edge (center to center)
			FVector4 dEdge = siteEdge.dEdge;

			// Corners come from edge vertices
			FMapCorner cornerOne = MakeCorner(FVector2D(vEdge.X, vEdge.Y));
			FMapCorner cornerTwo = MakeCorner(FVector2D(vEdge.Z, vEdge.W));
			FMapCenter centerOne = MakeCenter(FVector2D(dEdge.X, dEdge.Y));
			FMapCenter centerTwo = MakeCenter(FVector2D(dEdge.Z, dEdge.W));

			int32 edgeIndex = -1;
			for (int32 j = 0; j < Edges.Num(); j++)
			{
				int32 d0 = Edges[j].DelaunayEdge0;
				int32 d1 = Edges[j].DelaunayEdge1;
				int32 v0 = Edges[j].VoronoiEdge0;
				int32 v1 = Edges[j].VoronoiEdge1;

				if (((d0 == centerOne.Index && d1 == centerTwo.Index) || (d0 == centerTwo.Index && d1 == centerOne.Index)) &&
					((v0 == cornerOne.Index && v1 == cornerTwo.Index) || (v0 == cornerTwo.Index && v1 == cornerOne.Index)))
				{
					edgeIndex = j;
					break;
				}
			}


			if (edgeIndex < 0)
			{
				// Create edge object
				FMapEdge edge;
				// Edges point to corners
				edge.VoronoiEdge0 = cornerOne.Index;
				edge.VoronoiEdge1 = cornerTwo.Index;
				// Edges point to centers
				edge.DelaunayEdge0 = centerOne.Index;
				edge.DelaunayEdge1 = centerTwo.Index;

				edge.Index = Edges.Num();
				edgeIndex = edge.Index;
				if (cornerOne.Index >= 0 && cornerTwo.Index >= 0)
				{
					edge.Midpoint = FVector2D(FMath::Lerp(vEdge.X, vEdge.Z, 0.5f), FMath::Lerp(vEdge.Y, vEdge.W, 0.5f));
				}
				Edges.Add(edge);
			}

			// Corners point to edges
			if (cornerOne.Index >= 0) { cornerOne.Protrudes.AddUnique(edgeIndex); }
			if (cornerTwo.Index >= 0) { cornerTwo.Protrudes.AddUnique(edgeIndex); }
			// Centers point to edges
			if (centerOne.Index >= 0) { centerOne.Borders.AddUnique(edgeIndex); }
			if (centerTwo.Index >= 0) { centerTwo.Borders.AddUnique(edgeIndex); }

			// Centers point to centers
			if (centerOne.Index >= 0 && centerTwo.Index >= 0)
			{
				centerOne.Neighbors.AddUnique(centerTwo.Index);
				centerTwo.Neighbors.AddUnique(centerOne.Index);
			}

			// Corners point to corners
			if (cornerOne.Index >= 0 && cornerTwo.Index >= 0)
			{
				cornerOne.Adjacent.AddUnique(cornerTwo.Index);
				cornerTwo.Adjacent.AddUnique(cornerOne.Index);
			}

			// Centers point to corners
			if (centerOne.Index >= 0)
			{
				if (cornerOne.Index >= 0) { centerOne.Corners.AddUnique(cornerOne.Index); }
				if (cornerTwo.Index >= 0) { centerOne.Corners.AddUnique(cornerTwo.Index); }
			}
			if (centerTwo.Index >= 0)
			{
				if (cornerOne.Index >= 0) { centerTwo.Corners.AddUnique(cornerOne.Index); }
				if (cornerTwo.Index >= 0) { centerTwo.Corners.AddUnique(cornerTwo.Index); }
			}

			// Corners point to centers
			if (cornerOne.Index >= 0)
			{
				if (centerOne.Index >= 0) { cornerOne.Touches.AddUnique(centerOne.Index); }
				if (centerOne.Index >= 0) { cornerOne.Touches.AddUnique(centerTwo.Index); }
			}
			if (cornerTwo.Index >= 0)
			{
				if (centerTwo.Index >= 0) { cornerTwo.Touches.AddUnique(centerOne.Index); }
				if (centerTwo.Index >= 0) { cornerTwo.Touches.AddUnique(centerTwo.Index); }
			}

			// Update Array (it won't update automatically)
			UpdateCenter(centerOne);
			UpdateCenter(centerTwo);
			UpdateCorner(cornerOne);
			UpdateCorner(cornerTwo);
		}
	}
	UE_LOG(LogWorldGen, Log, TEXT("Created a total of %d Centers, %d Corners, and %d Edges."), Centers.Num(), Corners.Num(), Edges.Num());
}

FMapCenter UPolygonMap::MakeCenter(const FVector2D& point)
{
	if (point.X > MapSize || point.Y > MapSize)
	{
		return GetCenter(-1);
	}
	if (CenterLookup.Contains(point))
	{
		return GetCenter(CenterLookup[point]);
	}

	FMapCenter center;
	center.CenterData.Point = point;
	center.CenterData = UMapDataHelper::RemoveBorder(center.CenterData);
	center.CenterData = UMapDataHelper::RemoveOcean(center.CenterData);
	center.Index = Centers.Num();

	Centers.Add(center);
	CenterLookup.Add(point, center.Index);

	return GetCenter(center.Index);
}

FMapCenter UPolygonMap::GetCenter(const int32& index) const
{
	if (index < 0)
	{
		return FMapCenter();
	}
	return Centers[index];
}

FMapCorner UPolygonMap::MakeCorner(const FVector2D& point)
{
	if (point.X > MapSize || point.Y > MapSize)
	{
		return GetCorner(-1);
	}
	if (CornerLookup.Contains(point))
	{
		return GetCorner(CornerLookup[point]);
	}

	FMapCorner corner;
	corner.CornerData.Point = point;
	corner.Index = Corners.Num();
	corner.CornerData = UMapDataHelper::RemoveOcean(corner.CornerData);
	if (PointSelector->PointIsOnBorder(corner.CornerData.Point))
	{
		corner.CornerData = UMapDataHelper::SetBorder(corner.CornerData);
	}
	else
	{
		corner.CornerData = UMapDataHelper::RemoveBorder(corner.CornerData);
	}
	
	Corners.Add(corner);
	CornerLookup.Add(point, corner.Index);

	return GetCorner(corner.Index);
}

FMapCorner UPolygonMap::GetCorner(const int32& index) const
{
	if (index < 0)
	{
		return FMapCorner();
	}
	return Corners[index];
}

FMapEdge UPolygonMap::GetEdge(const int32& index) const
{
	if (index < 0)
	{
		return FMapEdge();
	}
	return Edges[index];
}
FMapEdge UPolygonMap::FindEdgeFromCenters(const FMapCenter& v0, const FMapCenter& v1) const
{
	if (v0.Index < 0 || v0.Index >= Edges.Num() || v1.Index <0 || v1.Index >= Edges.Num())
	{
		return GetEdge(-1);
	}
	for (int i = 0; i < Edges.Num(); i++)
	{
		if ((Edges[i].DelaunayEdge0 == v0.Index && Edges[i].DelaunayEdge1 == v1.Index) || (Edges[i].DelaunayEdge0 == v1.Index && Edges[i].DelaunayEdge1 == v0.Index))
		{
			return GetEdge(i);
		}
	}
	return GetEdge(-1);
}
FMapEdge UPolygonMap::FindEdgeFromCorners(const FMapCorner& v0, const FMapCorner& v1) const
{
	if (v0.Index < 0 || v0.Index >= Edges.Num() || v1.Index <0 || v1.Index >= Edges.Num())
	{
		return GetEdge(-1);
	}
	for (int i = 0; i < Edges.Num(); i++)
	{
		if ((Edges[i].VoronoiEdge0 == v0.Index && Edges[i].VoronoiEdge1 == v1.Index) || (Edges[i].VoronoiEdge0 == v1.Index && Edges[i].VoronoiEdge1 == v0.Index))
		{
			return GetEdge(i);
		}
	}
	return GetEdge(-1);
}

FMapCenter UPolygonMap::FindCenterFromCorners(FMapCorner CornerA, FMapCorner CornerB) const
{
	for (int i = 0; i < CornerA.Touches.Num(); i++)
	{
		for (int j = 0; j < CornerB.Touches.Num(); j++)
		{
			if (CornerA.Touches[i] == CornerB.Touches[j])
			{
				return GetCenter(CornerA.Touches[i]);
			}
		}
	}
	return GetCenter(-1);
}

int32 UPolygonMap::GetCenterNum() const
{
	return Centers.Num();
}
int32 UPolygonMap::GetCornerNum() const
{
	return Corners.Num();
}
int32 UPolygonMap::GetEdgeNum() const
{
	return Edges.Num();
}

int32 UPolygonMap::GetGraphSize() const
{
	return MapSize;
}

void UPolygonMap::UpdateCenter(const FMapCenter& center)
{
	if (center.Index < 0 || center.Index >= Centers.Num())
	{
		return;
	}
	Centers[center.Index] = center;
}
void UPolygonMap::UpdateCorner(const FMapCorner& corner)
{
	if (corner.Index < 0 || corner.Index >= Corners.Num())
	{
		return;
	}
	Corners[corner.Index] = corner;
}
void UPolygonMap::UpdateEdge(const FMapEdge& edge)
{
	if (edge.Index < 0 || edge.Index >= Edges.Num())
	{
		return;
	}
	Edges[edge.Index] = edge;
}

void UPolygonMap::ImproveCorners()
{
	TArray<FVector2D> newCorners;
	newCorners.SetNumZeroed(Corners.Num());

	// First we compute the average of the centers next to each corner.
	for (int i = 0; i < Corners.Num(); i++)
	{
		if (UMapDataHelper::IsBorder(Corners[i].CornerData))
		{
			newCorners[i] = Corners[i].CornerData.Point;
		}
		else
		{
			FVector2D point = FVector2D::ZeroVector;
			int32 touchesLength = Corners[i].Touches.Num();
			for (int j = 0; j < touchesLength; j++)
			{
				int32 centerIndex = Corners[i].Touches[j];
				point += Centers[centerIndex].CenterData.Point;
			}
			point /= touchesLength;
			newCorners[i] = point;
		}
	}

	// Move the corners to the new locations.
	for (int i = 0; i < Corners.Num(); i++)
	{
		Corners[i].CornerData.Point = newCorners[i];
	}

	// The edge midpoints were computed for the old corners and need
	// to be recomputed.
	for (int i = 0; i < Edges.Num(); i++)
	{
		if (Edges[i].VoronoiEdge0 >= 0 && Edges[i].VoronoiEdge1 >= 0)
		{
			FVector2D edgeVertexOne = Corners[Edges[i].VoronoiEdge0].CornerData.Point;
			FVector2D edgeVertexTwo = Corners[Edges[i].VoronoiEdge1].CornerData.Point;
			Edges[i].Midpoint = FVector2D(FMath::Lerp(edgeVertexOne.X, edgeVertexTwo.X, 0.5f), FMath::Lerp(edgeVertexOne.Y, edgeVertexTwo.Y, 0.5f));
		}
	}
}

TArray<int32> UPolygonMap::FindLandCorners() const
{
	TArray<int32> landCorners;
	for (int i = 0; i < Corners.Num(); i++)
	{
		FMapCorner corner = Corners[i];
		if (UMapDataHelper::IsOcean(corner.CornerData) || UMapDataHelper::IsCoast(corner.CornerData))
		{
			continue;
		}
		landCorners.Emplace(i);
	}
	return landCorners;
}

TArray<FMapData>& UPolygonMap::GetAllMapData()
{
	return CachedMapData;
}

void UPolygonMap::CompileMapData()
{
	CachedMapData.Empty();
	for (int i = 0; i < GetCenterNum(); i++)
	{
		CachedMapData.Add(GetCenter(i).CenterData);
	}
	for (int i = 0; i < GetCornerNum(); i++)
	{
		CachedMapData.Add(GetCorner(i).CornerData);
	}
}

FVector UPolygonMap::ConvertGraphPointToWorldSpace(const FMapData& MapData, const FWorldSpaceMapData& WorldData, int32 MapSize)
{
	float elevationOffset = WorldData.ElevationOffset;
	float xyScale = WorldData.XYScaleFactor / MapSize;
	float elevationScale = WorldData.ElevationScale;

	FVector worldLocation = FVector::ZeroVector;
	worldLocation.X = MapData.Point.X * MapSize * xyScale;
	worldLocation.Y = MapData.Point.Y * MapSize * xyScale;
	worldLocation.Z = (MapData.Elevation * elevationScale) + elevationOffset;

	return worldLocation;
}