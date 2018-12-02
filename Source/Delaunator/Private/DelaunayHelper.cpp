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
	return FMath::Abs((by - ay) * (cx - bx) - (bx - ax) * (cy - by));
}

FVector2D FDelaunayTriangle::GetCircumcenter() const
{
	const float ad = A.X * A.X + A.Y * A.Y;
	const float bd = B.X * B.X + B.Y * B.Y;
	const float cd = C.X * C.X + C.Y * C.Y;
	const float d = 2.0f * (A.X * (A.Y - C.Y) + B.X * (C.Y - A.Y) + C.X * (A.Y - B.Y));
	return FVector2D(1.0f / d * (ad * (B.Y - C.Y) + bd * (C.Y - A.Y) + cd * (A.Y - B.Y)),
			1.0f / d * (ad * (C.X - B.X) + bd * (A.X - C.X) + cd * (B.X - A.X)));
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
		auto halfEdge = delaunay.halfedges[i];
		if (halfEdge == delaunator::INVALID_INDEX)
		{
			HalfEdges.Add(-1);
		}
		else
		{
			// Technically, each half-edge is represented by a size_t value, not an int.
			// However, Unreal doesn't accept size_t in Blueprint arrays, nor does it
			// accept uint64 values. The highest we can go while still supporting Blueprint
			// is int32. This cast is technically unsafe, but it's unlikely we'll have issues.
			HalfEdges.Add((int32)halfEdge);
		}
	}

	// Triangles
	DelaunayTriangles.Empty(delaunay.triangles.size());
	for (int i = 0; i < delaunay.triangles.size(); i++)
	{
		DelaunayTriangles.Add((int32)delaunay.triangles[i]);
	}

	/*Triangles.Empty(DelaunayTriangles.Num());
	for (int i = 0; i < DelaunayTriangles.Num(); i += 3)
	{
		Triangles.Add(UDelaunayHelper::ConvertTriangleIDToTriangle(i));
	}*/

	UE_LOG(LogDelaunator, Log, TEXT("Created Delaunay Triangulation with %d points, %d triangles, and %d half-edges."), Coordinates.Num(), DelaunayTriangles.Num(), HalfEdges.Num());

	// Hull
	// Index of the first point in the hull
	HullStart = (int32)delaunay.hull_start;
	// All triangles making up our hull
	HullTriangles.Empty(delaunay.hull_tri.size());
	for (int i = 0; i < delaunay.hull_tri.size(); i++)
	{
		int32 triangleID = (int32)delaunay.hull_tri[i];
		HullTriangles.Add(triangleID);
	}
	// The previous triangle in the hull
	HullPrevious.Empty(delaunay.hull_prev.size());
	for (int i = 0; i < delaunay.hull_prev.size(); i++)
	{
		int32 triangleID = (int32)delaunay.hull_prev[i];
		HullPrevious.Add(triangleID);
	}
	// The next triangle in the hull
	HullNext.Empty(delaunay.hull_next.size());
	for (int i = 0; i < delaunay.hull_next.size(); i++)
	{
		int32 triangleID = (int32)delaunay.hull_next[i];
		HullNext.Add(triangleID);
	}
}

