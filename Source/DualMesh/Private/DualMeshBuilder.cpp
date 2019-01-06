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

#include "DualMeshBuilder.h"
#include "Delaunator/Public/DelaunayHelper.h"
#include "RandomSampling/PoissonDiscUtilities.h"


TArray<FVector2D> UDualMeshBuilder::AddBoundaryPoints(int32 Spacing, const FVector2D& Size)
{
	TSet<FVector2D> points;

	// If we have no spacing, return an empty array of points
	if (Spacing <= 0)
	{
		return points.Array();
	}

	/*
	* Add vertices evenly along the boundary of the mesh;
	* use a slight curve so that the Delaunay triangulation
	* doesn't make long thin triangles along the boundary.
	* These points also prevent the Poisson disc generator
	* from making uneven points near the boundary.
	*/
	int32 x = FMath::CeilToInt(Size.X / Spacing);
	for (int xi = 0; xi < x; xi++)
	{
		float t = ((float)xi + 0.5f) / ((float)x + 1.0f);
		float w = Size.X * t;
		float offset = FMath::Pow(t - 0.5f, 2.0f);
		points.Add(FVector2D(offset, w));
		points.Add(FVector2D(Size.X - offset, w));
		points.Add(FVector2D(w, offset));
		points.Add(FVector2D(w, Size.X - offset));
	}
	// Now do the same for Y
	int32 y = FMath::CeilToInt(Size.Y / Spacing);
	for (int yi = 0; yi < y; yi++)
	{
		float t = ((float)yi + 0.5f) / ((float)y + 1.0f);
		float w = Size.Y * t;
		float offset = FMath::Pow(t - 0.5f, 2.0f);
		points.Add(FVector2D(offset, w));
		points.Add(FVector2D(Size.Y - offset, w));
		points.Add(FVector2D(w, offset));
		points.Add(FVector2D(w, Size.Y - offset));
	}
	return points.Array();
}

UDualMeshBuilder::UDualMeshBuilder()
{
	NumBoundaryRegions = -1;
	MaxMeshSize = FVector2D::ZeroVector;
}

void UDualMeshBuilder::Initialize(const FVector2D& MaxSize, int32 BoundarySpacing /*= 0*/)
{
	MaxMeshSize = MaxSize;
	Points = AddBoundaryPoints(BoundarySpacing, MaxMeshSize);
	NumBoundaryRegions = Points.Num();
}

void UDualMeshBuilder::AddPoint(const FVector2D& Point)
{
	Points.Add(Point);
}

void UDualMeshBuilder::AddPoints(const TArray<FVector2D>& NewPoints)
{
	Points.Append(NewPoints);
}

TArray<FVector2D> UDualMeshBuilder::GetBoundaryPoints() const
{
	TArray<FVector2D> boundaryPoints;
	for (int i = 0; i < NumBoundaryRegions; i++)
	{
		boundaryPoints.Add(Points[i]);
	}
	return boundaryPoints;
}

void UDualMeshBuilder::ClearNonBoundaryPoints()
{
	Points.SetNum(NumBoundaryRegions);
}

void UDualMeshBuilder::AddPoisson(FRandomStream& Rng, FVector2D MapOffset, float Spacing, int32 MaxStepSamples)
{
	UPoissonDiscUtilities::Distribute2D(Points, Rng.GetCurrentSeed(), MaxMeshSize - MapOffset, MapOffset * 0.5f, Spacing, MaxStepSamples);
	Rng.GetFraction(); // Generates the next seed
}

UTriangleDualMesh* UDualMeshBuilder::Create()
{
	if (NumBoundaryRegions == -1)
	{
		UE_LOG(LogDualMesh, Error, TEXT("Dual mesh's attributes were not set. Initialize before trying to create a DualMesh."));
		return NULL;
	}

	UTriangleDualMesh* mesh = NewObject<UTriangleDualMesh>();
	check(mesh);
	
	FDualMesh dualMesh = FDualMesh(Points, MaxMeshSize);
	mesh->InitializeMesh(dualMesh, NumBoundaryRegions);
	
	return mesh;
}
