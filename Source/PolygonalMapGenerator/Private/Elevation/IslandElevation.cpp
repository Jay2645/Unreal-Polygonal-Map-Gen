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
#include "IslandElevation.h"

TArray<FTriangleIndex> UIslandElevation::FindCoastTriangles(UTriangleDualMesh* Mesh, const TArray<bool>& r_ocean) const
{
	TSet<FTriangleIndex> coasts_t;
	for (FSideIndex s = 0; s < Mesh->NumSides; s++)
	{
		// Get the points at the start and end of each side
		FPointIndex r0 = Mesh->s_begin_r(s);
		FPointIndex r1 = Mesh->s_end_r(s);
		// Something is a coast if it's an ocean on one side and not on the other
		if (r_ocean[r0] && !r_ocean[r1])
		{
			// It might seem that we also need to check !r_ocean[r0] && r_ocean[r1]
			// and it might seem that we have to add both t and its opposite but
			// each t vertex shows up in *four* directed sides, so we only have to test
			// one fourth of those conditions to get the vertex in the list once.
			coasts_t.Add(Mesh->s_inner_t(s));
		}
	}
	return coasts_t.Array();
}

bool UIslandElevation::IsTriangleOcean(FTriangleIndex t, UTriangleDualMesh* Mesh, const TArray<bool>& r_ocean) const
{
	TArray<FPointIndex> trianglePoints = Mesh->t_circulate_r(t);
	int count = 0;
	for (FPointIndex r : trianglePoints)
	{
		if (r_ocean[r])
		{
			count++;
		}
	}
	// True if 2 or more points of the triangle are ocean
	return count >= 2;
}

bool UIslandElevation::IsRegionLake(FPointIndex r, const TArray<bool>& r_water, const TArray<bool>& r_ocean) const
{
	return r_water[r] && !r_ocean[r];
}

bool UIslandElevation::IsSideLake(FSideIndex s, UTriangleDualMesh* Mesh, const TArray<bool>& r_water, const TArray<bool>& r_ocean) const
{
	return IsRegionLake(Mesh->s_begin_r(s), r_water, r_ocean) || IsRegionLake(Mesh->s_end_r(s), r_water, r_ocean);
}

void UIslandElevation::DistributeElevations(TArray<float> &t_elevation, UTriangleDualMesh* Mesh, const TArray<int32> &t_coastdistance, const TArray<bool>& r_ocean, int32 MinDistance, int32 MaxDistance) const
{
	// We initially base elevation on distance from a coast
	for (FTriangleIndex t = 0; t < t_coastdistance.Num(); t++)
	{
		float d = (float)t_coastdistance[t];
		// Ocean values scale linearly down, so they're "upside-down mountains"
		if (IsTriangleOcean(t, Mesh, r_ocean))
		{
			t_elevation[t] = -d / (float)MinDistance;
		}
		else
		{
			t_elevation[t] = d / (float)MaxDistance;
		}
	}
}

void UIslandElevation::UpdateCoastDistance(TArray<int32> &t_coastdistance, UTriangleDualMesh* Mesh, FTriangleIndex Triangle, int32 Distance) const
{
	// Update the coast distance array to make sure we're still pointing to the nearest coast
	t_coastdistance[Triangle] = Distance;
	TArray<FSideIndex> out_s = Mesh->t_circulate_s(Triangle);
	for (int i = 0; i < out_s.Num(); i++)
	{
		FSideIndex s = out_s[i];
		FTriangleIndex neighbor_t = Mesh->s_outer_t(s);
		if (t_coastdistance[neighbor_t] > Distance + 1)
		{
			UpdateCoastDistance(t_coastdistance, Mesh, neighbor_t, Distance + 1);
		}
	}
}

