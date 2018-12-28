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

#include "IslandRivers.h"

UIslandRivers::UIslandRivers()
{
	MinSpringElevation = 0.3f;
	MaxSpringElevation = 0.9f;
}

bool UIslandRivers::IsTriangleWater(FTriangleIndex t, UTriangleDualMesh* Mesh, const TArray<bool>& r_water) const
{
	TArray<FPointIndex> regions = Mesh->t_circulate_r(t);
	for (FPointIndex r : regions)
	{
		if (r_water[r])
		{
			return true;
		}
	}
	return false;
}

TArray<FTriangleIndex> UIslandRivers::FindSpringTriangles_Implementation(UTriangleDualMesh* Mesh, const TArray<bool>& r_water, const TArray<float>& t_elevation, const TArray<FSideIndex>& t_downslope_s) const
{
	TSet<FTriangleIndex> spring_t;
	if (Mesh != NULL)
	{
		// Add everything above some elevation, but not lakes
		for (FTriangleIndex t = 0; t < Mesh->NumSolidTriangles; t++)
		{
			if (t_elevation[t] >= MinSpringElevation &&
				t_elevation[t] <= MaxSpringElevation &&
				!IsTriangleWater(t, Mesh, r_water))
			{
				spring_t.Add(t);
			}
		}
	}
	else
	{
		UE_LOG(LogMapGen, Error, TEXT("Mesh was invalid!"));
	}
	return spring_t.Array();
}

void UIslandRivers::AssignSideFlow_Implementation(TArray<int32>& s_flow, UTriangleDualMesh* Mesh, const TArray<FSideIndex>& t_downslope_s, const TArray<FTriangleIndex>& river_t) const
{
	if (Mesh)
	{
		// Each river in river_t contributes 1 flow down to the coastline
		s_flow.Empty(Mesh->NumSides);
		s_flow.SetNumZeroed(Mesh->NumSides);
		for (int i = 0; i < river_t.Num(); i++)
		{
			FTriangleIndex t = river_t[i];
			TSet<FTriangleIndex> processedSlopes;
			for (;;)
			{
				FSideIndex s = t_downslope_s[t];

				if (!s.IsValid())
				{
					break;
				}

				s_flow[s]++;

				FTriangleIndex next_t = Mesh->s_outer_t(s);
				if (next_t == t)
				{
					break;
				}
				if (processedSlopes.Contains(t))
				{
					UE_LOG(LogMapGen, Warning, TEXT("Tried to process a slope we've already processed once this loop! We have an infinite loop."));
					break;
				}
				processedSlopes.Add(t);

				if (processedSlopes.Num() > 100)
				{
					UE_LOG(LogMapGen, Error, TEXT("Processed too many slopes!"));
					return;
				}

				t = next_t;
			}
		}
	}
	else
	{
		UE_LOG(LogMapGen, Error, TEXT("Mesh was invalid!"));
	}
}

TArray<FTriangleIndex> UIslandRivers::find_spring_t(UTriangleDualMesh* Mesh, const TArray<bool>& r_water, const TArray<float>& t_elevation, const TArray<FSideIndex>& t_downslope_s) const
{
	return FindSpringTriangles(Mesh, r_water, t_elevation, t_downslope_s);
}

void UIslandRivers::assign_s_flow(TArray<int32>& s_flow, UTriangleDualMesh* Mesh, const TArray<FSideIndex>& t_downslope_s, const TArray<FTriangleIndex>& river_t) const
{
	AssignSideFlow(s_flow, Mesh, t_downslope_s, river_t);
}