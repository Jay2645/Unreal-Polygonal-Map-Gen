// Unreal Engine 4 Delaunay implementation.
// Source based on https://github.com/delfrrr/delaunator-cpp
// Used under the MIT License.

#include "DelaunayHelper.h"
#include <delaunator.hpp>

float FDelaunayTriangle::GetArea() const
{
	const float ax = A.X;
	const float ay = A.Y;
	const float bx = B.X;
	const float by = B.Y;
	const float cx = C.X;
	const float cy = C.Y;
	return FMath::Abs((by - ay) * (cx - bx) - (bx - ax) * (cy - by)) / 2.0f;
}

float FDelaunayTriangle::GetCircumradius() const
{
	const float area = GetArea();
	if (area == 0.0f)
	{
		// collinear triangle
		return 0.0f;
	}
	return (ABLength * BCLength * CALength) / (4 * area);
}

FVector2D FDelaunayTriangle::GetCircumcenter() const
{
	float a = B.X - A.X;
	float b = B.Y - A.Y;
	float c = C.X - A.X;
	float d = C.Y - A.Y;
	float e = a * (A.X + B.X) + b * (A.Y + B.Y);
	float f = c * (A.X + C.X) + d * (A.Y + C.Y);
	float g = 2 * (a * (C.Y - B.Y) - b * (C.X - B.X));

	/* If the points of the triangle are collinear, then just find the
	* extremes and use the midpoint as the center of the circumcircle. */
	if (FMath::Abs(g) < 0.000001f)
	{
		float minx, miny, dx, dy;
		minx = FMath::Min(A.X, FMath::Min(B.X, C.X));
		miny = FMath::Min(A.Y, FMath::Min(B.Y, C.Y));
		dx = (FMath::Max(A.X, FMath::Max(B.X, C.X)) - minx) * 0.5f;
		dy = (FMath::Max(A.Y, FMath::Max(B.Y, C.Y)) - miny) * 0.5f;

		return FVector2D(minx + dx, miny + dy);
	}
	else
	{
		return FVector2D((d * e - b * f) / g, (a * f - c * e) / g);
	}
}

void FDelaunayMesh::CreatePoints(const TArray<FVector2D>& GivenPoints)
{
	// Convert to standard vector
	std::vector<double> coords = {};
	for (const FVector2D& point : GivenPoints)
	{
		// Push the coordinates into the vector 2 at a time
		coords.push_back((double)point.X);
		coords.push_back((double)point.Y);
	}

	// Triangulation happens here
	delaunator::Delaunator delaunay(coords);

	// Place all the data from the Delaunator into this struct for
	// easy access in Unreal

	// Coordinates
	Coordinates.Empty(delaunay.coords.size() / 2);
	for (int i = 0; i < delaunay.coords.size(); i += 2)
	{
		// The Delaunator stores everything in a vector of doubles
		// It doesn't store information about which doubles are paired to each other
		// However, we know that all even indices represent the X value, while odd indices
		// represent the Y value
		FVector2D coord = FVector2D((float)delaunay.coords[i], (float)delaunay.coords[i + 1]);
		Coordinates.Add(coord);
	}

	// Half-edges
	HalfEdges.Empty(delaunay.halfedges.size());
	for (int i = 0; i < delaunay.halfedges.size(); i++)
	{
		HalfEdges.Add((FSideIndex)delaunay.halfedges[i]);
	}

	// Triangles
	DelaunayTriangles.Empty(delaunay.triangles.size());
	for (int i = 0; i < delaunay.triangles.size(); i++)
	{
		DelaunayTriangles.Add((FPointIndex)delaunay.triangles[i]);
	}

	PointToEdge.Empty();
	for (FSideIndex e = 0; e < DelaunayTriangles.Num(); e++)
	{
		FPointIndex endpoint = DelaunayTriangles[UDelaunayHelper::NextHalfEdge(e)];
		if (!PointToEdge.Contains(endpoint) || !HalfEdges[e].IsValid())
		{
			PointToEdge.Add(endpoint, e);
		}
	}

	UE_LOG(LogDelaunator, Log, TEXT("Created Delaunay Triangulation with %d points, %d triangles, and %d half-edges."), Coordinates.Num(), DelaunayTriangles.Num() / 3, HalfEdges.Num());

	// Hull
	// Index of the first point in the hull
	HullStart = delaunay.hull_start;
	// All triangles making up our hull
	HullTriangles.Empty(delaunay.hull_tri.size());
	for (int i = 0; i < delaunay.hull_tri.size(); i++)
	{
		HullTriangles.Add(delaunay.hull_tri[i]);
	}
	// The previous triangle in the hull
	HullPrevious.Empty(delaunay.hull_prev.size());
	for (int i = 0; i < delaunay.hull_prev.size(); i++)
	{
		HullPrevious.Add(delaunay.hull_prev[i]);
	}
	// The next triangle in the hull
	HullNext.Empty(delaunay.hull_next.size());
	for (int i = 0; i < delaunay.hull_next.size(); i++)
	{
		HullNext.Add(delaunay.hull_next[i]);
	}
}

