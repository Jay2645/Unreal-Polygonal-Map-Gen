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

#pragma once

#include "Delaunay.h"
#include <vector>

struct VEdge
{
	FVector4 vEdge;
	FVector4 dEdge;
};

class VSite {
public:
	VSite() = default;
	TArray<VEdge> edges;
};

class Voronoi
{
public:
	Voronoi(TArray<FVector2D> &points);
	Voronoi(Delaunay delaunay);

	TArray<VSite> sites;
private:
	Delaunay DelaunayData;

	void MakeSites(std::vector<DelaunayTriangle> triangles);

	VSite MakeSite(DelaunayTriangle tri);
};