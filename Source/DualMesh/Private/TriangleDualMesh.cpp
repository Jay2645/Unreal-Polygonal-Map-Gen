/*
* Based on https://github.com/redblobgames/dual-mesh
* Original work copyright 2017 Red Blob Games <redblobgames@gmail.com>
* Unreal Engine 4 implementation copyright 2018 Jay Stevens <jaystevens42@gmail.com>
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
* Unreal Engine 4 Dual Mesh implementation.
*/

#include "TriangleDualMesh.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Actor.h"

FDualMesh::FDualMesh(const TArray<FVector2D>& GivenPoints, const FVector2D& MaxMapSize)
	: FDelaunayMesh(GivenPoints)
{
	MaxSize = MaxMapSize;
	NumSolidSides = DelaunayTriangles.Num();
	AddGhostStructure();

	UE_LOG(LogDualMesh, Log, TEXT("Final dual mesh had %d solid sides and a map size of %f, %f."), NumSolidSides, MaxSize.X, MaxSize.Y);
}

void FDualMesh::AddGhostStructure()
{
	const FPointIndex ghostRegion = Coordinates.Num();
	int32 numUnpairedSides = 0;
	FPointIndex firstUnpairedEdge = FPointIndex();
	TArray<FPointIndex> regionsUnpairedSides;
	regionsUnpairedSides.SetNumZeroed(NumSolidSides);
	for (FPointIndex i = 0; i < NumSolidSides; i++)
	{
		if (!HalfEdges[i].IsValid())
		{
			numUnpairedSides++;
			regionsUnpairedSides[DelaunayTriangles[i]] = i;
			firstUnpairedEdge = i;
		}
	}

	TArray<FVector2D> newVertices;
	newVertices.Append(Coordinates);
	newVertices.Add(FVector2D(MaxSize.X / 2.0f, MaxSize.Y / 2.0f));

	TArray<FPointIndex> sideNewStartRegions;
	sideNewStartRegions.Append(DelaunayTriangles);
	sideNewStartRegions.SetNumZeroed(NumSolidSides + 3 * numUnpairedSides);

	TArray<FSideIndex> sideNewOppositeSides;
	sideNewOppositeSides.Append(HalfEdges);
	sideNewOppositeSides.SetNumZeroed(NumSolidSides + 3 * numUnpairedSides);

	int s = firstUnpairedEdge;
	for (int i = 0; i < numUnpairedSides; i++)
	{
		// Construct a ghost side for s
		FSideIndex ghostSide = NumSolidSides + 3 * i;
		sideNewOppositeSides[s] = ghostSide;
		sideNewOppositeSides[ghostSide] = s;
		sideNewStartRegions[ghostSide] = sideNewStartRegions[UTriangleDualMesh::s_next_s(s)];

		// Construct the rest of the ghost triangle
		sideNewStartRegions[ghostSide + 1] = sideNewStartRegions[s];
		sideNewStartRegions[ghostSide + 2] = ghostRegion;

		int k = NumSolidSides + (3 * i + 4) % (3 * numUnpairedSides);
		sideNewOppositeSides[ghostSide + 2] = k;
		sideNewOppositeSides[k] = ghostSide + 2;

		s = regionsUnpairedSides[sideNewStartRegions[UTriangleDualMesh::s_next_s(s)]];
	}

	// Put the new arrays back into this structure
	Coordinates = newVertices;
	DelaunayTriangles = sideNewStartRegions;
	HalfEdges = sideNewOppositeSides;
}

FTriangleIndex UTriangleDualMesh::s_to_t(FSideIndex s)
{
	return FTriangleIndex(UDelaunayHelper::GetTriangleIndexFromHalfEdge(s) / 3);
}

FSideIndex UTriangleDualMesh::s_next_s(FSideIndex s)
{
	return UDelaunayHelper::NextHalfEdge(s);
}

FSideIndex UTriangleDualMesh::s_prev_s(FSideIndex s)
{
	return UDelaunayHelper::PreviousHalfEdge(s);
}

float UTriangleDualMesh::r_x(FPointIndex r) const
{
	return r_pos(r).X;
}

float UTriangleDualMesh::r_y(FPointIndex r) const
{
	return r_pos(r).Y;
}

float UTriangleDualMesh::t_x(FTriangleIndex t) const
{
	return t_pos(t).X;
}

float UTriangleDualMesh::t_y(FTriangleIndex t) const
{
	return t_pos(t).Y;
}

FVector2D UTriangleDualMesh::r_pos(FPointIndex r) const
{
	if(_r_vertex.IsValidIndex(r))
	{
		return _r_vertex[r];
	}
	else
	{
		return FVector2D(-1.0f, -1.0f);
	}
}