float FDelaunayMesh::GetHullArea(float& OutErrorAmount) const
{
	TArray<float> hullArea;
	FTriangleIndex current = HullStart;
	do
	{
		if (!Coordinates.IsValidIndex(current))
		{
			UE_LOG(LogDelaunator, Error, TEXT("Invalid Coordinate index. Index: %d, Array size: %d"), current, Coordinates.Num());
			return 0.0f;
		}
		if (!HullPrevious.IsValidIndex(current))
		{
			UE_LOG(LogDelaunator, Error, TEXT("Invalid HullPrevious index. Index: %d, Array size: %d"), current, HullPrevious.Num());
			return 0.0f;
		}
		if (!Coordinates.IsValidIndex(HullPrevious[current]))
		{
			UE_LOG(LogDelaunator, Error, TEXT("Invalid Coordinate index. Index: %d, Array size: %d"), HullPrevious[current], Coordinates.Num());
			return 0.0f;
		}
		float area = (Coordinates[current].X - Coordinates[HullPrevious[current]].X) * (Coordinates[current].Y + Coordinates[HullPrevious[current]].Y) / 2.0f;
		hullArea.Add(area);
		if (current == HullNext[current])
		{
			// Stuck in an infinite loop!
			UE_LOG(LogDelaunator, Error, TEXT("Next vertex pointed to itself! Vertex index: %d"), current);
			return 0.0f;
		}
		if (!HullNext.IsValidIndex(current))
		{
			UE_LOG(LogDelaunator, Error, TEXT("Invalid HullNext index. Index: %d, Array size: %d"), current, HullNext.Num());
			return 0.0f;
		}
		current = HullNext[current];
	} while (current != HullStart);
	return Sum(hullArea, OutErrorAmount);
}

float FDelaunayMesh::Sum(const TArray<float>& Area, float& OutErrorAmount) const
{
	if (Area.Num() == 0)
	{
		return 0.0f;
	}

	float sum = Area[0];
	float err = 0.0f;

	for (int i = 1; i < Area.Num(); i++)
	{
		const float k = Area[i];
		const float m = sum + k;
		err += FMath::Abs(sum) >= FMath::Abs(k) ? sum - m + k : k - m + sum;
		sum = m;
	}
	UE_LOG(LogDelaunator, Log, TEXT("Sum: %f, Err: %f"), sum, err);
	OutErrorAmount = FMath::Abs(err);
	return sum + err;
}

bool UDelaunayHelper::PointIsValid(const FPointIndex& Point)
{
	return Point.IsValid();
}

bool UDelaunayHelper::SideIsValid(const FSideIndex& Side)
{
	return Side.IsValid();
}

bool UDelaunayHelper::TriangleIsValid(const FTriangleIndex& Triangle)
{
	return Triangle.IsValid();
}

FDelaunayMesh UDelaunayHelper::CreateDelaunayTriangulation(const TArray<FVector2D>& Points)
{
	return FDelaunayMesh(Points);
}

FVector2D UDelaunayHelper::GetTriangleCircumcenter(const FDelaunayTriangle& Triangle)
{
	return Triangle.GetCircumcenter();
}

float UDelaunayHelper::GetTriangleArea(const FDelaunayTriangle& Triangle)
{
	return Triangle.GetArea();
}

FVector2D UDelaunayHelper::GetTrianglePoint(const FDelaunayMesh& Triangulation, FTriangleIndex TriangleIndex)
{
	if (!Triangulation.DelaunayTriangles.IsValidIndex(TriangleIndex))
	{
		UE_LOG(LogDelaunator, Error, TEXT("Invalid triangle ID: %d"), TriangleIndex);
		return FVector2D(-1, -1);
	}

	return Triangulation.Coordinates[UDelaunayHelper::PointsOfTriangle(Triangulation, TriangleIndex)[0]];
}

FDelaunayTriangle UDelaunayHelper::ConvertTriangleIDToTriangle(const FDelaunayMesh& Triangulation, FTriangleIndex TriangleIndex)
{
	FTriangleIndex offset = TriangleIndex % 3;

	FVector2D a, b, c;
	FPointIndex aIndex, bIndex, cIndex;

	aIndex = Triangulation.DelaunayTriangles[TriangleIndex - offset];
	bIndex = Triangulation.DelaunayTriangles[TriangleIndex - offset + 1];
	cIndex = Triangulation.DelaunayTriangles[TriangleIndex - offset + 2];

	a = Triangulation.Coordinates[aIndex];
	b = Triangulation.Coordinates[bIndex];
	c = Triangulation.Coordinates[cIndex];

	return FDelaunayTriangle(a, b, c, aIndex, bIndex, cIndex);
}

