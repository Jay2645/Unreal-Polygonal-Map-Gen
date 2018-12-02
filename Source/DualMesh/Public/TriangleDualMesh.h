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
#include "DualMeshHelpers.h"
#include "TriangleDualMesh.generated.h"

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
	TArray<int32> RegionInSides;
	TArray<int32> HalfEdges;
	TArray<int32> Triangles;
	TArray<FVector2D> Vertices;

public:
	int32 NumSides;
	int32 NumSolidSides;
	int32 NumRegions;
	int32 NumSolidRegions;
	int32 NumTriangles;
	int32 NumSolidTriangles;
	int32 NumBoundaryRegions;

public:
	void InitializeMesh(const FDualMesh& Input);

/*
	r_x(r: number): number;
	r_y(r: number): number;
	t_x(t: number): number;
	t_y(t: number): number;
	r_pos(out: number[], r: number): number[];
	t_pos(out: number[], t: number): number[];

	s_begin_r(s: number): number;
	s_end_r(s: number): number;

	s_inner_t(s: number): number;
	s_outer_t(s: number): number;

	s_next_s(s: number): number;
	s_prev_s(s: number): number;

	s_opposite_s(s: number): number;

	t_circulate_s(out_s: number[], t: number): number[];
	t_circulate_r(out_s: number[], t: number): number[];
	t_circulate_t(out_s: number[], t: number): number[];
	r_circulate_s(out_s: number[], t: number): number[];
	r_circulate_r(out_s: number[], t: number): number[];
	r_circulate_t(out_s: number[], t: number): number[];

	ghost_r(): number;
	s_ghost(s: number): boolean;
	r_ghost(r: number): boolean;
	t_ghost(t: number): boolean;
	s_boundary(s: number): boolean;
	r_boundary(s: number): boolean;
*/
};
