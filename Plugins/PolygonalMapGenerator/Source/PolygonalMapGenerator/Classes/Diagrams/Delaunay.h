// Original Work Copyright (c) 2015 Simon Zeni (simonzeni@gmail.com), Modified Work Copyright (c) 2016 Jay M Stevens

#pragma once

#include "DelaunayTriangle.h"
#include "Vector2D.h"

#include <vector>

class Delaunay
{
public:
	const std::vector<DelaunayTriangle>& triangulate(TArray<FVector2D> &vertices);
	const std::vector<DelaunayTriangle>& getTriangles() const { return _triangles; };
	const std::vector<DelaunayEdge>& getEdges() const { return _edges; };

private:
	std::vector<DelaunayTriangle> _triangles;
	std::vector<DelaunayEdge> _edges;
};