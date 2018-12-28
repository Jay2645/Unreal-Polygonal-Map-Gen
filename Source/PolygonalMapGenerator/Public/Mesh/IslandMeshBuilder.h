/*
* From http://www.redblobgames.com/maps/mapgen2/
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
*/

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "TriangleDualMesh.h"

#include "IslandMeshBuilder.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class POLYGONALMAPGENERATOR_API UIslandMeshBuilder : public UDataAsset
{
	GENERATED_BODY()

public:
	// The size of our map, starting at (0, 0).
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Map")
	FVector2D MapSize;
	// The amount of spacing on the edge of the map.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Edges", meta = (ClampMin = "0"))
	int32 BoundarySpacing;

public:
	UIslandMeshBuilder();

protected:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Procedural Generation|Island Generation|Points")
	void AddPoints(UDualMeshBuilder* Builder, UPARAM(ref) FRandomStream& Rng) const;
	virtual void AddPoints_Implementation(UDualMeshBuilder* Builder, FRandomStream& Rng) const;
	virtual UTriangleDualMesh* GenerateDualMesh_Implementation(FRandomStream& Rng) const;

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Procedural Generation|Island Generation|Points")
	UTriangleDualMesh* GenerateDualMesh(UPARAM(ref) FRandomStream& Rng) const;
};
