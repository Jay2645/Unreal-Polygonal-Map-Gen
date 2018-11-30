// Copyright 2018 Schemepunk Studios

#include "DelaunayHelper.h"
#include <delaunator.hpp>

float FDelaunayTriangle::GetArea() const
{
	return FMath::Abs(A.X * (B.Y - C.Y) + B.X * (C.Y - A.Y) + C.X * (A.Y - B.Y));
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
	DelaunayCoords.Empty(delaunay.coords.size());
	for (int i = 0; i < delaunay.coords.size(); i += 2)
	{
		// The Delaunator stores everything in a vector of doubles
		// It doesn't store information about which doubles are paired to each other
		// However, we know that all even indices represent the X value, while odd indices
		// represent the Y value
		FVector2D coord = FVector2D((float)delaunay.coords[i], (float)delaunay.coords[i + 1]);
		DelaunayCoords.Add(coord.X);
		DelaunayCoords.Add(coord.Y);
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
	Triangles.Empty(DelaunayTriangles.Num());
	for (int i = 0; i < DelaunayTriangles.Num(); i += 3)
	{
		FVector2D a, b, c;
		int32 aIndex, bIndex, cIndex;

		aIndex = DelaunayTriangles[i];
		bIndex = DelaunayTriangles[i + 1];
		cIndex = DelaunayTriangles[i + 2];

		a = Coordinates[aIndex];
		b = Coordinates[bIndex];
		c = Coordinates[cIndex];
		Triangles.Add(FDelaunayTriangle(a, b, c, aIndex, bIndex, cIndex));
	}

	UE_LOG(LogDelaunator, Log, TEXT("Created Delaunay Triangulation with %d points (%d raw points), %d triangles (%d raw triangles), and %d half-edges."), Coordinates.Num(), DelaunayCoords.Num(), Triangles.Num(), DelaunayTriangles.Num(), HalfEdges.Num());

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

float FDelaunayMesh::GetHullArea() const
{
	TArray<float> hullArea;
	int32 current = HullStart;
	do
	{
		if (!Coordinates.IsValidIndex(current))
		{
			UE_LOG(LogDelaunator, Error, TEXT("Invalid Coordinate index. Index: %d, Array size: %d, Reported Delaunay Array size: %d"), current, Coordinates.Num(), DelaunayCoords.Num());
			return 0.0f;
		}
		if (!HullPrevious.IsValidIndex(current))
		{
			UE_LOG(LogDelaunator, Error, TEXT("Invalid HullPrevious index. Index: %d, Array size: %d"), current, HullPrevious.Num());
			return 0.0f;
		}
		if (!Coordinates.IsValidIndex(HullPrevious[current]))
		{
			UE_LOG(LogDelaunator, Error, TEXT("Invalid Coordinate index. Index: %d, Array size: %d, Reported Delaunay Array size: %d"), HullPrevious[current], Coordinates.Num(), DelaunayCoords.Num());
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
	return Sum(hullArea);
}

float FDelaunayMesh::Sum(const TArray<float>& Area) const
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

FVector2D UDelaunayHelper::GetTrianglePoint(const FDelaunayMesh& Triangulation, int32 TriangleID)
{
	if (!Triangulation.Triangles.IsValidIndex(TriangleID))
	{
		UE_LOG(LogDelaunator, Error, TEXT("Invalid triangle ID: %d"), TriangleID);
		return FVector2D(-1, -1);
	}

	return Triangulation.Triangles[TriangleID].A;
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
	if (!Triangulation.Triangles.IsValidIndex(triangleIndex))
	{
		return -1;
	}

	int32 pointNum = HalfEdge % 3;
	switch (pointNum)
	{
	case 0:
		return Triangulation.Triangles[triangleIndex].AIndex;
	case 1:
		return Triangulation.Triangles[triangleIndex].BIndex;
	case 2:
		return Triangulation.Triangles[triangleIndex].CIndex;
	default:
		checkNoEntry();
		return -1;
	}
}

FDelaunayTriangle UDelaunayHelper::GetTriangleFromHalfEdge(const FDelaunayMesh& Triangulation, int32 HalfEdge)
{
	int32 triangleIndex = GetTriangleIndexFromHalfEdge(Triangulation, HalfEdge);
	if (!Triangulation.Triangles.IsValidIndex(triangleIndex))
	{
		return FDelaunayTriangle();
	}
	return Triangulation.Triangles[triangleIndex];
}

int32 UDelaunayHelper::GetTriangleIndexFromHalfEdge(const FDelaunayMesh& Triangulation, int32 HalfEdge)
{
	if (HalfEdge == -1)
	{
		return -1;
	}
	int32 pointNum = HalfEdge % 3;
	int32 triangleID = (HalfEdge - pointNum) / 3;
	if (!Triangulation.Triangles.IsValidIndex(triangleID))
	{
		return -1;
	}
	return triangleID;
}