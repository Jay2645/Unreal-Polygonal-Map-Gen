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

#include "DualMeshHelpers.h"
#include "DelaunayHelper.h"

int32 UDualMeshHelpers::s_next_s(int32 s)
{
	return UDelaunayHelper::NextHalfEdge(s);
}

FDualMesh::FDualMesh(const TArray<FVector2D>& GivenPoints, float MaxMapSize)
	: FDelaunayMesh(GivenPoints)
{
	MaxSize = MaxMapSize;
	NumSolidSides = DelaunayTriangles.Num();
	AddGhostStructure();
}

void FDualMesh::AddGhostStructure()
{
	const int32 ghostRegion = Coordinates.Num();
	int32 numUnpairedSides = 0;
	int32 firstUnpairedEdge = -1;
	TArray<int32> regionsUnpairedSides;
	regionsUnpairedSides.SetNumZeroed(NumSolidSides);
	for (int i = 0; i < NumSolidSides; i++)
	{
		if (HalfEdges[i] == -1)
		{
			numUnpairedSides++;
			regionsUnpairedSides[DelaunayTriangles[i]] = i;
			firstUnpairedEdge = i;
		}
	}

	TArray<FVector2D> newVertices;
	newVertices.Append(Coordinates);
	newVertices.Add(FVector2D(MaxSize / 2.0f, MaxSize / 2.0f));
	
	TArray<int32> sideNewStartRegions;
	sideNewStartRegions.Append(DelaunayTriangles);
	sideNewStartRegions.SetNumZeroed(NumSolidSides + 3 * numUnpairedSides);

	TArray<int32> sideNewOppositeSides;
	sideNewOppositeSides.Append(HalfEdges);
	sideNewOppositeSides.SetNumZeroed(NumSolidSides + 3 * numUnpairedSides);

	int s = firstUnpairedEdge;
	for (int i = 0; i < numUnpairedSides; i++)
	{
		// Construct a ghost side for s
		int32 ghostSide = NumSolidSides + 3 * i;
		sideNewOppositeSides[s] = ghostSide;
		sideNewOppositeSides[ghostSide] = s;
		sideNewStartRegions[ghostSide] = sideNewStartRegions[UDualMeshHelpers::s_next_s(s)];

		// Construct the rest of the ghost triangle
		sideNewStartRegions[ghostSide + 1] = sideNewStartRegions[s];
		sideNewStartRegions[ghostSide + 2] = ghostRegion;

		int k = NumSolidSides + (3 * i + 4) % (3 * numUnpairedSides);
		sideNewOppositeSides[ghostSide + 2] = k;
		sideNewOppositeSides[k] = ghostSide + 2;

		s = regionsUnpairedSides[sideNewStartRegions[UDualMeshHelpers::s_next_s(s)]];
	}

	// Put the new arrays back into this structure
	Coordinates = newVertices;
	DelaunayTriangles = sideNewStartRegions;
	HalfEdges = sideNewOppositeSides;
}