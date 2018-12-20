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
* Generates a random triangle mesh for the given area.
*
* Intended for use in Unreal Engine 4. Use the NewObject function to create a
* pointer to this object, set up the max size, boundaries, and points, then
* call Create() to return a UTriangleDualMesh object to work with.
*/

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TriangleDualMesh.h"
#include "DualMeshBuilder.generated.h"

/**
* Generates a random triangle mesh for the given area.
 */
UCLASS()
class DUALMESH_API UDualMeshBuilder : public UObject
{
	GENERATED_BODY()
protected:
	TArray<FVector2D> Points;
	int32 NumBoundaryRegions;
	FVector2D MaxMeshSize;

protected:
	TArray<FVector2D> AddBoundaryPoints(int32 Spacing, const FVector2D& Size);

public:
	UDualMeshBuilder();

public:
	void Initialize(const FVector2D& MaxSize, int32 BoundarySpacing = 0);
	void AddPoint(const FVector2D& Point);
	void AddPoints(const TArray<FVector2D>& NewPoints);
	TArray<FVector2D> GetBoundaryPoints() const;
	void ClearNonBoundaryPoints();
	void AddPoisson(FRandomStream& Rng, FVector2D MapOffset = FVector2D(0.0f, 0.0f), float Spacing = 1.0f, int32 MaxStepSamples = 30);

	UTriangleDualMesh* Create();
};
