// Original Work Copyright (c) 2015 Simon Zeni (simonzeni@gmail.com), Modified Work Copyright (c) 2016 Jay M Stevens

#pragma once

#include "PolygonalMapGeneratorPrivatePCH.h"
#include "Diagrams/Delaunay.h"
#include "Diagrams/DelaunayTriangle.h"

#include <assert.h>
#include <math.h>


DelaunayTriangle::DelaunayTriangle(const FVector2D &_p1, const FVector2D &_p2, const FVector2D &_p3)
{
	p1 = _p1;
	p2 = _p2;
	p3 = _p3;
	e1 = DelaunayEdge(p1, p2);
	e2 = DelaunayEdge(p2, p3);
	e3 = DelaunayEdge(p3, p1);

	float ab = (p1.X * p1.X) + (p1.Y * p1.Y);
	float cd = (p2.X * p2.X) + (p2.Y * p2.Y);
	float ef = (p3.X * p3.X) + (p3.Y * p3.Y);
	float circum_x = (ab * (p3.Y - p2.Y) + cd * (p1.Y - p3.Y) + ef * (p2.Y - p1.Y)) / (p1.X * (p3.Y - p2.Y) + p2.X * (p1.Y - p3.Y) + p3.X * (p2.Y - p1.Y)) / 2.f;
	float circum_y = (ab * (p3.X - p2.X) + cd * (p1.X - p3.X) + ef * (p2.X - p1.X)) / (p1.Y * (p3.X - p2.X) + p2.Y * (p1.X - p3.X) + p3.Y * (p2.X - p1.X)) / 2.f;
	circumCircleVertex = FVector2D(circum_x, circum_y);
}

bool DelaunayTriangle::containsVertex(const FVector2D &v)
{
	return p1 == v || p2 == v || p3 == v;
}

bool DelaunayTriangle::circumCircleContains(const FVector2D &v)
{
	float ab = (p1.X * p1.X) + (p1.Y * p1.Y);
	float cd = (p2.X * p2.X) + (p2.Y * p2.Y);
	float ef = (p3.X * p3.X) + (p3.Y * p3.Y);
	float circum_x = (ab * (p3.Y - p2.Y) + cd * (p1.Y - p3.Y) + ef * (p2.Y - p1.Y)) / (p1.X * (p3.Y - p2.Y) + p2.X * (p1.Y - p3.Y) + p3.X * (p2.Y - p1.Y)) / 2.f;
	float circum_y = (ab * (p3.X - p2.X) + cd * (p1.X - p3.X) + ef * (p2.X - p1.X)) / (p1.Y * (p3.X - p2.X) + p2.Y * (p1.X - p3.X) + p3.Y * (p2.X - p1.X)) / 2.f;
	float circum_radius = sqrtf(((p1.X - circum_x) * (p1.X - circum_x)) + ((p1.Y - circum_y) * (p1.Y - circum_y)));

	float dist = sqrtf(((v.X - circum_x) * (v.X - circum_x)) + ((v.Y - circum_y) * (v.Y - circum_y)));
	return dist <= circum_radius;
}
