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

int32 UTriangleDualMesh::s_to_t(int32 s)
{
	return FMath::FloorToInt((float)s / 3.0f);
}

int32 UTriangleDualMesh::s_next_s(int32 s)
{
	return UDelaunayHelper::NextHalfEdge(s);
}

int32 UTriangleDualMesh::s_prev_s(int32 s)
{
	return UDelaunayHelper::PreviousHalfEdge(s);
}

float UTriangleDualMesh::r_x(int32 r) const
{
	return r_pos(r).X;
}

float UTriangleDualMesh::r_y(int32 r) const
{
	return r_pos(r).Y;
}

float UTriangleDualMesh::t_x(int32 t) const
{
	return t_pos(t).X;
}

float UTriangleDualMesh::t_y(int32 t) const
{
	return t_pos(t).Y;
}

FVector2D UTriangleDualMesh::r_pos(int32 r) const
{
	return _r_vertex[r];
}

FVector2D UTriangleDualMesh::t_pos(int32 t) const
{
	return _t_vertex[t];
}

int32 UTriangleDualMesh::s_begin_r(int32 s) const
{
	return _triangles[s];
}

int32 UTriangleDualMesh::s_end_r(int32 s) const
{
	return _triangles[UTriangleDualMesh::s_next_s(s)];
}

int32 UTriangleDualMesh::s_inner_t(int32 s) const
{
	return UTriangleDualMesh::s_to_t(s);
}

int32 UTriangleDualMesh::s_outer_t(int32 s) const
{
	return UTriangleDualMesh::s_to_t(_halfedges[s]);
}

int32 UTriangleDualMesh::s_opposite_s(int32 s) const
{
	return _halfedges[s];
}

TArray<int32> UTriangleDualMesh::t_circulate_s(int32 t) const
{
	TArray<int32> out_s;
	out_s.SetNum(3);
	for (int32 i = 0; i < 3; i++)
	{
		out_s[i] = 3 * t + i; 
	} 
	return out_s;
}

TArray<int32> UTriangleDualMesh::t_circulate_r(int32 t) const
{
	TArray<int32> out_r;
	out_r.SetNum(3);
	for (int32 i = 0; i < 3; i++) 
	{ 
		out_r[i] = _triangles[3 * t + i]; 
	} 
	return out_r;
}

TArray<int32> UTriangleDualMesh::t_circulate_t(int32 t) const
{
	TArray<int32> out_t;
	out_t.SetNum(3);
	for (int32 i = 0; i < 3; i++)
	{
		out_t[i] = s_outer_t(3 * t + i);
	}
	return out_t;
}

TArray<int32> UTriangleDualMesh::r_circulate_s(int32 r) const
{
	const int32 s0 = _r_in_s[r];
	int32 incoming = s0;
	TArray<int32> out_s;
	do
	{
		out_s.Add(_halfedges[incoming]);
		int32 outgoing = UTriangleDualMesh::s_next_s(incoming);
		incoming = _halfedges[outgoing];
	} while (incoming != -1 && incoming != s0);
	return out_s;
}

TArray<int32> UTriangleDualMesh::r_circulate_r(int32 r) const
{
	const int32 s0 = _r_in_s[r];
	int32 incoming = s0;
	TArray<int32> out_r;
	do {
		out_r.Add(s_begin_r(incoming));
		int32 outgoing = UTriangleDualMesh::s_next_s(incoming);
		incoming = _halfedges[outgoing];
	} while (incoming != -1 && incoming != s0);
	return out_r;
}

TArray<int32> UTriangleDualMesh::r_circulate_t(int32 r) const
{
	const int32 s0 = _r_in_s[r];
	int32 incoming = s0;
	TArray<int32> out_t;
	do
	{
		out_t.Add(UTriangleDualMesh::s_to_t(incoming));
		int32 outgoing = UTriangleDualMesh::s_next_s(incoming);
		incoming = _halfedges[outgoing];
	} while (incoming != -1 && incoming != s0);
	return out_t;
}

int32 UTriangleDualMesh::ghost_r() const
{
	return NumRegions - 1;
}

bool UTriangleDualMesh::s_ghost(int32 s) const
{
	return s >= NumSolidSides;
}

bool UTriangleDualMesh::r_ghost(int32 r) const
{
	return r == NumRegions - 1;
}

bool UTriangleDualMesh::t_ghost(int32 t) const
{
	return s_ghost(3 * t);
}

bool UTriangleDualMesh::s_boundary(int32 s) const
{
	return s_ghost(s) && (s % 3 == 0);
}

bool UTriangleDualMesh::r_boundary(int32 r) const
{
	return r < NumBoundaryRegions;
}

void UTriangleDualMesh::InitializeMesh(const FDualMesh& Input, int32 BoundaryRegions)
{
	Mesh = Input;
	NumBoundaryRegions = BoundaryRegions;
	NumSolidSides = Mesh.NumSolidSides;
	_r_vertex = Mesh.Coordinates;
	_triangles = Mesh.DelaunayTriangles;
	_halfedges = Mesh.HalfEdges;

	NumSides = _triangles.Num();
	NumRegions = _r_vertex.Num();
	NumSolidRegions = NumRegions - 1;
	NumTriangles = NumSides / 3;
	NumSolidTriangles = NumSolidSides / 3;

	_r_in_s.SetNum(NumRegions);
	for (int32 s = 0; s < _triangles.Num(); s++)
	{
		int32 endpoint = _triangles[UTriangleDualMesh::s_next_s(s)];
		if (_r_in_s[endpoint] == 0 || _halfedges[s] == -1)
		{
			_r_in_s[endpoint] = s;
		}
	}

	// Construct triangle coordinates
	_t_vertex.SetNum(NumTriangles);
	for (int32 s = 0; s < _triangles.Num(); s += 3)
	{
		FVector2D a = _r_vertex[_triangles[s]];
		FVector2D b = _r_vertex[_triangles[s + 1]];
		FVector2D c = _r_vertex[_triangles[s + 2]];
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
