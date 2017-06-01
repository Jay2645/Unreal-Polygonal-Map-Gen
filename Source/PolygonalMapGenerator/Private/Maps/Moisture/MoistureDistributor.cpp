// Fill out your copyright notice in the Description page of Project Settings.

#include "PolygonalMapGeneratorPrivatePCH.h"
#include "MapDataHelper.h"
#include "MoistureDistributor.h"

void UMoistureDistributor::SetGraph(UPolygonMap* Graph, int32 Size)
{
	MapGraph = Graph;
	MapSize = Size;
}

void UMoistureDistributor::AssignOceanCoastAndLand()
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
		if (!UMapDataHelper::IsOcean(center.CenterData) && numWater > center.Corners.Num() * LakeThreshold)
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

void UMoistureDistributor::CalculateWatersheds()
{
	// Calculate the watershed of every land point. The watershed is
	// the last downstream land point in the downslope graph. TODO:
	// watersheds are currently calculated on corners, but it'd be
	// more useful to compute them on polygon centers so that every
	// polygon can be marked as being in one watershed.

	// Initially the watershed pointer points downslope one step.  
	for (int i = 0; i < MapGraph->GetCornerNum(); i++)
	{
		FMapCorner corner = MapGraph->GetCorner(i);
		corner.Watershed = corner.Index;
		if (!UMapDataHelper::IsOcean(corner.CornerData) && !UMapDataHelper::IsCoast(corner.CornerData))
		{
			corner.Watershed = corner.Downslope;
		}
		MapGraph->UpdateCorner(corner);
	}

	// Follow the downslope pointers to the coast. Limit to 100
	// iterations although most of the time with numPoints==2000 it
	// only takes 20 iterations because most points are not far from
	// a coast.
	for (int i = 0; i < 100; i++)
	{
		bool bChanged = false;
		for (int j = 0; j < MapGraph->GetCornerNum(); j++)
		{
			FMapCorner corner = MapGraph->GetCorner(j);
			FMapCorner watershed = MapGraph->GetCorner(corner.Watershed);
			if (!UMapDataHelper::IsOcean(corner.CornerData) && !UMapDataHelper::IsCoast(corner.CornerData) && !UMapDataHelper::IsCoast(watershed.CornerData))
			{
				FMapCorner downstreamWatershed = MapGraph->GetCorner(watershed.Watershed);
				if (!UMapDataHelper::IsOcean(downstreamWatershed.CornerData))
				{
					corner.Watershed = downstreamWatershed.Index;
					MapGraph->UpdateCorner(corner);
					bChanged = true;
				}
			}
		}
		if (!bChanged)
		{
			break;
		}
	}

	// How big is each watershed?
	for (int i = 0; i < MapGraph->GetCornerNum(); i++)
	{
		FMapCorner corner = MapGraph->GetCorner(i);
		FMapCorner watershed = MapGraph->GetCorner(corner.Watershed);
		watershed.WatershedSize += 1;
		MapGraph->UpdateCorner(watershed);
	}
}

