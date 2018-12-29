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
#include "IslandMoisture.generated.h"

/**
 * A class which determines how much moisture different areas on the island receive.
 */
UCLASS(Blueprintable)
class POLYGONALMAPGENERATOR_API UIslandMoisture : public UDataAsset
{
	GENERATED_BODY()

protected:
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "Procedural Generation|Island Generation|Moisture")
	virtual TSet<FPointIndex> FindRiverbanks(UTriangleDualMesh* Mesh, const TArray<int32>& s_flow) const;
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "Procedural Generation|Island Generation|Moisture")
	virtual TSet<FPointIndex> FindLakeshores(UTriangleDualMesh* Mesh, const TArray<bool>& r_ocean, const TArray<bool>& r_water) const;

	virtual TSet<FPointIndex> FindMoistureSeeds_Implementation(UTriangleDualMesh* Mesh, const TArray<int32>& s_flow, const TArray<bool>& r_ocean, const TArray<bool>& r_water) const;
	virtual void AssignRegionMoisture_Implementation(TArray<float>& r_moisture, TArray<int32>& r_waterdistance, UTriangleDualMesh* Mesh, const TArray<bool>& r_water, const TSet<FPointIndex>& r_moisture_seeds) const;
	virtual void RedistributeRegionMoisture_Implementation(TArray<float>& r_moisture, UTriangleDualMesh* Mesh, const TArray<bool>& r_water, float MinMoisture, float MaxMoisture) const;

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Procedural Generation|Island Generation|Moisture")
	TSet<FPointIndex> FindMoistureSeeds(UTriangleDualMesh* Mesh, const TArray<int32>& SideFlow, const TArray<bool>& OceanRegions, const TArray<bool>& WaterRegions) const;
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Procedural Generation|Island Generation|Moisture")
	void AssignRegionMoisture(UPARAM(ref) TArray<float>& RegionMoisture, UPARAM(ref) TArray<int32>& RegionWaterDistance, UTriangleDualMesh* Mesh, const TArray<bool>& WaterRegions, const TSet<FPointIndex>& MoistureSeedRegions) const;
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Procedural Generation|Island Generation|Moisture")
	void RedistributeRegionMoisture(UPARAM(ref) TArray<float>& RegionMoisture, UTriangleDualMesh* Mesh, const TArray<bool>& WaterRegions, float MinMoisture, float MaxMoisture) const;
	
	TSet<FPointIndex> find_moisture_seeds_r(UTriangleDualMesh* Mesh, const TArray<int32>& s_flow, const TArray<bool>& r_ocean, const TArray<bool>& r_water) const;
	void assign_r_moisture(TArray<float>& r_moisture, TArray<int32>& r_waterdistance, UTriangleDualMesh* Mesh, const TArray<bool>& r_water, const TSet<FPointIndex>& r_moisture_seeds) const;
	void redistribute_r_moisture(TArray<float>& r_moisture, UTriangleDualMesh* Mesh, const TArray<bool>& r_water, float MinMoisture, float MaxMoisture) const;
};
