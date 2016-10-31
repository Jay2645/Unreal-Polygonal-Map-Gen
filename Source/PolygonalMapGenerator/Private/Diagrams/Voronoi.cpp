/*
* Copyright (c) 2012 by Joseph Marshall, Modified Work Copyright (c) 2016 Jay M Stevens
*
* Parts of this code were originally written by Steven Fortune.
*
* Permission to use, copy, modify, and distribute this software for any
* purpose without fee is hereby granted, provided that this entire notice
* is included in all copies of any software which is or includes a copy
* or modification of this software and in all copies of the supporting
* documentation for such software.
* THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
* WARRANTY.  IN PARTICULAR, NEITHER THE AUTHORS NOR AT&T MAKE ANY
* REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY
* OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
*/

#include "PolygonalMapGeneratorPrivatePCH.h"
#include "Diagrams/Voronoi.h"

Voronoi::Voronoi(TArray<FVector2D> &points)
{
	Delaunay triangulation;
	std::vector<DelaunayTriangle> triangles = triangulation.triangulate(points);
	DelaunayData = triangulation;
	MakeSites(triangles);
}

Voronoi::Voronoi(Delaunay delaunay)
{
	DelaunayData = delaunay;
	MakeSites(DelaunayData.getTriangles());
}

void Voronoi::MakeSites(std::vector<DelaunayTriangle> triangles)
{
	for (auto t = begin(triangles); t != end(triangles); t++)
	{
		DelaunayTriangle tri = *t;
		sites.Add(MakeSite(tri));
	}
}

VSite Voronoi::MakeSite(DelaunayTriangle tri)
{
	VSite site;


	bool bE1HasNeighbor = false;
	bool bE2HasNeighbor = false;
	bool bE3HasNeighbor = false;
	FVector2D tri0Circum = tri.circumCircleVertex;

	for (auto d = begin(DelaunayData.getTriangles()); d != end(DelaunayData.getTriangles()); d++)
	{
		DelaunayTriangle tri2 = *d;
		if (tri == tri2)
		{
			continue;
		}
		FVector2D tri1Circum = tri2.circumCircleVertex;
		if (tri.e1 == tri2.e1 || tri.e1 == tri2.e2 || tri.e1 == tri2.e3)
		{
			bE1HasNeighbor = true;
			VEdge edge;
			edge.dEdge = FVector4(tri.e1.p1.X, tri.e1.p1.Y, tri.e1.p2.X, tri.e1.p2.Y);
			edge.vEdge = FVector4(tri0Circum.X, tri0Circum.Y, tri1Circum.X, tri1Circum.Y);
			site.edges.Add(edge);
		}
		if (tri.e2 == tri2.e1 || tri.e2 == tri2.e2 || tri.e2 == tri2.e3)
		{
			bE2HasNeighbor = true;
			VEdge edge;
			edge.dEdge = FVector4(tri.e2.p1.X, tri.e2.p1.Y, tri.e2.p2.X, tri.e2.p2.Y);
			edge.vEdge = FVector4(tri0Circum.X, tri0Circum.Y, tri1Circum.X, tri1Circum.Y);
			site.edges.Add(edge);
		}
		if (tri.e3 == tri2.e1 || tri.e3 == tri2.e2 || tri.e3 == tri2.e3)
		{
			bE3HasNeighbor = true;
			VEdge edge;
			edge.dEdge = FVector4(tri.e3.p1.X, tri.e3.p1.Y, tri.e3.p2.X, tri.e3.p2.Y);
			edge.vEdge = FVector4(tri0Circum.X, tri0Circum.Y, tri1Circum.X, tri1Circum.Y);
			site.edges.Add(edge);
		}
	}

	if (!bE1HasNeighbor)
	{
		VEdge edge;
		edge.dEdge = FVector4(tri.e1.p1.X, tri.e1.p1.Y, tri.e1.p2.X, tri.e1.p2.Y);
		edge.vEdge = FVector4(tri0Circum.X, tri0Circum.Y, INFINITY, INFINITY);
		site.edges.Add(edge);
	}
	if (!bE2HasNeighbor)
	{
		VEdge edge;
		edge.dEdge = FVector4(tri.e2.p1.X, tri.e2.p1.Y, tri.e2.p2.X, tri.e2.p2.Y);
		edge.vEdge = FVector4(tri0Circum.X, tri0Circum.Y, INFINITY, INFINITY);
		site.edges.Add(edge);
	}
	if (!bE3HasNeighbor)
	{
		VEdge edge;
		edge.dEdge = FVector4(tri.e3.p1.X, tri.e3.p1.Y, tri.e3.p2.X, tri.e3.p2.Y);
		edge.vEdge = FVector4(tri0Circum.X, tri0Circum.Y, INFINITY, INFINITY);
		site.edges.Add(edge);
	}
	return site;
}