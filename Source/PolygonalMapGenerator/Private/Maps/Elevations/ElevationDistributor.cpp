// Original Work Copyright (c) 2010 Amit J Patel, Modified Work Copyright (c) 2016 Jay M Stevens

#include "PolygonalMapGeneratorPrivatePCH.h"
#include "Maps/IslandShapes/IslandShape.h"
#include "MapDataHelper.h"
#include "Maps/Elevations/ElevationDistributor.h"


void UElevationDistributor::SetGraph(UPolygonMap* graph)
{
	MapGraph = graph;
}

void UElevationDistributor::AssignCornerElevations(UIslandShape* islandShape, bool bneedsMoreRandomness, FRandomStream& randomGenerator)
{
	TQueue<int32> cornerQueue;

	for (int i = 0; i < MapGraph->GetCornerNum(); i++)
	{
		FMapCorner corner = MapGraph->GetCorner(i);
		if (islandShape->IsPointLand(corner.CornerData.Point))
		{
			corner.CornerData = UMapDataHelper::RemoveOcean(corner.CornerData);
		}
		else
		{
			corner.CornerData = UMapDataHelper::SetOcean(corner.CornerData);
		}

		if (UMapDataHelper::IsBorder(corner.CornerData) || UMapDataHelper::IsWater(corner.CornerData))
		{
			// The borders are the "base" of our map, and will have the lowest elevation
			// As we expand the queue in the next loop, we will move towards the center
			corner.CornerData.Elevation = 0.0f;
			cornerQueue.Enqueue(i);
		}
		else
		{
			// Set any other corners to a high elevation so we can easily detect changes
			// This will be rescaled later
			corner.CornerData.Elevation = 100.0f;
		}
		MapGraph->UpdateCorner(corner);
	}

	float queueOffsetAmount = 0.0f;
	while (!cornerQueue.IsEmpty())
	{
		int32 cornerIndex;
		cornerQueue.Dequeue(cornerIndex);
		FMapCorner corner = MapGraph->GetCorner(cornerIndex);

		for (int i = 0; i < corner.Adjacent.Num(); i++)
		{
			FMapCorner adjacent = MapGraph->GetCorner(corner.Adjacent[i]);
			float newElevation = 0.1f + corner.CornerData.Elevation;
			//newElevation += queueOffsetAmount;
			if (!UMapDataHelper::IsWater(corner.CornerData) && !UMapDataHelper::IsWater(adjacent.CornerData))
			{
				newElevation += 1.0f;

				// HACK: the map looks nice because of randomness of
				// points, randomness of rivers, and randomness of
				// edges. Without random point selection, I needed to
				// inject some more randomness to make maps look
				// nicer. I'm doing it here, with elevations, but I
				// think there must be a better way. This hack is only
				// used with square/hexagon grids.
				if (bneedsMoreRandomness)
				{
					newElevation += randomGenerator.GetFraction();
				}
			}

			// If this point changed, we'll add it to the queue so
			// that we can process its neighbors too. This makes
			// it so the middle regions have the highest elevation.
			if (newElevation < adjacent.CornerData.Elevation)
			{
				adjacent.CornerData.Elevation = newElevation;
				cornerQueue.Enqueue(adjacent.Index);
			}

			MapGraph->UpdateCorner(adjacent);
		}

		queueOffsetAmount += 0.05f;
	}
}

void UElevationDistributor::CalculateDownslopes()
{
	for (int i = 0; i < MapGraph->GetCornerNum(); i++)
	{
		FMapCorner corner = MapGraph->GetCorner(i);
		int32 downslopeIndex = corner.Index;
		for (int j = 0; j < corner.Adjacent.Num(); j++)
		{
			FMapCorner adjacent = MapGraph->GetCorner(corner.Adjacent[j]);
			if (adjacent.CornerData.Elevation <= MapGraph->GetCorner(downslopeIndex).CornerData.Elevation)
			{
				downslopeIndex = adjacent.Index;
			}
		}
		corner.Downslope = downslopeIndex;
		MapGraph->UpdateCorner(corner);
	}
}

