// Original Work Copyright (c) 2015 Simon Zeni (simonzeni@gmail.com), Modified Work Copyright (c) 2016 Jay M Stevens

#pragma once

#include "PolygonalMapGeneratorPrivatePCH.h"
#include "Diagrams/Delaunay.h"
#include <algorithm>

const std::vector<DelaunayTriangle>& Delaunay::triangulate(TArray<FVector2D> &vertices)
{
	// Determinate the super triangle
	float minX = vertices[0].X;
	float minY = vertices[0].Y;
	float maxX = minX;
	float maxY = minY;

	for (std::size_t i = 0; i < (std::size_t)vertices.Num(); ++i)
	{
		if (vertices[i].X < minX) minX = vertices[i].X;
		if (vertices[i].Y < minY) minY = vertices[i].Y;
		if (vertices[i].X > maxX) maxX = vertices[i].X;
		if (vertices[i].Y > maxY) maxY = vertices[i].Y;
	}

	float dx = maxX - minX;
	float dy = maxY - minY;
	float deltaMax = std::max(dx, dy);
	float midx = (minX + maxX) / 2.f;
	float midy = (minY + maxY) / 2.f;

	FVector2D p1(midx - 20 * deltaMax, midy - deltaMax);
	FVector2D p2(midx, midy + 20 * deltaMax);
	FVector2D p3(midx + 20 * deltaMax, midy - deltaMax);

	//std::cout << "Super triangle " << std::endl << DelaunayTriangle(p1, p2, p3) << std::endl;

	// Create a list of triangles, and add the supertriangle in it
	_triangles.push_back(DelaunayTriangle(p1, p2, p3));

	for (int i = 0; i < vertices.Num(); i++)
	{
		FVector2D p = vertices[i];
		//std::cout << "Traitement du point " << *p << std::endl;
		//std::cout << "_triangles contains " << _triangles.size() << " elements" << std::endl;	

		std::vector<DelaunayTriangle> badTriangles;
		std::vector<DelaunayEdge> polygon;

		for (auto t = begin(_triangles); t != end(_triangles); t++)
		{
			//std::cout << "Processing " << std::endl << *t << std::endl;

			if (t->circumCircleContains(p))
			{
				//std::cout << "Pushing bad triangle " << *t << std::endl;
				badTriangles.push_back(*t);
				polygon.push_back(t->e1);
				polygon.push_back(t->e2);
				polygon.push_back(t->e3);
			}
			else
			{
				//std::cout << " does not contains " << *p << " in his circum center" << std::endl;
			}
		}

		_triangles.erase(std::remove_if(begin(_triangles), end(_triangles), [badTriangles](DelaunayTriangle &t) {
			for (auto bt = begin(badTriangles); bt != end(badTriangles); bt++)
			{
				if (*bt == t)
				{
					//std::cout << "Removing bad triangle " << std::endl << *bt << " from _triangles" << std::endl;
					return true;
				}
			}
			return false;
		}), end(_triangles));

		std::vector<DelaunayEdge> badEdges;
		for (auto e1 = begin(polygon); e1 != end(polygon); e1++)
		{
			for (auto e2 = begin(polygon); e2 != end(polygon); e2++)
			{
				if (e1 == e2)
					continue;

				if (*e1 == *e2)
				{
					badEdges.push_back(*e1);
					badEdges.push_back(*e2);
				}
			}
		}

		polygon.erase(std::remove_if(begin(polygon), end(polygon), [badEdges](DelaunayEdge &e) {
			for (auto it = begin(badEdges); it != end(badEdges); it++)
			{
				if (*it == e)
					return true;
			}
			return false;
		}), end(polygon));

		for (auto e = begin(polygon); e != end(polygon); e++)
			_triangles.push_back(DelaunayTriangle(e->p1, e->p2, p));

	}

	_triangles.erase(std::remove_if(begin(_triangles), end(_triangles), [p1, p2, p3](DelaunayTriangle &t) {
		return t.containsVertex(p1) || t.containsVertex(p2) || t.containsVertex(p3);
	}), end(_triangles));

	for (auto t = begin(_triangles); t != end(_triangles); t++)
	{
		_edges.push_back(t->e1);
		_edges.push_back(t->e2);
		_edges.push_back(t->e3);
	}

	return _triangles;
}