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
#include "PolygonalMapGenerator.h"
#include "TriangleDualMesh.h"
#include "IslandRivers.generated.h"

/**
 * A class which determines where "springs" are on the island, and then
 * generates rivers which flow downstream from these springs.
 */
UCLASS(Blueprintable)
class POLYGONALMAPGENERATOR_API UIslandRivers : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinSpringElevation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MaxSpringElevation;

public:
	UIslandRivers();

protected:
	/**
	* Is this triangle water?
	*/
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "Procedural Generation|Island Generation|Rivers")
	virtual bool IsTriangleWater(FTriangleIndex t, UTriangleDualMesh* Mesh, const TArray<bool>& WaterRegions) const;

	virtual TArray<FTriangleIndex> FindSpringTriangles_Implementation(UTriangleDualMesh* Mesh, const TArray<bool>& r_water, const TArray<float>& t_elevation, const TArray<FSideIndex>& t_downslope_s) const;
	virtual void AssignSideFlow_Implementation(TArray<int32>& s_flow, UTriangleDualMesh* Mesh, const TArray<FSideIndex>& t_downslope_s, const TArray<FTriangleIndex>& river_t) const;

public:
	/**
	* Find candidates for river sources
	*
	* Unlike the assign_* functions this does not write into an existing array
	*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Procedural Generation|Island Generation|Rivers")
	TArray<FTriangleIndex> FindSpringTriangles(UTriangleDualMesh* Mesh, const TArray<bool>& WaterRegions, const TArray<float>& TriangleElevations, const TArray<FSideIndex>& TriangleSideDownslopes) const;
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Procedural Generation|Island Generation|Rivers")
	void AssignSideFlow(UPARAM(ref) TArray<int32>& SideFlow, UTriangleDualMesh* Mesh, const TArray<FSideIndex>& TriangleSideDownslopes, const TArray<FTriangleIndex>& RiverTriangles) const;

	TArray<FTriangleIndex> find_spring_t(UTriangleDualMesh* Mesh, const TArray<bool>& r_water, const TArray<float>& t_elevation, const TArray<FSideIndex>& t_downslope_s) const;
	void assign_s_flow(TArray<int32>& s_flow, UTriangleDualMesh* Mesh, const TArray<FSideIndex>& t_downslope_s, const TArray<FTriangleIndex>& river_t) const;
};