FSideIndex UDelaunayHelper::TriangleIndexToEdge(FTriangleIndex TriangleIndex)
{
	return FSideIndex(TriangleIndex.Value);
}

TArray<FSideIndex> UDelaunayHelper::EdgesOfTriangle(FTriangleIndex TriangleIndex)
{
	return TArray<FSideIndex> {TriangleIndexToEdge(TriangleIndex), TriangleIndexToEdge(TriangleIndex + 1), TriangleIndexToEdge(TriangleIndex.Value + 2) };
}

TArray<FPointIndex> UDelaunayHelper::PointsOfTriangle(const FDelaunayMesh& Triangulation, FTriangleIndex TriangleIndex)
{
	TArray<FSideIndex> edges = UDelaunayHelper::EdgesOfTriangle(TriangleIndex);
	TArray<FPointIndex> points;
	for (int i = 0; i < edges.Num(); i++)
	{
		points.Add(Triangulation.DelaunayTriangles[edges[i]]);
	}
	return points;
}

FVector2D UDelaunayHelper::GetPointFromHalfEdge(const FDelaunayMesh& Triangulation, FSideIndex HalfEdge)
{
	FPointIndex index = GetPointIndexFromHalfEdge(Triangulation, HalfEdge);
	if (!Triangulation.Coordinates.IsValidIndex(index))
	{
		return FVector2D(-1, -1);
	}
	return Triangulation.Coordinates[index];
}

FPointIndex UDelaunayHelper::GetPointIndexFromHalfEdge(const FDelaunayMesh& Triangulation, FSideIndex HalfEdge)
{
	FTriangleIndex triangleIndex = GetTriangleIndexFromHalfEdge(HalfEdge);
	if (!Triangulation.DelaunayTriangles.IsValidIndex(triangleIndex))
	{
		return FPointIndex();
	}

	uint8 pointNum = HalfEdge % 3; // 0, 1, or 2
	TArray<FPointIndex> trianglePoints = UDelaunayHelper::PointsOfTriangle(Triangulation, triangleIndex);
	return trianglePoints[pointNum];
}

FDelaunayTriangle UDelaunayHelper::GetTriangleFromHalfEdge(const FDelaunayMesh& Triangulation, FSideIndex HalfEdge)
{
	FTriangleIndex triangleIndex = GetTriangleIndexFromHalfEdge(HalfEdge);
	if (!Triangulation.DelaunayTriangles.IsValidIndex(triangleIndex))
	{
		return FDelaunayTriangle();
	}
	return UDelaunayHelper::ConvertTriangleIDToTriangle(Triangulation, triangleIndex);
}

FTriangleIndex UDelaunayHelper::GetTriangleIndexFromHalfEdge(FSideIndex HalfEdge)
{
	if (!HalfEdge.IsValid())
	{
		return FTriangleIndex();
	}
	uint8 pointNum = HalfEdge % 3;
	FTriangleIndex triangleID = (HalfEdge - pointNum);
	return triangleID;
}

FSideIndex UDelaunayHelper::NextHalfEdge(FSideIndex HalfEdge)
{
	return (HalfEdge % 3 == 2) ? HalfEdge - 2 : HalfEdge + 1;
}

FSideIndex UDelaunayHelper::PreviousHalfEdge(FSideIndex HalfEdge)
{
	return (HalfEdge % 3 == 0) ? HalfEdge + 2 : HalfEdge - 1;
}

FSideIndex UDelaunayHelper::OppositeHalfEdge(const FDelaunayMesh& Triangulation, FSideIndex HalfEdge)
{
	if (Triangulation.HalfEdges.IsValidIndex(HalfEdge))
	{
		return Triangulation.HalfEdges[HalfEdge];
	}
	else
	{
		return FSideIndex();
	}
}

TArray<FSideIndex> UDelaunayHelper::EdgesFromIncomingEdge(const FDelaunayMesh& Triangulation, FSideIndex Start)
{
	TArray<FSideIndex> result;
	FSideIndex incoming = Start;
	if (!incoming.IsValid())
	{
		// Can't process
		return result;
	}
	do 
	{
		result.Add(incoming);
		const FSideIndex outgoing = UDelaunayHelper::NextHalfEdge(incoming);
		incoming = Triangulation.HalfEdges[outgoing];
	} while (incoming.IsValid() && incoming != Start);
	return result;
}