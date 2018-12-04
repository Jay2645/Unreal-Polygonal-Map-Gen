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
#include "Biome.generated.h"

/**
 * 
 */
UCLASS()
class POLYGONALMAPGENERATOR_API UBiome : public UDataAsset
{
	GENERATED_BODY()
	
public:
	void assign_r_coast(TArray<int32>& r_coast, UTriangleDualMesh* Mesh, const TArray<int32>& r_ocean) const;
	void assign_r_temperature(TArray<int32>& r_temperature, UTriangleDualMesh* Mesh, const TArray<int32>& r_ocean, const TArray<int32>& r_water, const TArray<int32>& r_elevation, const TArray<int32>& r_moisture, float NorthernTemperature, float SouthernTemperature) const;
	void assign_r_biome(TArray<FName>& r_biome, UTriangleDualMesh* Mesh, const TArray<int32>& r_ocean, const TArray<int32>& r_water, const TArray<int32>& r_coast, const TArray<int32>& r_temperature, const TArray<int32>& r_moisture) const;
};
