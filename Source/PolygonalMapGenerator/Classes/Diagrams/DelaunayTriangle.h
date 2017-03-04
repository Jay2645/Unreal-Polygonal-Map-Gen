// Original Work Copyright (c) 2015 Simon Zeni (simonzeni@gmail.com), Modified Work Copyright (c) 2016 Jay M Stevens

#pragma once

#include "Vector2D.h"

class DelaunayEdge
{
public:
	DelaunayEdge() {};

	DelaunayEdge(const FVector2D &p1, const FVector2D &p2) : p1(p1), p2(p2) {};

	DelaunayEdge(const DelaunayEdge &e) : p1(e.p1), p2(e.p2) {};

	FVector2D p1;
	FVector2D p2;
};

inline bool operator == (const DelaunayEdge & e1, const DelaunayEdge & e2)
{
	return 	(e1.p1 == e2.p1 && e1.p2 == e2.p2) ||
		(e1.p1 == e2.p2 && e1.p2 == e2.p1);
}

class DelaunayTriangle
{
public:
	DelaunayTriangle(const FVector2D &_p1, const FVector2D &_p2, const FVector2D &_p3);

	bool containsVertex(const FVector2D &v);
	bool circumCircleContains(const FVector2D &v);
	DelaunayEdge e1;
	DelaunayEdge e2;
	DelaunayEdge e3;
	FVector2D circumCircleVertex;
	FVector2D p1;
	FVector2D p2;
	FVector2D p3;
};

inline bool operator == (const DelaunayTriangle &t1, const DelaunayTriangle &t2)
{
	return	(t1.p1 == t2.p1 || t1.p1 == t2.p2 || t1.p1 == t2.p3) &&
		(t1.p2 == t2.p1 || t1.p2 == t2.p2 || t1.p2 == t2.p3) &&
		(t1.p3 == t2.p1 || t1.p3 == t2.p2 || t1.p3 == t2.p3);
}
