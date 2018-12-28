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

#include "IslandMoisture.h"

TSet<int32> UIslandMoisture::FindMoistureSeeds_Implementation(UTriangleDualMesh* Mesh, const TArray<int32>& s_flow, const TArray<bool>& r_ocean, const TArray<bool>& r_water) const
{
	unimplemented();
	return TSet<int32>();
}

void UIslandMoisture::AssignRegionMoisture_Implementation(TArray<int32>& r_moisture, TArray<int32>& r_waterdistance, UTriangleDualMesh* Mesh, const TArray<bool>& r_water, const TSet<int32>& r_moisture_seeds) const
{
	unimplemented();
}

void UIslandMoisture::RedistributeRegionMoisture_Implementation(TArray<int32>& r_moisture, UTriangleDualMesh* Mesh, const TArray<bool>& r_water, float MinMoisture, float MaxMoisture) const
{
	unimplemented();
}

void UIslandMoisture::assign_r_moisture(TArray<int32>& r_moisture, TArray<int32>& r_waterdistance, UTriangleDualMesh* Mesh, const TArray<bool>& r_water, const TSet<int32>& r_moisture_seeds) const
{
	AssignRegionMoisture(r_moisture, r_waterdistance, Mesh, r_water, r_moisture_seeds);
}

void UIslandMoisture::redistribute_r_moisture(TArray<int32>& r_moisture, UTriangleDualMesh* Mesh, const TArray<bool>& r_water, float MinMoisture, float MaxMoisture) const
{
	RedistributeRegionMoisture(r_moisture, Mesh, r_water, MinMoisture, MaxMoisture);
}

TSet<int32> UIslandMoisture::find_moisture_seeds_r(UTriangleDualMesh* Mesh, const TArray<int32>& s_flow, const TArray<bool>& r_ocean, const TArray<bool>& r_water) const
{
	return FindMoistureSeeds(Mesh, s_flow, r_ocean, r_water);
}
