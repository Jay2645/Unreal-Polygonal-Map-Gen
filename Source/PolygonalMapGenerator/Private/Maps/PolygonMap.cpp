// Original Work Copyright (c) 2010 Amit J Patel, Modified Work Copyright (c) 2016 Jay M Stevens

#include "PolygonalMapGeneratorPrivatePCH.h"
#include "Maps/PointGenerators/PointGenerator.h"
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
	MinPointLocation = PointSelector->MinPoint();
	MaxPointLocation = PointSelector->MaxPoint();
}

void UPolygonMap::BuildGraph(const int32& mapSize, const FWorldSpaceMapData& data)
{
	MapSize = mapSize;
	WorldData = data;
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

			FMapEdge edge;
			edge.DelaunayEdge0 = centerOne.Index;
			edge.DelaunayEdge1 = centerTwo.Index;
			edge.VoronoiEdge0 = cornerOne.Index;
			edge.VoronoiEdge1 = cornerTwo.Index;
			if(cornerOne.Index >=0 && cornerTwo.Index >= 0)
			{
				edge.Midpoint = FVector2D(FMath::Lerp(vEdge.X, vEdge.Z, 0.5f), FMath::Lerp(vEdge.Y, vEdge.W, 0.5f));
			}
			edge.Index = Edges.Num();
			Edges.Add(edge);

			/*int32 edgeIndex = -1;
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
			}*/

			// Corners point to edges
			if (cornerOne.Index >= 0) { cornerOne.Protrudes.AddUnique(edge.Index); }
			if (cornerTwo.Index >= 0) { cornerTwo.Protrudes.AddUnique(edge.Index); }
			// Centers point to edges
			if (centerOne.Index >= 0) { centerOne.Borders.AddUnique(edge.Index); }
			if (centerTwo.Index >= 0) { centerTwo.Borders.AddUnique(edge.Index); }

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
	checkf(!(Corners[corner.Index].CornerData.Biome.IsValid() && !corner.CornerData.Biome.IsValid()), TEXT("Attempted to set corner %d from biome %s to an invalid biome!"), corner.Index, *Corners[corner.Index].CornerData.Biome.ToString())
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

TArray<FMapCorner> UPolygonMap::GetCopyOfMapCornerArray()
{
	return Corners;
}

TArray<FMapEdge> UPolygonMap::GetCopyOfMapEdgeArray()
{
	return Edges;
}

TArray<FMapCenter> UPolygonMap::GetCopyOfMapCenterArray()
{
	return Centers;
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

FVector2D UPolygonMap::ConvertWorldPointToGraphSpace(const FVector WorldPoint)
{

	FVector2D graphLocation = FVector2D::ZeroVector;
	graphLocation.X = WorldPoint.X / WorldData.XYScaleFactor;
	graphLocation.Y = WorldPoint.Y / WorldData.XYScaleFactor;

	return graphLocation;
}

FVector UPolygonMap::ConvertGraphPointToWorldSpace(const FMapData& MapPointData)
{
	FVector worldLocation = FVector::ZeroVector;
	worldLocation.X = MapPointData.Point.X *  WorldData.XYScaleFactor;
	worldLocation.Y = MapPointData.Point.Y *  WorldData.XYScaleFactor;
	worldLocation.Z = (MapPointData.Elevation * WorldData.ElevationScale) + WorldData.ElevationOffset;

	return worldLocation;
}

FMapCenter UPolygonMap::FindMapCenterForCoordinate(const FVector2D& Point)
{
	if (Point.X > MaxPointLocation || Point.Y > MaxPointLocation || Point.X < MinPointLocation || Point.Y < MinPointLocation)
	{
		// Point out of bounds
		return FMapCenter();
	}

	FVector2D intMapCoordinates = Point;
	intMapCoordinates.X = FMath::RoundToInt(Point.X);
	intMapCoordinates.Y = FMath::RoundToInt(Point.Y);
	if (CenterLookup.Contains(intMapCoordinates))
	{
		return GetCenter(CenterLookup[intMapCoordinates]);
	}

	FMapCenter center = FMapCenter();
	for (int i = 0; i < Centers.Num(); i++)
	{
		if (CenterContainsPoint(Point, Centers[i]))
		{
			center = Centers[i];
			break;
		}
	}

	CenterLookup.Add(intMapCoordinates, center.Index);
	return center;
}

FMapCorner UPolygonMap::FindMapCornerForCoordinate(const FVector2D& Point)
{
	if (Point.X > MaxPointLocation || Point.Y > MaxPointLocation || Point.X < MinPointLocation || Point.Y < MinPointLocation)
	{
		// Point out of bounds; don't even bother putting it in the cache
		return FMapCorner();
	}

	FVector2D intMapCoordinates = Point;
	intMapCoordinates.X = FMath::RoundToInt(Point.X);
	intMapCoordinates.Y = FMath::RoundToInt(Point.Y);
	if (CornerLookup.Contains(intMapCoordinates))
	{
		return GetCorner(CornerLookup[intMapCoordinates]);
	}

	FMapCorner corner = FMapCorner();
	for (int i = 0; i < Corners.Num(); i++)
	{
		if (CornerContainsPoint(Point, Corners[i]).bTriangleIsValid)
		{
			corner = Corners[i];
			if (corner.Touches.Num() == 0)
			{
				corner = FMapCorner();
			}
			else if (corner.Touches.Num() != 3)
			{
				UE_LOG(LogTemp, Error, TEXT("Polygon at coordinates (%f, %f) was not a triangle (%d vertices)."), intMapCoordinates.X, intMapCoordinates.Y, corner.Touches.Num());
				corner = FMapCorner();
			}
			break;
		}
	}
	CornerLookup.Add(intMapCoordinates, corner.Index);
	return corner;
}

bool UPolygonMap::CenterContainsPoint(const FVector2D& Point, const FMapCenter& Center) const
{
	float minX = Center.CenterData.Point.X;
	float maxX = Center.CenterData.Point.X;
	float minY = Center.CenterData.Point.Y;
	float maxY = Center.CenterData.Point.Y;
	for (int i = 0; i < Center.Corners.Num(); i++)
	{
		// Iterate over the borders, creating a bounding box for the polygon
		FMapCorner corner = GetCorner(Center.Corners[i]);
		if(corner.Index < 0)
		{
			// Invalid corner
			continue;
		}
		maxX = FMath::Max(maxX, corner.CornerData.Point.X);
		minX = FMath::Min(minX, corner.CornerData.Point.X);
		maxY = FMath::Max(maxY, corner.CornerData.Point.Y);
		minY = FMath::Min(minY, corner.CornerData.Point.Y);
	}

	// Check to see if bounding box contains point
	if(Point.X < minX || Point.X > maxX || Point.Y < minY || Point.Y > maxY)
	{
		// Not even in the polygon's bounding box
		return false;
	}

	// Set the padding to be 1% of the polygon size
	float epsilon = ((maxX - minX) / 100.0f);
	FVector2D pointTarget = FVector2D(minX - epsilon, Point.Y);

	int32 intersections = 0;
	for (int i = 0; i < Center.Borders.Num(); i++)
	{
		FMapEdge border = GetEdge(Center.Borders[i]);
		if (border.Index < 0)
		{
			// Invalid index
			UE_LOG(LogWorldGen, Warning, TEXT("Invalid border! Center number %d, Edge index was %d."), Center.Index, Center.Borders[i]);
			continue;
		}
		if (SegementsIntersect(border, Point, pointTarget))
		{
			intersections++;
			//UE_LOG(LogWorldGen, Log, TEXT("Checking if point (%f, %f)-(%f, %f) and point (%f, %f)-(%f, %f) intersect."), p1.X, p1.Y, q1.X, q1.Y, p2.X, p2.Y, q2.X, q2.Y);
		}
	}
	
	return (intersections & 1) == 1; // True if point is odd (inside of polygon)
}

FPointInterpolationData UPolygonMap::CornerContainsPoint(const FVector2D& Point, const FMapCorner& Corner) const
{
	FPointInterpolationData output = FPointInterpolationData();
	if (Corner.Touches.Num() != 3)
	{
		return output;
	}

	FMapData center1 = Centers[Corner.Touches[0]].CenterData;
	FMapData center2 = Centers[Corner.Touches[1]].CenterData;
	FMapData center3 = Centers[Corner.Touches[2]].CenterData;
	FVector2D p1 = center1.Point;
	FVector2D p2 = center2.Point;
	FVector2D p3 = center3.Point;

	// Check bounding box
	//float maxX = FMath::Max3(p1.X, p2.X, p3.X);
	//if (Point.X > maxX)
	if (Point.X > p1.X && Point.X > p2.X && Point.X > p3.X)
	{
		// To the right of maximum triangle bounds
		return output;
	}
	//float maxY = FMath::Max3(p1.Y, p2.Y, p3.Y);
	//if (Point.Y > maxY)
	if (Point.Y > p1.Y && Point.Y > p2.Y && Point.Y > p3.Y)
	{
		// Above maximum triangle bounds
		return output;
	}
	//float minX = FMath::Min3(p1.X, p2.X, p3.X);
	//if (Point.X < minX)
	if (Point.X < p1.X && Point.X < p2.X && Point.X < p3.X)
	{
		// To the left of maximum triangle bounds
		return output;
	}
	//float minY = FMath::Min3(p1.Y, p2.Y, p3.Y);
	//if (Point.Y < minY)
	if (Point.Y < p1.Y && Point.Y < p2.Y && Point.Y < p3.Y)
	{
		// Underneath maximum triangle bounds
		return output;
	}

	// Point is inside of bounding box

	// Calculate determinant
	float det = (p2.Y - p3.Y) * (p1.X - p3.X) + (p3.X - p2.X) * (p1.Y - p3.Y);
	if (det == 0.0f)
	{
		// Shouldn't happen, but just in case
		return output;
	}

	// https://stackoverflow.com/questions/36090269/finding-height-of-point-on-height-map-triangles
	// p lies in T if and only if 0 <= a <= 1 and 0 <= b <= 1 and 0 <= c <= 1
	float lambda1 = ((p2.Y - p3.Y) * (Point.X - p3.X) + (p3.X - p2.X) * (Point.Y - p3.Y)) / det;
	if (0 > lambda1 || lambda1 > 1)
	{
		return output;
	}
	float lambda2 = ((p3.Y - p1.Y) * (Point.X - p3.X) + (p1.X - p3.X) * (Point.Y - p3.Y)) / det;
	if (0 > lambda2 || lambda2 > 1)
	{
		return output;
	}
	float lambda3 = 1 - lambda1 - lambda2;
	if (0 > lambda3 || lambda3 > 1)
	{
		return output;
	}

	output.bTriangleIsValid = true;
	output.SourceTriangle = Corner;
	output.InterpolatedElevation = lambda1 * center1.Elevation + lambda2 * center2.Elevation + lambda3 * center3.Elevation;
	output.InterpolatedMoisture = lambda1 * center1.Moisture + lambda2 * center2.Moisture + lambda3 * center3.Moisture;
	
	return output;
}

FPointInterpolationData UPolygonMap::FindInterpolatedDataForPoint(const FVector2D& Point)
{
	if (Point.X > MaxPointLocation || Point.Y > MaxPointLocation || Point.X < MinPointLocation || Point.Y < MinPointLocation)
	{
		// Point out of bounds; don't even bother
		return FPointInterpolationData();
	}

	FVector2D intMapCoordinates = Point;
	intMapCoordinates.X = FMath::RoundToInt(Point.X);
	intMapCoordinates.Y = FMath::RoundToInt(Point.Y);
	if (CornerLookup.Contains(intMapCoordinates))
	{
		UE_LOG(LogWorldGen, Log, TEXT("Cache hit! (%f, %f)"), Point.X, Point.Y);
		return CornerContainsPoint(Point, GetCorner(CornerLookup[intMapCoordinates]));
	}

	FPointInterpolationData data = FPointInterpolationData();
	TArray<int> tried = TArray<int>();
	if (LastFoundCorner.Index >= 0)
	{
		// Optimization: Check to see if we share a triangle with the last point we found.
		// If we're running on a single thread, this is very helpful.
		// It doesn't work so well if we're multithreaded.
		data = CornerContainsPoint(Point, LastFoundCorner);
		if (data.bTriangleIsValid)
		{
			CornerLookup.Add(intMapCoordinates, LastFoundCorner.Index);
		}
		else
		{
			tried.Add(LastFoundCorner.Index);
			// Check the neighboring triangles
			for (int i = 0; i < LastFoundCorner.Adjacent.Num(); i++)
			{
				FMapCorner adjacent = Corners[LastFoundCorner.Adjacent[i]];
				data = CornerContainsPoint(Point, adjacent);
				if (data.bTriangleIsValid)
				{
					LastFoundCorner = adjacent;
					CornerLookup.Add(intMapCoordinates, LastFoundCorner.Index);
					break;
				}
				else
				{
					tried.Add(LastFoundCorner.Adjacent[i]);
				}
			}
		}
	}

	if (!data.bTriangleIsValid)
	{
		// Wasn't in the previous triangle we tried; search the whole array
		// This is VERY SLOW. It's the biggest bottleneck in the whole process.
		// Try to avoid getting here as much as physically possible.
		for (int i = 0; i < Corners.Num(); i++)
		{
			if (tried.Contains(i))
			{
				continue;
			}
			data = CornerContainsPoint(Point, Corners[i]);
			if (data.bTriangleIsValid)
			{
				LastFoundCorner = Corners[i];
				CornerLookup.Add(intMapCoordinates, i);
				break;
			}
		}
	}

	if (!data.bTriangleIsValid)
	{
		// Couldn't find this corner
		CornerLookup.Add(intMapCoordinates, -1);
	}
	return data;
}

// The main function that returns true if line segment 'p1q1'
// and 'p2q2' intersect.
bool UPolygonMap::SegementsIntersect(const FMapEdge& Edge, const FVector2D& StartPoint, const FVector2D& EndPoint) const
{
	FMapCorner corner1 = GetCorner(Edge.VoronoiEdge0);
	FMapCorner corner2 = GetCorner(Edge.VoronoiEdge1);
	if (corner1.Index < 0 || corner2.Index < 0)
	{
		// Invalid corner
		UE_LOG(LogWorldGen, Warning, TEXT("Invalid corner! Edge number %d, Corner 1 index was %d, Corner 2 index was %d."), Edge.Index, Edge.VoronoiEdge0, Edge.VoronoiEdge1);
		return false;
	}
	const float v1x1 = corner1.CornerData.Point.X;
	const float v1y1 = corner1.CornerData.Point.Y;
	const float v1x2 = corner2.CornerData.Point.X;
	const float v1y2 = corner2.CornerData.Point.Y;
	const float v2x1 = StartPoint.X;
	const float v2y1 = StartPoint.Y;
	const float v2x2 = EndPoint.X;
	const float v2y2 = EndPoint.Y;

	float d1, d2;
	float a1, a2, b1, b2, c1, c2;

	// Convert the first vector to a line of infinite length
	// We want this line in a linear equation
	a1 = v1y2 - v1y1;
	b1 = v1x1 - v1x2;
	c1 = (v1x2 * v1y1) - (v1x1 * v1y2);

	// Every point (x, y) that solves the equation above is on the line.
	// Every point that does not solve it is not.
	// The equation will have a positive result if it is on one side of the line
	// and a negative result if it is on the other.
	// We insert our start and end points into the equation above.
	d1 = (a1 * v2x1) + (b1 * v2y1) + c1;
	d2 = (a1 * v2x2) + (b1 * v2y2) + c1;

	// If d1 and d2 have the same sign, they are both on the same side of our Edge.
	// In this case, no intersection is possible.
	// 0 is a special case, which is why < and > are used instead of <= and >=.
	if((d1 > 0 && d2 > 0) || (d1 < 0 && d2 < 0))
	{
		return false;
	}

	// If we have gotten to this point, the infinite lines intersect somewhere (i.e., they are not parallel).
	// However, we are looking for an intersection in a specific interval (the edge).
	// The edge is a subset of the infinite line, and it is possible that the intersection happened before or after that interval
	// In order to know for sure, we have to do the same test the other way around.
	// Starting with converting the second segment into an infinite line:
	a2 = v2y2 - v2y1;
	b2 = v2x1 - v2x2;
	c2 = (v2x2 * v2y1) - (v2x1 * v2y2);

	// Calculate d1 and d2 again, using the other segment
	d1 = (a2 * v1x1) + (b2 * v1y1) + c2;
	d2 = (a2 * v1x2) + (b2 * v1y2) + c2;

	// Again, if both have the same sign, no intersection is possible
	if ((d1 > 0 && d2 > 0) || (d1 < 0 && d2 < 0))
	{
		return false;
	}

	// Now we have 2 options.
	// Either they intersect at exactly one point, or they are colinear and intersect at any number of points.
	// We treat colinear lines as failing this test.
	if((a1 * b2) - (a2 * b1) < 0.001f)
	{
		// Points are colinear
		return false;
	}
	else
	{
		// Points intersect somewhere
		return true;
	}
}