void UIslandElevation::AssignTriangleElevations_Implementation(TArray<float>& t_elevation, TArray<int32>& t_coastdistance, TArray<FSideIndex>& t_downslope_s, UTriangleDualMesh* Mesh, const TArray<bool>& r_ocean, const TArray<bool>& r_water, FRandomStream& DrainageRng) const
{
	// TODO: this messes up lakes, as they will no longer all be at the same elevation

	// Initialize all triangles to be -1 triangles away from the nearest coast
	t_coastdistance.Empty(Mesh->NumTriangles);
	t_coastdistance.SetNumZeroed(Mesh->NumTriangles);
	for (int i = 0; i < t_coastdistance.Num(); i++)
	{
		t_coastdistance[i] = -1;
	}
	// Initialize all downslopes to point to an invalid index
	t_downslope_s.Empty(Mesh->NumTriangles);
	t_downslope_s.SetNumZeroed(Mesh->NumTriangles);
	for (int i = 0; i < t_downslope_s.Num(); i++)
	{
		t_downslope_s[i] = FSideIndex();
	}

	// Reset the elevation arrays to 0
	t_elevation.Empty(Mesh->NumTriangles);
	t_elevation.SetNumZeroed(Mesh->NumTriangles);

	// Find all coasts and set them to be 0 distance away from the nearest coast
	TArray<FTriangleIndex> queue_t = FindCoastTriangles(Mesh, r_ocean);
	if (queue_t.Num() == 0)
	{
		UE_LOG(LogMapGen, Error, TEXT("No triangles were marked as coast!"));
		return;
	}

	for (int t = 0; t < queue_t.Num(); t++)
	{
		t_coastdistance[queue_t[t]] = 0;
	}

	// Distance underwater to nearest shore
	int32 minDistance = 1;
	// Distance overland to nearest shore
	int32 maxDistance = 1;
	while (queue_t.Num() > 0)
	{
		// Get the next triangle and pop it from the queue
		FTriangleIndex current_t = queue_t[0];
		queue_t.RemoveAt(0);
		// Find all sides of the current triangle
		TArray<FSideIndex> out_s = Mesh->t_circulate_s(current_t);

		// Iterate over each side of the triangle, starting from a random offset
		int32 iOffset = DrainageRng.RandRange(0, out_s.Num() - 1);
		for (int i = 0; i < out_s.Num(); i++)
		{
			// Get the index of the side we're working on
			FSideIndex s = out_s[(i + iOffset) % out_s.Num()];
			// Check to see if this side is a lake
			// If it is, keep the distance from the nearest coast the same (to ensure that lakes keep elevation)
			// If it isn't, increment the distance from the nearest coast
			bool lake = IsSideLake(s, Mesh, r_water, r_ocean);
			int32 newDistance = (lake ? 0 : 1) + t_coastdistance[current_t];

			// Get the next triangle down the line
			FTriangleIndex neighbor_t = Mesh->s_outer_t(s);
			// Check if this next triangle has not had its distance set yet
			// or if it's closer to a coast than it thought it was
			if (t_coastdistance[neighbor_t] == -1 || newDistance < t_coastdistance[neighbor_t])
			{
				// Point it "downhill" to the next side
				t_downslope_s[neighbor_t] = Mesh->s_opposite_s(s);

				UpdateCoastDistance(t_coastdistance, Mesh, neighbor_t, newDistance);

				// If this tile is ocean, see if we need to update how far away this underwater tile 
				// is from a coast
				if (IsTriangleOcean(neighbor_t, Mesh, r_ocean) && newDistance > minDistance) { minDistance = newDistance; }
				// If this tile is land, see if we need to update how far away this land tile is from a coast
				else if (!IsTriangleOcean(neighbor_t, Mesh, r_ocean) && newDistance > maxDistance) { maxDistance = newDistance; }

				if (lake)
				{
					// If we're a lake, make sure we're processed next
					queue_t.Insert(neighbor_t, 0);
				}
				else
				{
					// Otherwise, add us to the end of the queue
					queue_t.Add(neighbor_t);
				}
			}
		}

		if (queue_t.Num() == 0)
		{
			for (FTriangleIndex t = 0; t < t_coastdistance.Num(); t++)
			{
				if (t_coastdistance[t] == -1)
				{
					UE_LOG(LogMapGen, Warning, TEXT("Found unitialized coast distance triangle: %d"), t);
				}
			}
		}
	}

	// Now to distribute the actual elevations
	DistributeElevations(t_elevation, Mesh, t_coastdistance, r_ocean, minDistance, maxDistance);
}