FVector2D UTriangleDualMesh::t_pos(FTriangleIndex t) const
{
	if(_t_vertex.IsValidIndex(t))
	{
		return _t_vertex[t];
	}
	else
	{
		return FVector2D(-1.0f, -1.0f);
	}
}

FPointIndex UTriangleDualMesh::s_begin_r(FSideIndex s) const
{
	return UDelaunayHelper::GetPointIndexFromHalfEdge(Mesh, s);
}

FPointIndex UTriangleDualMesh::s_end_r(FSideIndex s) const
{		
	return s_begin_r(UTriangleDualMesh::s_next_s(s));
}

FTriangleIndex UTriangleDualMesh::s_inner_t(FSideIndex s) const
{
	return UTriangleDualMesh::s_to_t(s);
}

FTriangleIndex UTriangleDualMesh::s_outer_t(FSideIndex s) const
{
	return UTriangleDualMesh::s_to_t(s_opposite_s(s));
}

FSideIndex UTriangleDualMesh::s_opposite_s(FSideIndex s) const
{
	return UDelaunayHelper::OppositeHalfEdge(Mesh, s);
}

TArray<FSideIndex> UTriangleDualMesh::t_circulate_s(FTriangleIndex t) const
{
	return UDelaunayHelper::EdgesOfTriangle(t * 3);
}

TArray<FPointIndex> UTriangleDualMesh::t_circulate_r(FTriangleIndex t) const
{
	TArray<FPointIndex> out_r;
	TArray<FSideIndex> out_s = t_circulate_s(t);
	out_r.SetNum(3);
	for (int i = 0; i < 3; i++) 
	{ 
		out_r[i] = UDelaunayHelper::GetPointIndexFromHalfEdge(Mesh, out_s[i]);
	} 
	return out_r;
}

TArray<FTriangleIndex> UTriangleDualMesh::t_circulate_t(FTriangleIndex t) const
{
	TArray<FTriangleIndex> out_t;
	TArray<FSideIndex> out_s = t_circulate_s(t);
	out_t.SetNum(3);
	for (int i = 0; i < 3; i++)
	{
		out_t[i] = s_outer_t(out_s[i]);
	}
	return out_t;
}

TArray<FSideIndex> UTriangleDualMesh::r_circulate_s(FPointIndex r) const
{
	TArray<FSideIndex> out_s;
	if(!_r_in_s.Contains(r))
	{
		UE_LOG(LogDualMesh, Warning, TEXT("Region list did not contain point %d!"), r);
		return out_s;
	}

	const FSideIndex s0 = _r_in_s[r];
	FSideIndex incoming = s0;
	do
	{
		if (!_halfedges.IsValidIndex(incoming))
		{
			UE_LOG(LogDualMesh, Error, TEXT("Incoming side was invalid!"));
			return out_s;
		}
		FSideIndex next = _halfedges[incoming];
		if (!next.IsValid())
		{
			UE_LOG(LogDualMesh, Error, TEXT("Next side was invalid!"));
			return out_s;
		}
		out_s.Add(next);
		FSideIndex outgoing = UTriangleDualMesh::s_next_s(incoming);
		incoming = _halfedges[outgoing];
	} while (incoming.IsValid() && incoming != s0);
	return out_s;
}

TArray<FPointIndex> UTriangleDualMesh::r_circulate_r(FPointIndex r) const
{
	TArray<FPointIndex> out_r;
	if (!_r_in_s.Contains(r))
	{
		UE_LOG(LogDualMesh, Warning, TEXT("Region list did not contain point %d!"), r);
		return out_r;
	}

	const FSideIndex s0 = _r_in_s[r];
	if (s0.IsValid())
	{
		FSideIndex incoming = s0;
		do {
			FPointIndex next = s_begin_r(incoming);
			if (!next.IsValid())
			{
				UE_LOG(LogDualMesh, Error, TEXT("Next region was invalid!"));
				return out_r;
			}
			out_r.Add(next);
			FSideIndex outgoing = UTriangleDualMesh::s_next_s(incoming);
			incoming = _halfedges[outgoing];
		} while (incoming.IsValid() && incoming != s0);
	}
	else
	{
		UE_LOG(LogDualMesh, Warning, TEXT("Attempted to start from an invalid region (%d)."), r);
	}
	return out_r;
}

