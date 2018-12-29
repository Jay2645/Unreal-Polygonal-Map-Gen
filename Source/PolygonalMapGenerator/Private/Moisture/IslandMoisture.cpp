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

TSet<FPointIndex> UIslandMoisture::FindRiverbanks(UTriangleDualMesh* Mesh, const TArray<int32>& s_flow) const
{
	TSet<FPointIndex> banks;
	for (FSideIndex s = 0; s < Mesh->NumSolidSides; s++)
	{
		if (s_flow[s] > 0)
		{
			banks.Add(Mesh->s_begin_r(s));
			banks.Add(Mesh->s_end_r(s));
		}
	}
	return banks;
}

TSet<FPointIndex> UIslandMoisture::FindLakeshores(UTriangleDualMesh* Mesh, const TArray<bool>& r_ocean, const TArray<bool>& r_water) const
{
	TSet<FPointIndex> shores;
	for (FSideIndex s = 0; s < Mesh->NumSolidSides; s++)
	{
		FPointIndex r = Mesh->s_begin_r(s);
		if (r_water[r] && !r_ocean[r])
		{
			shores.Add(r);
			shores.Add(Mesh->s_end_r(s));
		}
	}
	return shores;
}

TSet<FPointIndex> UIslandMoisture::FindMoistureSeeds_Implementation(UTriangleDualMesh* Mesh, const TArray<int32>& s_flow, const TArray<bool>& r_ocean, const TArray<bool>& r_water) const
{
	TSet<FPointIndex> seeds;

	seeds.Append(FindRiverbanks(Mesh, s_flow));
	seeds.Append(FindLakeshores(Mesh, r_ocean, r_water));

	return seeds;
}

void UIslandMoisture::AssignRegionMoisture_Implementation(TArray<float>& r_moisture, TArray<int32>& r_waterdistance, UTriangleDualMesh* Mesh, const TArray<bool>& r_water, const TSet<FPointIndex>& seed_r) const
{
	r_moisture.Empty(Mesh->NumRegions);
	r_moisture.SetNumZeroed(Mesh->NumRegions);
	r_waterdistance.Empty(Mesh->NumRegions);
	r_waterdistance.SetNumZeroed(Mesh->NumRegions);
	for (FPointIndex r = 0; r < r_waterdistance.Num(); r++)
	{
		r_waterdistance[r] = -1;
	}

	TArray<FPointIndex> queue_r = seed_r.Array();

	// Set all freshwater regions to have distance 0 from water
	for (FPointIndex r : queue_r)
	{
		r_waterdistance[r] = 0;
	}

	int32 maxDistance = 1;

	while (queue_r.Num() > 0)
	{
		FPointIndex current_r = queue_r[0];
		queue_r.RemoveAt(0);
		TArray<FPointIndex> out_r = Mesh->r_circulate_r(current_r);
		for (FPointIndex neighbor_r : out_r)
		{
			if (!r_water[neighbor_r] && r_waterdistance[neighbor_r] == -1)
			{
				int32 newDistance = 1 + r_waterdistance[current_r];
				r_waterdistance[neighbor_r] = newDistance;
				if (newDistance > maxDistance) { maxDistance = newDistance; }
				queue_r.Add(neighbor_r);
			}
		}
	}

	// Actually set the moisture
	for (FPointIndex r = 0; r < r_waterdistance.Num(); r++)
	{
		r_moisture[r] = r_water[r] ? 1.0f : 1.0f - FMath::Pow((float)r_waterdistance[r] / maxDistance, 0.5f);
	}
}

void UIslandMoisture::RedistributeRegionMoisture_Implementation(TArray<float>& r_moisture, UTriangleDualMesh* Mesh, const TArray<bool>& r_water, float MinMoisture, float MaxMoisture) const
{
	TArray<FPointIndex> land_r;
	for (FPointIndex r = 0; r < Mesh->NumSolidRegions; r++)
	{
		if (!r_water[r])
		{
			land_r.Add(r);
		}
	}

	if (land_r.Num() <= 1)
	{
		return;
	}

	land_r.Sort([r_moisture](const FPointIndex& A, const FPointIndex& B)
	{
		return r_moisture[A] < r_moisture[B];
	});

	for (int i = 0; i < land_r.Num(); i++)
	{
		r_moisture[land_r[i]] = MinMoisture + (MaxMoisture - MinMoisture) * i / ((float)land_r.Num() - 1.0f);
	}
}

void UIslandMoisture::assign_r_moisture(TArray<float>& r_moisture, TArray<int32>& r_waterdistance, UTriangleDualMesh* Mesh, const TArray<bool>& r_water, const TSet<FPointIndex>& r_moisture_seeds) const
{
	AssignRegionMoisture(r_moisture, r_waterdistance, Mesh, r_water, r_moisture_seeds);
}

void UIslandMoisture::redistribute_r_moisture(TArray<float>& r_moisture, UTriangleDualMesh* Mesh, const TArray<bool>& r_water, float MinMoisture, float MaxMoisture) const
{
	RedistributeRegionMoisture(r_moisture, Mesh, r_water, MinMoisture, MaxMoisture);
}

TSet<FPointIndex> UIslandMoisture::find_moisture_seeds_r(UTriangleDualMesh* Mesh, const TArray<int32>& s_flow, const TArray<bool>& r_ocean, const TArray<bool>& r_water) const
{
	return FindMoistureSeeds(Mesh, s_flow, r_ocean, r_water);
}
