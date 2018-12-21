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

#include "Water.h"
#include "RandomSampling/SimplexNoise.h"

UWater::UWater()
{
	WaterCutoff = 0.0f;
}

void UWater::assign_r_ocean(TArray<bool>& r_ocean, UTriangleDualMesh* Mesh, const TArray<bool>& r_water) const
{
	/* A region is ocean if it is a water region connected to the ghost region,
	which is outside the boundary of the map; this could be any seed set but
	for islands, the ghost region is a good seed */
	r_ocean.Empty(Mesh->NumRegions);
	r_ocean.SetNumZeroed(Mesh->NumRegions);
	TArray<FPointIndex> stack = { Mesh->ghost_r() };
	TArray<FPointIndex> r_out;
	while (stack.Num() > 0)
	{
		FPointIndex r1 = stack.Pop();
		check(r1.IsValid());
		r_out = Mesh->r_circulate_r(r1);
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
	bool bFoundOceanTile = false;
	for (int i = 0; i < r_ocean.Num(); i++)
	{
		if (r_ocean[i])
		{
			bFoundOceanTile = true;
			break;
		}
	}
	if (!bFoundOceanTile)
	{
		UE_LOG(LogMapGen, Error, TEXT("Did not generate any ocean tiles!"));
	}
#endif
}

void UWater::assign_r_water(TArray<bool>& r_water, FRandomStream& Rng, UTriangleDualMesh* Mesh, const FIslandShape& Shape) const
{
	if (Mesh)
	{
#if !UE_BUILD_SHIPPING
		int32 count = 0;
#endif
		/* A region is water if the noise value is low */
		r_water.Empty(Mesh->NumRegions);
		r_water.SetNumZeroed(Mesh->NumRegions);
		FVector2D meshSize = Mesh->GetSize() * 0.5f;
		FVector2D offset = FVector2D(Rng.FRandRange(-100000.0f, 100000.0f), Rng.FRandRange(-100000.0f, 100000.0f));
		for (int r = 0; r < r_water.Num(); r++)
		{
			if (Mesh->r_ghost(r) || Mesh->r_boundary(r))
			{
				r_water[r] = true;
			}
			else
			{
				FVector2D nVector = Mesh->r_pos(r) * 0.5f;
				nVector.X /= meshSize.X;
				nVector.Y /= meshSize.Y;
				float n = UIslandMapUtils::FBMNoise(Shape.Amplitudes, nVector + offset);
				float distance = FMath::Max(FMath::Abs(nVector.X), FMath::Abs(nVector.Y));
				float lerpedN = FMath::Lerp(n, 0.5f, Shape.Round);
				float modifiedN = lerpedN - (1.0f - Shape.Inflate);
				r_water[r] = modifiedN * distance * distance < WaterCutoff;
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