TArray<FTriangleIndex> UTriangleDualMesh::r_circulate_t(FPointIndex r) const
{
	TArray<FTriangleIndex> out_t;
	if (!_r_in_s.Contains(r))
	{
		UE_LOG(LogDualMesh, Warning, TEXT("Region list did not contain point %d!"), r);
		return out_t;
	}

	const FSideIndex s0 = _r_in_s[r];
	FSideIndex incoming = s0;
	do
	{
		FTriangleIndex next = UTriangleDualMesh::s_to_t(incoming);
		if (!next.IsValid())
		{
			UE_LOG(LogDualMesh, Error, TEXT("Next triangle was invalid!"));
			return out_t;
		}
		out_t.Add(next);
		FSideIndex outgoing = UTriangleDualMesh::s_next_s(incoming);
		incoming = _halfedges[outgoing];
	} while (incoming.IsValid() && incoming != s0);
	return out_t;
}

FPointIndex UTriangleDualMesh::ghost_r() const
{
	if (NumRegions == 0)
	{
		UE_LOG(LogDualMesh, Error, TEXT("No regions defined yet! Did you initialize the dual mesh?"));
		return FPointIndex();
	}
	return FPointIndex(NumRegions - 1);
}

bool UTriangleDualMesh::s_ghost(FSideIndex s) const
{
	return s >= NumSolidSides;
}

bool UTriangleDualMesh::r_ghost(FPointIndex r) const
{
	return r == ghost_r();
}

bool UTriangleDualMesh::t_ghost(FTriangleIndex t) const
{
	return s_ghost(UDelaunayHelper::TriangleIndexToEdge(t * 3));
}

bool UTriangleDualMesh::t_ghost(const FDelaunayTriangle& Triangle) const
{
	return r_ghost(Triangle.AIndex) || r_ghost(Triangle.BIndex) || r_ghost(Triangle.CIndex);
}

bool UTriangleDualMesh::s_boundary(FSideIndex s) const
{
	return s_ghost(s) && (s % 3 == 0);
}

bool UTriangleDualMesh::r_boundary(FPointIndex r) const
{
	return r < NumBoundaryRegions;
}

void UTriangleDualMesh::InitializeMesh(const FDualMesh& Input, int32 BoundaryRegions)
{
	Mesh = Input;
	NumBoundaryRegions = BoundaryRegions;
	NumSolidSides = Mesh.NumSolidSides;
	_r_vertex = Mesh.Coordinates;
	_triangles.SetNum(Mesh.DelaunayTriangles.Num() / 3);
	for (FTriangleIndex t = 0; t < Mesh.DelaunayTriangles.Num(); t += 3)
	{
		_triangles[t / 3] = UDelaunayHelper::ConvertTriangleIDToTriangle(Mesh, t);
	}
	_halfedges = Mesh.HalfEdges;

	NumSides = _halfedges.Num();
	NumRegions = _r_vertex.Num();
	NumSolidRegions = NumRegions - 1;
	NumTriangles = _triangles.Num();
	NumSolidTriangles = NumSolidSides / 3;

	for (FSideIndex s = 0; s < _halfedges.Num(); s++)
	{
		FPointIndex endpoint = UDelaunayHelper::GetPointIndexFromHalfEdge(Mesh, UTriangleDualMesh::s_next_s(s));
		if (!_r_in_s.Contains(endpoint) || !_halfedges[s].IsValid())
		{
			_r_in_s.Add(endpoint, s);
		}
	}

	// Construct triangle coordinates
	_t_vertex.SetNum(NumTriangles);
	for (FSideIndex s = 0; s < _halfedges.Num(); s += 3)
	{
		FDelaunayTriangle triangle = UDelaunayHelper::GetTriangleFromHalfEdge(Mesh, s);
		FVector2D a = triangle.A;
		FVector2D b = triangle.B;
		FVector2D c = triangle.C;

		if (s_ghost(s))
		{
			// ghost triangle center is just outside the unpaired side
			float dx = b.X - a.X;
			float dy = b.Y - a.Y;
			float scale = 10.0f / FMath::Sqrt(dx * dx + dy * dy); // go 10 units away from side
			_t_vertex[s / 3] = FVector2D(0.5f * (a.X + b.X) + dy * scale, 0.5f * (a.Y + b.Y) - dx * scale);
		}
		else
		{
			// solid triangle center is at the centroid
			_t_vertex[s / 3] = FVector2D((a.X + b.X + c.X) / 3.0f, (a.Y + b.Y + c.Y) / 3.0f);
		}
	}
}

FVector2D UTriangleDualMesh::GetSize() const
{
	return Mesh.MaxSize;
}

TArray<FVector2D>& UTriangleDualMesh::GetPoints()
{
	return _r_vertex;
}

TArray<FVector2D>& UTriangleDualMesh::GetTriangleCentroids()
{
	return _t_vertex;
}

