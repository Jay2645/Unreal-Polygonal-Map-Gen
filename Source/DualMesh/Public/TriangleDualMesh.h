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

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DelaunayHelper.h"
#include "TriangleDualMesh.generated.h"

USTRUCT(BlueprintType)
struct DUALMESH_API FDualMesh : public FDelaunayMesh
{
	GENERATED_BODY()
public:
	FVector2D MaxSize;

	int32 NumSolidSides;

public:
	FDualMesh()
		: FDelaunayMesh()
	{
		HullStart = -1;
		NumSolidSides = -1;
		MaxSize = FVector2D::ZeroVector;
	}

	FDualMesh(const TArray<FVector2D>& GivenPoints, const FVector2D& MaxMapSize);
private:
	void AddGhostStructure();
};

/**
* Represent a triangle-polygon dual mesh with:
*   - Regions (r)
*   - Sides (s)
*   - Triangles (t)
*
* Each element has an id:
*   - 0 <= r < numRegions
*   - 0 <= s < numSides
*   - 0 <= t < numTriangles
*
* Naming convention: x_name_y takes x (r, s, t) as input and produces
* y (r, s, t) as output. If the output isn't a mesh index (r, s, t)
* then the _y suffix is omitted.
*
* A side is directed. If two triangles t0, t1 are adjacent, there will
* be two sides representing the boundary, one for t0 and one for t1. These
* can be accessed with s_inner_t and s_outer_t.
*
* A side also represents the boundary between two regions. If two regions
* r0, r1 are adjacent, there will be two sides representing the boundary,
* s_begin_r and s_end_r.
*
* Each side will have a pair, accessed with s_opposite_s.
*
* The mesh has no boundaries; it wraps around the "back" using a
* "ghost" region. Some regions are marked as the boundary; these are
* connected to the ghost region. Ghost triangles and ghost sides
* connect these boundary regions to the ghost region. Elements that
* aren't "ghost" are called "solid".
*/
UCLASS()
class DUALMESH_API UTriangleDualMesh : public UObject
{
	GENERATED_BODY()
	friend class UDualMeshBuilder;

protected:
	TArray<int32> _halfedges;
	TArray<int32> _triangles;
	TArray<FVector2D> _r_vertex;
	TArray<FVector2D> _t_vertex;
	TArray<int32> _r_in_s;

	FDualMesh Mesh;

public:
	int32 NumSides;
	int32 NumSolidSides;
	int32 NumRegions;
	int32 NumSolidRegions;
	int32 NumTriangles;
	int32 NumSolidTriangles;
	int32 NumBoundaryRegions;

public:
	// Static helpers

	// Alias for UDelaunayHelper::GetNextSide, for people coming from the original DualMesh API
	static int32 s_to_t(int32 s);
	static int32 s_next_s(int32 s);
	static int32 s_prev_s(int32 s);

protected:
	float r_x(int32 r) const;
	float r_y(int32 r) const;
	float t_x(int32 t) const;
	float t_y(int32 t) const;

	FVector2D r_pos(int32 r) const;
	FVector2D t_pos(int32 t) const;

	int32 s_begin_r(int32 s) const;
	int32 s_end_r(int32 s) const;

	int32 s_inner_t(int32 s) const;
	int32 s_outer_t(int32 s) const;

	int32 s_opposite_s(int32 s) const;

	TArray<int32> t_circulate_s(int32 t) const;
	TArray<int32> t_circulate_r(int32 t) const;
	TArray<int32> t_circulate_t(int32 t) const;

	TArray<int32> r_circulate_s(int32 r) const;
	TArray<int32> r_circulate_r(int32 r) const;
	TArray<int32> r_circulate_t(int32 r) const;

	int32 ghost_r() const;
	bool s_ghost(int32 s) const;
	bool r_ghost(int32 r) const;
	bool t_ghost(int32 t) const;

	bool s_boundary(int32 s) const;
	bool r_boundary(int32 r) const;


public:
	void InitializeMesh(const FDualMesh& Input, int32 BoundaryRegions);
};