void UMoistureDistributor::CreateRivers(FRandomStream& RandomGenerator)
{
	/*// The moisture distributor continues until it hits a coast tile
	// First, we need to check to ensure that we have a coast tile
	bool hasCoastTile = false;
	for (int i = 0; i < MapGraph->GetCornerNum(); i++)
	{
		if (UMapDataHelper::IsCoast(MapGraph->GetCorner(i).CornerData))
		{
			hasCoastTile = true;
			break;
		}
	}
	if (!hasCoastTile)
	{
		UE_LOG(LogWorldGen, Error, TEXT("No tiles were marked as being coastline!"));
		return;
	}

	for (int i = 0; i < MapSize * RiverRatio; i++)
	{
		// Randomly select a corner
		int cornerIndex = RandomGenerator.RandRange(0, MapGraph->GetCornerNum() - 1);
		FMapCorner riverSource = MapGraph->GetCorner(cornerIndex);
		if (UMapDataHelper::IsOcean(riverSource.CornerData))
		{
			// Ignore it if it is ocean
			continue;
		}
		while (!UMapDataHelper::IsCoast(riverSource.CornerData))
		{
			// Continue downstream until we reach the coast
			FMapCorner downslopeCorner = MapGraph->GetCorner(riverSource.Downslope);
			if (downslopeCorner.Index == riverSource.Index || downslopeCorner.Index < 0)
			{
				break;
			}

			FMapEdge edge = MapGraph->FindEdgeFromCorners(riverSource, downslopeCorner);
			edge.RiverVolume++;
			riverSource.RiverSize++;
			riverSource.CornerData = UMapDataHelper::SetRiver(riverSource.CornerData);
			riverSource.CornerData.Elevation -= 0.05f;
			downslopeCorner.RiverSize++;
			downslopeCorner.CornerData = UMapDataHelper::SetRiver(downslopeCorner.CornerData);
			downslopeCorner.CornerData.Elevation -= 0.05f;

			MapGraph->UpdateEdge(edge);
			MapGraph->UpdateCorner(riverSource);
			MapGraph->UpdateCorner(downslopeCorner);

			riverSource = downslopeCorner;
		}
	}*/
	/*TArray<FMapCorner> cornerList;
	cornerList.Append(MapGraph->Corners);
	while (Rivers.Num() < RiverCount)
	{
		if (cornerList.Num() == 0)
		{
			UE_LOG(LogWorldGen, Warning, TEXT("Asked to generate %d rivers, but we could only generate %d!"), RiverCount, Rivers.Num());
			break;
		}
		int cornerIndex = RandomGenerator.RandRange(0, cornerList.Num() - 1);
		FMapCorner riverSource = cornerList[cornerIndex];
		cornerList.RemoveAt(cornerIndex);

		if (UMapDataHelper::IsOcean(riverSource.CornerData))
		{
			// Ignore it if it is ocean
			continue;
		}
		// Verify it's far enough away from the coastline
		if (UMapDataHelper::IsCoast(riverSource.CornerData))
		{
			// Ignore it if it is ocean
			continue;
		}
		// Verify it's not already part of a river
		if (riverSource.RiverSize != 0)
		{
			continue;
		}
		// Verify it's within the correct elevation
		if (riverSource.CornerData.Elevation < MinRiverStartElevation || riverSource.CornerData.Elevation > MaxRiverStartElevation)
		{
			continue;
		}
		// Verify it has multiple connections
		if (riverSource.Adjacent.Num() <= 1)
		{
			continue;
		}

		// River is valid!
		// Start making the river
		URiver* river = NewObject<URiver>();
		FMapCorner currentCorner = riverSource;
		bool bIsValid = true;
		while (bIsValid && !UMapDataHelper::IsOcean(currentCorner.CornerData))
		{
			for (int i = 0; i < river->RiverCorners.Num(); i++)
			{
				if (river->RiverCorners[i].Index == currentCorner.Index)
				{
					// River loops onto itself
					bIsValid = false;
					break;
				}
			}
			if (!bIsValid)
			{
				break;
			}
			river->RiverCorners.Add(currentCorner);

			float randomValue = RandomGenerator.GetFraction();
			FMapCorner next;
			if (currentCorner.Downslope != -1 && randomValue <= RiverDownstreamBias)
			{
				next = MapGraph->GetCorner(currentCorner.Downslope);
			}
			else
			{
				if (currentCorner.Adjacent.Num() == 0)
				{
					// No adjacent corners
					bIsValid = false;
					break;
				}
				// First, iterate over all the corners to try and find a good candidate by chance
				for (int i = 0; i < currentCorner.Adjacent.Num(); i++)
				{
					bool bShouldSkip = false;
					for (int j = 0; j < river->RiverCorners.Num(); j++)
					{
						if (river->RiverCorners[j].Index == currentCorner.Adjacent[i])
						{
							// This corner is already in the river
							bShouldSkip = true;
							break;
						}
					}
					if (bShouldSkip)
					{
						continue;
					}
					randomValue = RandomGenerator.GetFraction();
					if (randomValue <= StandingWaterBias && UMapDataHelper::IsWater(MapGraph->GetCorner(currentCorner.Adjacent[i]).CornerData))
					{
						next = MapGraph->GetCorner(currentCorner.Adjacent[i]);
						break;
					}
				}
				if (next.Index == -1)
				{
					// No good candidates
					while (next.Adjacent.Num() >= 1)
					{
						int randomIndex = RandomGenerator.RandRange(0, next.Adjacent.Num() - 1);
						FMapCorner possibleNext = MapGraph->GetCorner(next.Adjacent[randomIndex]);
						next.Adjacent.RemoveAt(randomIndex);

						bool bIsPossibleNextValid = true;
						for (int i = 0; i < river->RiverCorners.Num(); i++)
						{
							if (river->RiverCorners[i].Index == possibleNext.Index)
							{
								// River loops onto itself
								bIsPossibleNextValid = false;
								break;
							}
						}
						if (!bIsPossibleNextValid)
						{
							// Already have this point in the river
							continue;
						}
						next = possibleNext;
						if (next.RiverSize > 0)
						{
							randomValue = RandomGenerator.GetFraction();
							if (randomValue > RiverJoinBias)
							{
								continue;
							}
						}
						break;
					}
				}
			}

			if (next.Index == -1)
			{
				// Next point is invalid
				bIsValid = false;
				break;
			}

			currentCorner = next;
		}

		if (river->RiverCorners.Num() < MinRiverSectionCount)
		{
			bIsValid = false;
		}

		if (!bIsValid)
		{
			continue;
		}

		for (int i = 0; i < river->RiverCorners.Num(); i++)
		{
			river->RiverCorners[i].RiverSize += (int32)StartingRiverSize;
			river->RiverCorners[i].RiverSize = FMath::Clamp<int32>(river->RiverCorners[i].RiverSize, 0, (int32)MaxRiverSize);
			if (i + 1 < river->RiverCorners.Num())
			{
				FMapEdge riverEdge = MapGraph->FindEdgeFromCorners(river->RiverCorners[i], river->RiverCorners[i + 1]);
				if (riverEdge.Index != -1)
				{
					riverEdge.RiverVolume = river->RiverCorners[i].RiverSize;
					MapGraph->UpdateEdge(riverEdge);
				}
			}
			MapGraph->UpdateCorner(river->RiverCorners[i]);
		}

		Rivers.Add(river);
	}*/

	// Create rivers along edges. Pick a random corner point, then
	// move downslope. Mark the edges and corners as rivers.
	TArray<FMapCorner> cornerList;
	cornerList.Append(MapGraph->Corners);
	while (Rivers.Num() < RiverCount)
	{
		if (cornerList.Num() == 0)
		{
			//Debug.LogWarning("Ran out of corners to check for rivers! "
			//	+ "Using " + corners.Count + " corners and only made " + rivers.Count + " out of " + waterSettings.riverCount + " rivers!");
			break;
		}

		// Get a random element in the list
		int randomRiver = RandomGenerator.RandRange(0, cornerList.Num() - 1);
		FMapCorner riverSource = cornerList[randomRiver];
		cornerList.RemoveAt(randomRiver);

		// Check to see if it's already in the ocean
		if (UMapDataHelper::IsOcean(riverSource.CornerData))
		{
			continue;
		}
		// Verify it's far enough away from the coastline
		if (UMapDataHelper::IsCoast(riverSource.CornerData))
		{
			continue;
		}
		// Verify it's not already part of a river
		if (riverSource.River != NULL)
		{
			continue;
		}
		// Verify it's within the correct elevation
		if (riverSource.CornerData.Elevation < MinRiverStartElevation || riverSource.CornerData.Elevation > MaxRiverStartElevation)
		{
			continue;
		}
		// Verify it has multiple connections
		if (riverSource.Adjacent.Num() <= 1)
		{
			continue;
		}

		// Source is valid, so start making a river
		URiver* river = NewObject<URiver>();
		river->MapGraph = MapGraph;

		FString invalidRiverReason;
		FMapCorner current = riverSource;
		while (!UMapDataHelper::IsOcean(current.CornerData))
		{
			if (current.River != NULL)
			{
				current = river->AddCorner(current, 0);
				if (!current.River->JoinRiver(river, current))
				{
					invalidRiverReason = "river could not be joined.";
				}
				break;
			}
			else
			{
				river->AddCorner(current);
			}

			FMapCorner next;
			// Check to see if we should move downstream
			float randomValue = RandomGenerator.GetFraction();
			if (randomValue <= RiverDownstreamBias &&
				current.Downslope != -1 &&
				current.Index != current.Downslope &&
				MapGraph->GetCorner(current.Downslope).River != river)
			{
				next = MapGraph->GetCorner(current.Downslope);
			}
			else
			{
				// Don't move downstream; find an adjacent corner to move to
				TArray<int32> adjacent;
				adjacent.Append(current.Adjacent);
				if (adjacent.Num() == 0)
				{
					invalidRiverReason = "there were no adjacent corners.";
					break;
				}

				for(int i = 0; i < adjacent.Num(); i++)
				{
					FMapCorner neighbor = MapGraph->GetCorner(adjacent[i]);
					if (neighbor.River == river)
					{
						continue;
					}
					randomValue = RandomGenerator.GetFraction();
					if (UMapDataHelper::IsWater(neighbor.CornerData) && randomValue <= StandingWaterBias)
					{
						next = neighbor;
						break;
					}
				}

				if (next.Index == -1)
				{
					while (adjacent.Num() >= 1)
					{
						int randomIndex = RandomGenerator.RandRange(0, adjacent.Num() - 1);
						FMapCorner possibleNext = MapGraph->GetCorner(adjacent[randomIndex]);
						adjacent.RemoveAt(randomIndex);
						if (possibleNext.River == river)
						{
							continue;
						}
						if (possibleNext.Adjacent.Num() < 1)
						{
							continue;
						}

						next = possibleNext;

						if (next.River != NULL)
						{
							randomValue = RandomGenerator.GetFraction();
							if (randomValue > RiverJoinBias)
							{
								continue;
							}
						}
						break;
					}
				}
			}

			if (next.Index == -1)
			{
				invalidRiverReason = "there was no valid candidate for the next node.";
				break;
			}
			else if (next.River == river)
			{
				invalidRiverReason = "the river would feed into itself.";
				break;
			}

			current = next;
		}

		if (!invalidRiverReason.IsEmpty())
		{
			// River was invalid, continue
			//Debug.LogWarning("Tossing " + river + " because " + invalidRiverReason);
			river->Clear();
			continue;
		}
		else
		{
			river->AddCorner(current);
			if (river->Terminates())
			{
				if (river->RiverCorners.Num() <= (int32)MinRiverSectionCount)
				{
					if (river->FeedsInto.Key == NULL)
					{
						//Debug.LogWarning("Tossing " + river + " because its length (" + river.Length + ") is smaller than the water settings count (" + waterSettings.minNumberRiverSectionCount + ").");
						river->Clear();
						continue;
					}
					else
					{
						river->MakeTributary();
					}
				}
				river->InitializeRiver(RiverNameTable, RandomGenerator);
				Rivers.Add(river);
			}
			else
			{
				//Debug.LogWarning("Tossing " + river + " because it does not terminate properly.");
				river->Clear();
				continue;
			}
		}
		/*Corner q = corners[Random.Range(0, corners.Count)];
		if (q.ocean || q.coast || q.elevation < waterSettings.minRiverStartElevation || q.elevation > waterSettings.maxRiverStartElevation)
		{
		continue;
		}

		River river = new River(graph, waterSettings.riverSize);

		while (!q.ocean)
		{
		if (q == q.downslope)
		{
		break;
		}
		Edge edge = graph.lookupEdgeFromCorner(q, q.downslope);
		edge.river = edge.river + 1;

		river.AddCorner(q);
		q.river++;
		q.downslope.river++;  // TODO: fix double count
		q.river = Mathf.Min(q.river, waterSettings.maxRiverSize);
		q.downslope.river = Mathf.Min(q.downslope.river, waterSettings.maxRiverSize);
		q = q.downslope;
		}

		rivers.Add(river)*/

		// Find rivers to convert into lakes
		TArray<URiver*> riverConnectionList;
		riverConnectionList.Append(Rivers);
		while (riverConnectionList.Num() > 0)
		{
			URiver* r1 = riverConnectionList[0];
			riverConnectionList.RemoveAt(0);
			FMapCorner source1 = r1->GetCorner(0);
			// If this is a coastal tile, turning it into a lake would just expand the ocean
			// This would also impact the delta formation
			if (UMapDataHelper::IsCoast(source1.CornerData))
			{
				continue;
			}

			TArray<URiver*> remove;
			// Iterate over all our rivers
			for(int i = 0; i < riverConnectionList.Num(); i++)
			{
				URiver* r2 = riverConnectionList[i];
				// If rivers are adjacent but one feeds into the other, don't make them into a lake
				if (r1->FeedsInto.Key == r2 || r2->FeedsInto.Key == r1)
				{
					continue;
				}


				FMapCorner adjacent;
				FMapCorner source2 = r2->GetCorner(0);
				// If this is a coastal tile, turning it into a lake would just expand the ocean
				if (UMapDataHelper::IsCoast(source2.CornerData))
				{
					continue;
				}
				for(int j = 0; j < source1.Adjacent.Num(); j++)
				{
					FMapCorner adjacent1 = MapGraph->GetCorner(source1.Adjacent[j]);
					if (UMapDataHelper::IsCoast(adjacent1.CornerData))
					{
						continue;
					}
					if (adjacent1 == source2)
					{
						adjacent = adjacent1;
						break;
					}
				}

				// If we have a valid adjacent corner
				if (adjacent.Index != -1)
				{
					if (RiverLakeConversionFactor >= RandomGenerator.GetFraction())
					{
						FMapCenter center = MapGraph->FindCenterFromCorners(source1, adjacent);
						if (UMapDataHelper::IsCoast(center.CenterData))
						{
							continue;
						}

						// Set the polygon to be a lake
						center.CenterData = UMapDataHelper::SetFreshwater(center.CenterData);
						for(int j = 0; j < center.Corners.Num(); j++)
						{
							FMapCorner c = MapGraph->GetCorner(center.Corners[j]);
							if (UMapDataHelper::IsCoast(c.CornerData))
							{
								continue;
							}
							c.CornerData = UMapDataHelper::SetFreshwater(c.CornerData);
							MapGraph->UpdateCorner(c);
						}
						MapGraph->UpdateCenter(center);
					}
					// Remove the second river from consideration
					remove.Add(r2);
				}
			}
			for(int i = 0; i < remove.Num(); i++)
			{
				riverConnectionList.Remove(remove[i]);
			}
		}
	}

	for (int i = 0; i < Rivers.Num(); i++)
	{
		URiver* r = Rivers[i];
		for (int j = 0; j < r->RiverCorners.Num(); j++)
		{
			FMapCorner corner = r->GetCorner(j);
			corner.RiverSize = FMath::Clamp(corner.RiverSize, 0, (int32)MaxRiverSize);
			if(corner.RiverSize > 0)
			{
				corner.CornerData = UMapDataHelper::SetRiver(corner.CornerData);
			}
			MapGraph->UpdateCorner(corner);
		}
	}
}
void UMoistureDistributor::AssignCornerMoisture()
{
	TQueue<FMapCorner> moistureQueue;
	for (int i = 0; i < MapGraph->GetCornerNum(); i++)
	{
		FMapCorner corner = MapGraph->GetCorner(i);
		if ((UMapDataHelper::IsWater(corner.CornerData) || UMapDataHelper::IsRiver(corner.CornerData)) && !UMapDataHelper::IsOcean(corner.CornerData))
		{
			corner.CornerData.Moisture = UMapDataHelper::IsRiver(corner.CornerData) ? FMath::Min(3.0f, (0.2f * corner.RiverSize)) : 1.0f;
			moistureQueue.Enqueue(corner);
		}
		else
		{
			corner.CornerData.Moisture = 0.0f;
			MapGraph->UpdateCorner(corner);
		}
	}

	while (!moistureQueue.IsEmpty())
	{
		FMapCorner corner;
		moistureQueue.Dequeue(corner);
		for (int i = 0; i < corner.Adjacent.Num(); i++)
		{
			FMapCorner neighbor = MapGraph->GetCorner(corner.Adjacent[i]);
			float newMoisture = corner.CornerData.Moisture * 0.9f;
			if (newMoisture > neighbor.CornerData.Moisture)
			{
				neighbor.CornerData.Moisture = newMoisture;
				moistureQueue.Enqueue(neighbor);
			}
		}
		MapGraph->UpdateCorner(corner);
	}

	// Saltwater
	for (int i = 0; i < MapGraph->GetCornerNum(); i++)
	{
		FMapCorner corner = MapGraph->GetCorner(i);
		if (UMapDataHelper::IsCoast(corner.CornerData) || UMapDataHelper::IsOcean(corner.CornerData))
		{
			corner.CornerData.Moisture = 1.0f;
			MapGraph->UpdateCorner(corner);
		}
	}
}

void UMoistureDistributor::RedistributeMoisture(TArray<int32> landCorners)
{
	float maxMoisture = -1.0f;
	for (int i = 0; i < landCorners.Num(); i++)
	{
		FMapCorner corner = MapGraph->GetCorner(landCorners[i]);
		if (corner.CornerData.Moisture > maxMoisture)
		{
			maxMoisture = corner.CornerData.Moisture;
		}
	}

	for (int i = 0; i < landCorners.Num(); i++)
	{
		FMapCorner corner = MapGraph->GetCorner(landCorners[i]);
		corner.CornerData.Moisture /= maxMoisture;
		MapGraph->UpdateCorner(corner);
	}
}

void UMoistureDistributor::AssignPolygonMoisture()
{
	for (int i = 0; i < MapGraph->GetCenterNum(); i++)
	{
		FMapCenter center = MapGraph->GetCenter(i);
		float sumMoisture = 0.0f;
		for (int j = 0; j < center.Corners.Num(); j++)
		{
			FMapCorner corner = MapGraph->GetCorner(center.Corners[j]);
			sumMoisture = corner.CornerData.Moisture;
		}
		center.CenterData.Moisture = sumMoisture / center.Corners.Num();
		MapGraph->UpdateCenter(center);
	}
}