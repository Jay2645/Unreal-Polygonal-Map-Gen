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

#include "IslandWater.h"
#include "RandomSampling/SimplexNoise.h"

UIslandWater::UIslandWater()
{
	WaterCutoff = 0.0f;
	bInvertLandAndWater = false;
}

void UIslandWater::AssignOcean_Implementation(TArray<bool>& r_ocean, UTriangleDualMesh* Mesh, const TArray<bool>& r_water) const
{
	/* A region is ocean if it is a water region connected to the ghost region,
	which is outside the boundary of the map; this could be any seed set but
	for islands, the ghost region is a good seed */
	r_ocean.Empty(Mesh->NumRegions);
	r_ocean.SetNumZeroed(Mesh->NumRegions);
	TArray<FPointIndex> stack = { Mesh->ghost_r() };
	r_ocean[stack[0]] = true;

	while (stack.Num() > 0)
	{
		FPointIndex r1 = stack.Pop();
		check(r1.IsValid());
		TArray<FPointIndex> r_out = Mesh->r_circulate_r(r1);
		for (FPointIndex r2 : r_out)
		{
			if (!r2.IsValid())
			{
				continue;
			}
			if (r_water[r2] && !r_ocean[r2])
			{
				r_ocean[r2] = true;
				stack.Add(r2);
			}
		}
	}

#if !UE_BUILD_SHIPPING
	int32 oceanTileCount = 0;
	for (int i = 0; i < r_ocean.Num(); i++)
	{
		if (r_ocean[i])
		{
			oceanTileCount++;
		}
	}
	if (oceanTileCount == 0)
	{
		UE_LOG(LogMapGen, Error, TEXT("Did not generate any ocean tiles!"));
	}
	else
	{
		UE_LOG(LogMapGen, Log, TEXT("Generated %d ocean tiles out of %d total."), oceanTileCount, r_ocean.Num());
	}
#endif
}

void UIslandWater::AssignWater_Implementation(TArray<bool>& r_water, FRandomStream& Rng, UTriangleDualMesh* Mesh, const FIslandShape& Shape) const
{
	if (Mesh)
	{
#if !UE_BUILD_SHIPPING
		int32 count = 0;
#endif
		/* A region is water if the noise value is low */
		r_water.Empty(Mesh->NumRegions);
		r_water.SetNumZeroed(Mesh->NumRegions);

		InitializeWater(r_water, Mesh, Rng);

		FVector2D meshSize = Mesh->GetSize() * 0.5f;
		FVector2D offset = FVector2D(Rng.FRandRange(-meshSize.X, meshSize.X), Rng.FRandRange(-meshSize.Y, meshSize.Y));
		for (FPointIndex r = 0; r < r_water.Num(); r++)
		{
			if (Mesh->r_ghost(r) || Mesh->r_boundary(r))
			{
				r_water[r] = true;
			}
			else
			{
				r_water[r] = IsPointLand(r, Mesh, meshSize, offset, Shape);
				if (bInvertLandAndWater)
				{
					r_water[r] = !r_water[r];
				}
			}
#if !UE_BUILD_SHIPPING
			if (r_water[r])
			{
				count++;
			}
#endif
		}

#if !UE_BUILD_SHIPPING
		UE_LOG(LogMapGen, Log, TEXT("Generated %d water regions out of %d total regions."), count, r_water.Num());
#endif
	}
	else
	{
		UE_LOG(LogMapGen, Error, TEXT("Mesh was invalid!"));
	}
}

void UIslandWater::InitializeWater_Implementation(TArray<bool>& r_water, UTriangleDualMesh* Mesh, FRandomStream& Rng) const
{
	// Empty
}

bool UIslandWater::IsPointLand_Implementation(FPointIndex Point, UTriangleDualMesh* Mesh, const FVector2D& HalfMeshSize, const FVector2D& Offset, const FIslandShape& Shape) const
{
	return false;
}

void UIslandWater::assign_r_water(TArray<bool>& r_water, FRandomStream& Rng, UTriangleDualMesh* Mesh, const FIslandShape& Shape) const
{
	AssignWater(r_water, Rng, Mesh, Shape);
}

void UIslandWater::assign_r_ocean(TArray<bool>& r_ocean, UTriangleDualMesh* Mesh, const TArray<bool>& r_water) const
{
	AssignOcean(r_ocean, Mesh, r_water);
}
