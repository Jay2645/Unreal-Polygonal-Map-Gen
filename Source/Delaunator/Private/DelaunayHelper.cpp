// Copyright 2018 Schemepunk Studios

#include "DelaunayHelper.h"
#include <delaunator.hpp>

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
		FVector2D coord = FVector2D((float)delaunay.coords[i], (float)delaunay.coords[i + 1]);
		DelaunayCoords.Add(coord.X);
		DelaunayCoords.Add(coord.Y);
		Coordinates.Add(coord);
	}

	// Half-edges
	HalfEdges.Empty(delaunay.halfedges.size());
	for (int i = 0; i < delaunay.halfedges.size(); i++)
	{
		// Technically, each half-edge is represented by a size_t value, not an int.
		// However, Unreal doesn't accept size_t in Blueprint arrays, nor does it
		// accept uint64 values. The highest we can go while still supporting Blueprint
		// is int32. This cast is technically unsafe, but it's unlikely we'll have issues.
		HalfEdges.Add((int32)delaunay.halfedges[i]);
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
		a.X = DelaunayCoords[2 * DelaunayTriangles[i]];
		a.Y = DelaunayCoords[2 * DelaunayTriangles[i] + 1];
		b.X = DelaunayCoords[2 * DelaunayTriangles[i + 1]];
		b.Y = DelaunayCoords[2 * DelaunayTriangles[i + 1] + 1];
		c.X = DelaunayCoords[2 * DelaunayTriangles[i + 2]];
		c.Y = DelaunayCoords[2 * DelaunayTriangles[i + 2] + 1];
		Triangles.Add(FDelaunayTriangle(a, b, c));
	}
	
	// Hull
	HullStart = (int32)delaunay.hull_start;
	HullTriangles.Empty(delaunay.hull_tri.size());
	for (int i = 0; i < delaunay.hull_tri.size(); i++)
	{
		HullTriangles.Add((int32)delaunay.hull_tri[i]);
	}
	HullPrevious.Empty(delaunay.hull_prev.size());
	for (int i = 0; i < delaunay.hull_prev.size(); i++)
	{
		HullPrevious.Add((int32)delaunay.hull_prev[i]);
	}
	HullNext.Empty(delaunay.hull_next.size());
	for (int i = 0; i < delaunay.hull_next.size(); i++)
	{
		HullNext.Add((int32)delaunay.hull_next[i]);
	}
}

FDelaunayMesh UDelaunayHelper::CreateDelaunayTriangulation(const TArray<FVector2D>& Points)
{
	return FDelaunayMesh(Points);
}