float FDelaunayMesh::GetHullArea(float& OutErrorAmount) const
{
	TArray<float> hullArea;
	int32 current = HullStart;
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
		float area = (Coordinates[current].X - Coordinates[HullPrevious[current]].X) * (Coordinates[current].Y + Coordinates[HullPrevious[current]].Y);
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

FVector2D UDelaunayHelper::GetTrianglePoint(const FDelaunayMesh& Triangulation, int32 TriangleIndex)
{
	if (!Triangulation.DelaunayTriangles.IsValidIndex(TriangleIndex))
	{
		UE_LOG(LogDelaunator, Error, TEXT("Invalid triangle ID: %d"), TriangleIndex);
		return FVector2D(-1, -1);
	}

	return Triangulation.Coordinates[UDelaunayHelper::PointsOfTriangle(Triangulation, TriangleIndex)[0]];
}

FDelaunayTriangle UDelaunayHelper::ConvertTriangleIDToTriangle(const FDelaunayMesh& Triangulation, int32 TriangleIndex)
{
	int32 offset = TriangleIndex % 3;

	FVector2D a, b, c;
	int32 aIndex, bIndex, cIndex;

	aIndex = Triangulation.DelaunayTriangles[TriangleIndex - offset];
	bIndex = Triangulation.DelaunayTriangles[TriangleIndex - offset + 1];
	cIndex = Triangulation.DelaunayTriangles[TriangleIndex - offset + 2];

	a = Triangulation.Coordinates[aIndex];
	b = Triangulation.Coordinates[bIndex];
	c = Triangulation.Coordinates[cIndex];

	return FDelaunayTriangle(a, b, c, aIndex, bIndex, cIndex);
}

TArray<int32> UDelaunayHelper::EdgesOfTriangle(int32 TriangleIndex)
{
	return TArray<int32> {3 * TriangleIndex, 3 * TriangleIndex + 1, 3 * TriangleIndex + 2 };
}

TArray<int32> UDelaunayHelper::PointsOfTriangle(const FDelaunayMesh& Triangulation, int32 TriangleIndex)
{
	TArray<int32> edges = UDelaunayHelper::EdgesOfTriangle(TriangleIndex);
	TArray<int32> points;
	for (int i = 0; i < edges.Num(); i++)
	{
		points.Add(Triangulation.DelaunayTriangles[edges[i]]);
	}
	return points;
}

FVector2D UDelaunayHelper::GetPointFromHalfEdge(const FDelaunayMesh& Triangulation, int32 HalfEdge)
{
	int32 index = GetPointIndexFromHalfEdge(Triangulation, HalfEdge);
	if (!Triangulation.Coordinates.IsValidIndex(index))
	{
		return FVector2D(-1, -1);
	}
	return Triangulation.Coordinates[index];
}

int32 UDelaunayHelper::GetPointIndexFromHalfEdge(const FDelaunayMesh& Triangulation, int32 HalfEdge)
{
	int32 triangleIndex = GetTriangleIndexFromHalfEdge(Triangulation, HalfEdge);
	if (!Triangulation.DelaunayTriangles.IsValidIndex(triangleIndex))
	{
		return -1;
	}

	int32 pointNum = HalfEdge % 3;
	TArray<int32> trianglePoints = UDelaunayHelper::PointsOfTriangle(Triangulation, triangleIndex);
	return trianglePoints[pointNum];
}

FDelaunayTriangle UDelaunayHelper::GetTriangleFromHalfEdge(const FDelaunayMesh& Triangulation, int32 HalfEdge)
{
	int32 triangleIndex = GetTriangleIndexFromHalfEdge(Triangulation, HalfEdge);
	if (!Triangulation.DelaunayTriangles.IsValidIndex(triangleIndex))
	{
		return FDelaunayTriangle();
	}
	return UDelaunayHelper::ConvertTriangleIDToTriangle(Triangulation, triangleIndex);
}

int32 UDelaunayHelper::GetTriangleIndexFromHalfEdge(const FDelaunayMesh& Triangulation, int32 HalfEdge)
{
	if (HalfEdge == -1)
	{
		return -1;
	}
	int32 pointNum = HalfEdge % 3;
	int32 triangleID = (HalfEdge - pointNum) / 3;
	if (!Triangulation.DelaunayTriangles.IsValidIndex(triangleID))
	{
		return -1;
	}
	return triangleID;
}

int32 UDelaunayHelper::NextHalfEdge(int32 HalfEdge)
{
	return (HalfEdge % 3 == 2) ? HalfEdge - 2 : HalfEdge + 1;
}

int32 UDelaunayHelper::PreviousHalfEdge(int32 HalfEdge)
{
	return (HalfEdge % 3 == 0) ? HalfEdge + 2 : HalfEdge - 1;
}

TArray<int32> UDelaunayHelper::EdgesAroundPoint(const FDelaunayMesh& Triangulation, int32 Start)
{
	TArray<int32> result;
	int32 incoming = Start;
	if (incoming == -1)
	{
		// Can't process
		return result;
	}
	do 
	{
		result.Add(incoming);
		const int32 outgoing = UDelaunayHelper::NextHalfEdge(incoming);
		incoming = Triangulation.HalfEdges[outgoing];
	} while (incoming != -1 && incoming != Start);
	return result;
}