void UElevationDistributor::RedistributeElevations(TArray<int32> landCorners)
{
	TArray<FMapCorner> mapCorners;

	for (int i = 0; i < landCorners.Num(); i++)
	{
		mapCorners.Add(MapGraph->GetCorner(landCorners[i]));
	}

	// Sort by elevation
	mapCorners.HeapSort();

	float maxElevation = -1.0f;
	for (int i = 0; i < mapCorners.Num(); i++)
	{
		// Let y(x) be the total area that we want at elevation <= x.
		// We want the higher elevations to occur less than lower
		// ones, and set the area to be y(x) = 1 - (1-x)^2.
		float y = i / (mapCorners.Num() - 1.0f);
		// Since the data is sorted by elevation, this will linearly increase the elevation as the loop goes on.
		float x = FMath::Sqrt(1.1f) - FMath::Sqrt(1.1f * (1.0f - y));
		if (x > maxElevation)
		{
			maxElevation = x;
		}
		FMapCorner corner = MapGraph->GetCorner(mapCorners[i].Index);
		corner.CornerData.Elevation = x;
		MapGraph->UpdateCorner(corner);
	}

	// Now we normalize all the elevations relative to the largest elevation we have
	// This places all elevations between 0 and 1
	for (int i = 0; i < mapCorners.Num(); i++)
	{
		FMapCorner corner = MapGraph->GetCorner(mapCorners[i].Index);
		corner.CornerData.Elevation /= maxElevation;
		MapGraph->UpdateCorner(corner);
	}
}

void UElevationDistributor::FlattenWaterElevations()
{
	TArray<bool> processedCorners;
	processedCorners.SetNumZeroed(MapGraph->GetCornerNum());
	for (int i = 0; i < MapGraph->GetCornerNum(); i++)
	{
		if (processedCorners[i])
		{
			// Already finished this corner
			continue;
		}

		FMapCorner corner = MapGraph->GetCorner(i);
		if (UMapDataHelper::IsOcean(corner.CornerData))
		{
			corner.CornerData.Elevation = 0.0f;
			MapGraph->UpdateCorner(corner);
			processedCorners[i] = true;
		}
		/*else if (UMapDataHelper::IsFreshwater(corner.CornerData))
		{
			float currentElevation = corner.CornerData.Elevation;
			
			TArray<FMapCorner> waterBiomeCorners;
			TArray<FMapCorner> toProcess;
			toProcess.Add(corner);
			
			while(toProcess.Num() > 0)
			{
				FMapCorner current = toProcess[0];
				toProcess.RemoveAt(0);

				// Keep track of all corners in this biome
				waterBiomeCorners.Add(current);

				// Iterate over all the adjacent corners
				for (int j = 0; j < current.Adjacent.Num(); j++)
				{
					// Check to see if we've already processed this neighbor
					if (processedCorners[current.Adjacent[j]])
					{
						// Already processed
						continue;
					}

					FMapCorner neighbor = MapGraph->GetCorner(current.Adjacent[j]);
					if (!UMapDataHelper::IsFreshwater(neighbor.CornerData))
					{
						// Ignore non-water corners
						continue;
					}

					// Minimum elevation is given by the minimum of the current elevation vs. this neighbor's elevation
					// This will give us a flat water level
					currentElevation = FMath::Min(currentElevation, neighbor.CornerData.Elevation);
					// Add the neighbor to the process list to process its neighbors
					toProcess.Add(neighbor);

					// Mark the processing on this corner as done
					processedCorners[current.Adjacent[j]] = true;
				}
			}

			// Done finding all the corners to process for this biome
			// We also found the water level we're aiming for
			// Now we need to give that water level to all corners in the biome
			for (int j = 0; j < waterBiomeCorners.Num(); j++)
			{
				waterBiomeCorners[j].CornerData.Elevation = currentElevation;
				MapGraph->UpdateCorner(waterBiomeCorners[j]);
			}
		}*/
	}
}

void UElevationDistributor::AssignPolygonElevations()
{
	for (int i = 0; i < MapGraph->GetCenterNum(); i++)
	{
		float sumElevation = 0.0f;
		FMapCenter center = MapGraph->GetCenter(i);
		for (int32 j = 0; j < center.Corners.Num(); j++)
		{
			FMapCorner corner = MapGraph->GetCorner(center.Corners[j]);
			sumElevation += corner.CornerData.Elevation;
		}
		float elevation = sumElevation / center.Corners.Num();
		center.CenterData.Elevation = elevation;
		MapGraph->UpdateCenter(center);
	}
}