void UIslandElevation::RedistributeTriangleElevations_Implementation(TArray<float>& t_elevation, UTriangleDualMesh* Mesh, const TArray<bool>& r_ocean) const
{
	// SCALE_FACTOR increases the mountain area. At 1.0 the maximum
	// elevation barely shows up on the map, so we set it to 1.1.
	const float SCALE_FACTOR = 1.1f;

	TArray<FTriangleIndex> nonocean_t;
	for (FTriangleIndex t = 0; t < t_elevation.Num(); t++)
	{
		if (!IsTriangleOcean(t, Mesh, r_ocean))
		{
			nonocean_t.Add(t);
		}
	}

	nonocean_t.Sort([t_elevation](const FTriangleIndex& A, const FTriangleIndex& B)
	{
		return t_elevation[A] < t_elevation[B];
	});

	if (nonocean_t.Num() > 0)
	{
		UE_LOG(LogMapGen, Log, TEXT("Sorted non-ocean bottom value: %f. Sorted non-ocean top value: %f"), t_elevation[nonocean_t[0]], t_elevation[nonocean_t[nonocean_t.Num() - 1]]);
	}

	for (int i = 0; i < nonocean_t.Num(); i++)
	{
		// Let y(x) be the total area that we want at elevation <= x.
		// We want the higher elevations to occur less than lower
		// ones, and set the area to be y(x) = 1 - (1-x)^2.
		float y = i / (nonocean_t.Num() - 1.0f);
		// Now we have to solve for x, given the known y.
		//  *  y = 1 - (1-x)^2
		//  *  y = 1 - (1 - 2x + x^2)
		//  *  y = 2x - x^2
		//  *  x^2 - 2x + y = 0
		// From this we can use the quadratic equation to get:
		float x = FMath::Sqrt(SCALE_FACTOR) - FMath::Sqrt(SCALE_FACTOR*(1 - y));
		if (x > 1.0)
		{
			x = 1.0;
		}
		//UE_LOG(LogMapGen, Log, TEXT("Redistributing elevation for triangle %d for %f to %f."), nonocean_t[i], t_elevation[nonocean_t[i]], x);
		t_elevation[nonocean_t[i]] = x;
	}
}

void UIslandElevation::AssignRegionElevations_Implementation(TArray<float>& r_elevation, UTriangleDualMesh* Mesh, const TArray<float>& t_elevation, const TArray<bool>& r_ocean) const
{
	const float max_ocean_elevation = -0.01;

	r_elevation.Empty(Mesh->NumRegions);
	r_elevation.SetNumZeroed(Mesh->NumRegions);

	TArray<FTriangleIndex> out_t;
	for (FPointIndex r = 0; r < Mesh->NumRegions; r++)
	{
		out_t = Mesh->r_circulate_t(r);
		float elevation = 0.0f;
		for (FTriangleIndex t : out_t)
		{
			elevation += t_elevation[t];
		}

		r_elevation[r] = elevation / out_t.Num();
		if (r_ocean[r] && r_elevation[r] > max_ocean_elevation)
		{
			r_elevation[r] = max_ocean_elevation;
		}
	}
}

void UIslandElevation::assign_t_elevation(TArray<float>& t_elevation, TArray<int32>& t_coastdistance, TArray<FSideIndex>& t_downslope_s, UTriangleDualMesh* Mesh, const TArray<bool>& r_ocean, const TArray<bool>& r_water, FRandomStream& DrainageRng) const
{
	AssignTriangleElevations(t_elevation, t_coastdistance, t_downslope_s, Mesh, r_ocean, r_water, DrainageRng);
}

void UIslandElevation::redistribute_t_elevation(TArray<float>& t_elevation, UTriangleDualMesh* Mesh, const TArray<bool>& r_ocean) const
{
	RedistributeTriangleElevations(t_elevation, Mesh, r_ocean);
}

void UIslandElevation::assign_r_elevation(TArray<float>& r_elevation, UTriangleDualMesh* Mesh, const TArray<float>& t_elevation, const TArray<bool>& r_ocean) const
{
	AssignRegionElevations(r_elevation, Mesh, t_elevation, r_ocean);
}