TArray<FSideIndex>& UTriangleDualMesh::GetHalfEdges()
{
	return _halfedges;
}

TArray<FDelaunayTriangle>& UTriangleDualMesh::GetTriangles()
{
	return _triangles;
}

FDualMesh& UTriangleDualMesh::GetRawMesh()
{
	return Mesh;
}

void UTriangleDualMesh::Draw(const AActor* WorldObject) const
{
	Draw(WorldObject->GetWorld());
}

void UTriangleDualMesh::Draw(const UWorld* World) const
{
	DrawDelaunayVertices(World);
	DrawDelaunayEdges(World);
	//DrawVoronoiPoints(World);
	//DrawVoronoiEdges(World);
}

void UTriangleDualMesh::DrawDelaunayVertices(const UWorld* World) const
{
	// Draw delaunay vertices as red dots
	int32 count = 0;
	for (int r = 0; r < _r_vertex.Num(); r++)
	{
		FVector2D vertex = _r_vertex[r];
		float zCoord = r_ghost(r) ? -500.0f : 0.0f;
		FVector vertexWorldSpace = FVector(vertex.X, vertex.Y, zCoord);
		DrawDebugPoint(World, vertexWorldSpace, 10.0f, FColor::Red, false, 999.0f);
		count++;
	}
	UE_LOG(LogDualMesh, Log, TEXT("Drew %d delaunay points."), count);
}

void UTriangleDualMesh::DrawVoronoiEdges(const UWorld* World) const
{
	// Draw voronoi polygons as green lines
	int32 count = 0;
	for (int e = 0; e < _halfedges.Num(); e++)
	{
		if (e < _halfedges[e])
		{
			FDelaunayTriangle triangleP = UDelaunayHelper::GetTriangleFromHalfEdge(Mesh, e);
			FDelaunayTriangle triangleQ = UDelaunayHelper::GetTriangleFromHalfEdge(Mesh, _halfedges[e]);
			if (!triangleP.IsValid() || !triangleQ.IsValid())// || s_ghost(e) || s_ghost(_halfedges[e]))
			{
				continue;
			}
			FVector2D p = triangleP.GetCircumcenter();
			FVector2D q = triangleQ.GetCircumcenter();
			FVector pVector = FVector(p.X, p.Y, 0.0f);
			FVector qVector = FVector(q.X, q.Y, 0.0f);
			UE_LOG(LogDualMesh, Log, TEXT("Voronoi edge: (%f, %f) -> (%f, %f)"), pVector.X, pVector.Y, qVector.X, qVector.Y);
			DrawDebugLine(World, pVector, qVector, FColor::Green, false, 999.0f);
			count++;
		}
	}
	UE_LOG(LogDualMesh, Log, TEXT("Drew %d voronoi edges."), count);
}

void UTriangleDualMesh::DrawDelaunayEdges(const UWorld* World) const
{
	int32 count = 0;
	for (FSideIndex e = 0; e < _halfedges.Num(); e++)
	{
		if (e < _halfedges[e])
		{
			FPointIndex pIndex = UDelaunayHelper::GetPointIndexFromHalfEdge(Mesh, e);
			FPointIndex qIndex = UDelaunayHelper::GetPointIndexFromHalfEdge(Mesh, UDelaunayHelper::NextHalfEdge(e));
			const FVector2D p = _r_vertex[pIndex];
			const FVector2D q = _r_vertex[qIndex];
			float pZCoord = r_ghost(pIndex) ? -1000.0f : 0.0f;
			float qZCoord = r_ghost(qIndex) ? -1000.0f : 0.0f;
			FVector pVector = FVector(p.X, p.Y, pZCoord);
			FVector qVector = FVector(q.X, q.Y, qZCoord);
			DrawDebugDirectionalArrow(World, pVector, qVector, 10.0f, FColor::Magenta, false, 999.0f);
			count++;
		}
	}
	UE_LOG(LogDualMesh, Log, TEXT("Drew %d delaunay edges."), count);
}

void UTriangleDualMesh::DrawVoronoiPoints(const UWorld* World) const
{
	int32 count = 0;
	for (FTriangleIndex t = 0; t < _triangles.Num(); t++)
	{
		if (t_ghost(t))
		{
			continue;
		}
		FDelaunayTriangle triangle = _triangles[t];
		FVector2D vertex = triangle.GetCircumcenter();
		FVector vertexWorldSpace = FVector(vertex.X, vertex.Y, 0.0f);
		DrawDebugPoint(World, vertexWorldSpace, 10.0f, FColor::Blue, false, 999.0f);
		count++;
	}
	UE_LOG(LogDualMesh, Log, TEXT("Drew %d voronoi points."), count);
}
