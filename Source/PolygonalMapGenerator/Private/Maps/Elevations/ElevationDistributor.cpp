// Original Work Copyright (c) 2010 Amit J Patel, Modified Work Copyright (c) 2016 Jay M Stevens

#include "PolygonalMapGeneratorPrivatePCH.h"
#include "Maps/IslandShapes/IslandShape.h"
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

void UElevationDistributor::AssignOceanCoastAndLand(float lakeThreshold)
{
	TQueue<int32> centerQueue;
	// Compute polygon attributes 'ocean' and 'water' based on the
	// corner attributes. Count the water corners per
	// polygon. Oceans are all polygons connected to the edge of the
	// map. In the first pass, mark the edges of the map as ocean;
	// in the second pass, mark any water-containing polygon
	// connected an ocean as ocean.
	for (int32 i = 0; i < MapGraph->GetCenterNum(); i++)
	{
		uint16 numWater = 0;
		FMapCenter center = MapGraph->GetCenter(i);
		for (int32 j = 0; j < center.Corners.Num(); j++)
		{
			FMapCorner corner = MapGraph->GetCorner(center.Corners[j]);
			if (UMapDataHelper::IsBorder(corner.CornerData))
			{
				center.CenterData = UMapDataHelper::SetBorder(center.CenterData);
				center.CenterData = UMapDataHelper::SetOcean(center.CenterData);
				centerQueue.Enqueue(i);
			}
			if (UMapDataHelper::IsWater(corner.CornerData))
			{
				numWater++;
			}
			MapGraph->UpdateCorner(corner);
		}
		if (!UMapDataHelper::IsOcean(center.CenterData) && numWater > center.Corners.Num() * lakeThreshold)
		{
			center.CenterData = UMapDataHelper::SetFreshwater(center.CenterData);
		}
		else
		{
			center.CenterData = UMapDataHelper::RemoveFreshwater(center.CenterData);
		}
		MapGraph->UpdateCenter(center);
	}

	while (!centerQueue.IsEmpty())
	{
		int32 centerIndex;
		centerQueue.Dequeue(centerIndex);
		FMapCenter center = MapGraph->GetCenter(centerIndex);
		for (int32 i = 0; i < center.Neighbors.Num(); i++)
		{
			FMapCenter neighbor = MapGraph->GetCenter(center.Neighbors[i]);
			if (UMapDataHelper::IsWater(neighbor.CenterData) && !UMapDataHelper::IsOcean(neighbor.CenterData))
			{
				neighbor.CenterData = UMapDataHelper::SetOcean(neighbor.CenterData);
				centerQueue.Enqueue(neighbor.Index);
			}
			MapGraph->UpdateCenter(neighbor);
		}
	}

	// Set the polygon attribute 'coast' based on its neighbors. If
	// it has at least one ocean and at least one land neighbor,
	// then this is a coastal polygon.
	for (int32 i = 0; i < MapGraph->GetCenterNum(); i++)
	{
		uint16 numOcean = 0;
		uint16 numLand = 0;
		FMapCenter center = MapGraph->GetCenter(i);
		for (int j = 0; j < center.Neighbors.Num(); j++)
		{
			FMapCenter neighbor = MapGraph->GetCenter(center.Neighbors[j]);
			if (UMapDataHelper::IsOcean(neighbor.CenterData))
			{
				numOcean++;
			}
			else if (!UMapDataHelper::IsWater(neighbor.CenterData))
			{
				numLand++;
			}
			if (numOcean > 0 && numLand > 0)
			{
				break;
			}
		}
		if (numOcean > 0 && numLand > 0)
		{
			center.CenterData = UMapDataHelper::SetCoast(center.CenterData);
		}
		else
		{
			center.CenterData = UMapDataHelper::RemoveCoast(center.CenterData);
		}
		MapGraph->UpdateCenter(center);
	}

	// Set the corner attributes based on the computed polygon
	// attributes. If all polygons connected to this corner are
	// ocean, then it's ocean; if all are land, then it's land;
	// otherwise it's coast.
	for (int32 i = 0; i < MapGraph->GetCornerNum(); i++)
	{
		uint16 numOcean = 0;
		uint16 numLand = 0;
		FMapCorner corner = MapGraph->GetCorner(i);
		for (int j = 0; j < corner.Touches.Num(); j++)
		{
			FMapCenter neighbor = MapGraph->GetCenter(corner.Touches[j]);
			if (UMapDataHelper::IsOcean(neighbor.CenterData))
			{
				numOcean++;
			}
			else if (!UMapDataHelper::IsWater(neighbor.CenterData))
			{
				numLand++;
			}
		}
		if (numOcean == corner.Touches.Num())
		{
			corner.CornerData = UMapDataHelper::SetOcean(corner.CornerData);
		}
		else
		{
			corner.CornerData = UMapDataHelper::RemoveOcean(corner.CornerData);
		}
		if (numOcean > 0 && numLand > 0)
		{
			corner.CornerData = UMapDataHelper::SetCoast(corner.CornerData);
		}
		else
		{
			corner.CornerData = UMapDataHelper::RemoveCoast(corner.CornerData);
		}
		if (UMapDataHelper::IsBorder(corner.CornerData))
		{
			corner.CornerData = UMapDataHelper::SetOcean(corner.CornerData);
		}
		else if (numLand != corner.Touches.Num() && !UMapDataHelper::IsCoast(corner.CornerData))
		{
			corner.CornerData = UMapDataHelper::SetFreshwater(corner.CornerData);
		}
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
	for (int i = 0; i < MapGraph->GetCornerNum(); i++)
	{
		FMapCorner corner = MapGraph->GetCorner(i);
		if (UMapDataHelper::IsOcean(corner.CornerData) || UMapDataHelper::IsCoast(corner.CornerData))
		{
			corner.CornerData.Elevation = 0.0f;
		}
		MapGraph->UpdateCorner(corner);
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
