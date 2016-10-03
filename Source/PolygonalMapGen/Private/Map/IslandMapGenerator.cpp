// Original Work Copyright (c) 2010 Amit J Patel, Modified Work Copyright (c) 2016 Jay M Stevens

#include "PolygonalMapGen.h"
#include "IslandMapGenerator.h"

void AIslandMapGenerator::SetData(FIslandData islandData)
{
	IslandData = islandData;
	RandomGenerator.Initialize(IslandData.Seed);
}

void AIslandMapGenerator::ResetMap()
{
	UObject* outer = (UObject*)GetTransientPackage();

	if (IslandData.IslandType != NULL)
	{
		IslandShape = NewObject<UIslandShape>(outer, IslandData.IslandType);
	}
	if (IslandData.IslandPointSelector != NULL)
	{
		PointSelector = NewObject<UPointGenerator>(outer, IslandData.IslandPointSelector);
	}
}

void AIslandMapGenerator::GenerateMap_Implementation()
{
	UE_LOG(LogWorldGen, Log, TEXT("Generating a new map at %f."), IslandData.GameWorld->GetRealTimeSeconds());
	ResetMap();
	if (IslandShape == NULL || PointSelector == NULL)
	{
		UE_LOG(LogWorldGen, Error, TEXT("IslandShape or PointSelector were null!"));
		return;
	}

	// Initialization
	InitializeMap();

	BuildGraph();

	AssignElevation();

	AssignMoisture();

	// Package the map up to send to the voxel engine
	FinalizeAllPoints();

	MapGraph->DrawDebugVoronoiGrid(IslandData.GameWorld);
	//MapGraph->DrawDebugDelaunayGrid(IslandData.GameWorld);
	//PixelMap->DrawDebugPixelGrid(IslandData.GameWorld);
}

void AIslandMapGenerator::InitializeMap()
{
	MapGraph = NewObject<UPolygonMap>();
	IslandShape->SetSeed(IslandData.Seed, IslandData.Size);
	PointSelector->InitializeSelector(IslandData.Size, IslandData.Seed);
}

void AIslandMapGenerator::BuildGraph()
{
	// Place points
	MapGraph->CreatePoints(PointSelector, IslandData.NumberOfPoints);

	// Build graph
	MapGraph->BuildGraph(IslandData.Size);
	MapGraph->ImproveCorners();
}

UPolygonMap* AIslandMapGenerator::GetGraph()
{
	return MapGraph;
}

int32 AIslandMapGenerator::GetCenterNum()
{
	return MapGraph->GetCenterNum();
}
int32 AIslandMapGenerator::GetCornerNum()
{
	return MapGraph->GetCornerNum();
}
int32 AIslandMapGenerator::GetEdgeNum()
{
	return MapGraph->GetEdgeNum();
}

FMapCenter& AIslandMapGenerator::GetCenter(const int32 index)
{
	return MapGraph->GetCenter(index);
}
FMapCorner& AIslandMapGenerator::GetCorner(const int32 index)
{
	return MapGraph->GetCorner(index);
}
FMapEdge& AIslandMapGenerator::GetEdge(const int32 index)
{
	return MapGraph->GetEdge(index);
}
FMapEdge& AIslandMapGenerator::FindEdgeFromCenters(const FMapCenter& v0, const FMapCenter& v1)
{
	return MapGraph->FindEdgeFromCenters(v0, v1);
}

FMapEdge& AIslandMapGenerator::FindEdgeFromCorners(const FMapCorner& v0, const FMapCorner& v1)
{
	return MapGraph->FindEdgeFromCorners(v0, v1);
}

void AIslandMapGenerator::UpdateCenter(const FMapCenter& center)
{
	MapGraph->UpdateCenter(center);
}
void AIslandMapGenerator::UpdateCorner(const FMapCorner& corner)
{
	MapGraph->UpdateCorner(corner);
}
void AIslandMapGenerator::UpdateEdge(const FMapEdge& edge)
{
	MapGraph->UpdateEdge(edge);
}

void AIslandMapGenerator::AssignElevation()
{
	// Assign elevations
	AssignCornerElevations();
	// Determine polygon and corner type: ocean, coast, land.
	AssignOceanCoastAndLand();

	// Change the overall distribution of elevations so that lower
	// elevations are more common than higher
	// elevations. Specifically, we want elevation X to have frequency
	// (1-X).  To do this we will sort the corners, then set each
	// corner to its desired elevation.
	RedistributeElevations(MapGraph->FindLandCorners());
	FlattenWaterElevations();
	AssignPolygonElevations();
}

void AIslandMapGenerator::AssignCornerElevations()
{
	TQueue<int32> cornerQueue;

	for (int i = 0; i < GetCornerNum(); i++)
	{
		FMapCorner corner = GetCorner(i);
		corner.CornerData.bIsWater = !IslandShape->IsPointLand(corner.CornerData.Point);
		if (corner.CornerData.bIsBorder || corner.CornerData.bIsWater)
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
		UpdateCorner(corner);
	}

	float queueOffsetAmount = 0.0f;
	while (!cornerQueue.IsEmpty())
	{
		int32 cornerIndex;
		cornerQueue.Dequeue(cornerIndex);
		FMapCorner corner = GetCorner(cornerIndex);

		for (int i = 0; i < corner.Adjacent.Num(); i++)
		{
			FMapCorner adjacent = GetCorner(corner.Adjacent[i]);
			float newElevation = 0.1f + corner.CornerData.Elevation;
			//newElevation += queueOffsetAmount;
			if (!corner.CornerData.bIsWater && !adjacent.CornerData.bIsWater)
			{
				newElevation += 1.0f;

				// HACK: the map looks nice because of randomness of
				// points, randomness of rivers, and randomness of
				// edges. Without random point selection, I needed to
				// inject some more randomness to make maps look
				// nicer. I'm doing it here, with elevations, but I
				// think there must be a better way. This hack is only
				// used with square/hexagon grids.
				if (PointSelector->NeedsMoreRandomness())
				{
					newElevation += RandomGenerator.GetFraction();
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

			UpdateCorner(adjacent);
		}

		queueOffsetAmount += 0.05f;
	}
}

void AIslandMapGenerator::AssignOceanCoastAndLand()
{
	TQueue<int32> centerQueue;
	// Compute polygon attributes 'ocean' and 'water' based on the
	// corner attributes. Count the water corners per
	// polygon. Oceans are all polygons connected to the edge of the
	// map. In the first pass, mark the edges of the map as ocean;
	// in the second pass, mark any water-containing polygon
	// connected an ocean as ocean.
	for (int32 i = 0; i < GetCenterNum(); i++)
	{
		uint16 numWater = 0;
		FMapCenter center = GetCenter(i);
		for (int32 j = 0; j < center.Corners.Num(); j++)
		{
			FMapCorner corner = GetCorner(center.Corners[j]);
			if (corner.CornerData.bIsBorder)
			{
				center.CenterData.bIsBorder = true;
				center.CenterData.bIsOcean = true;
				corner.CornerData.bIsWater = true;
				centerQueue.Enqueue(i);
			}
			if (corner.CornerData.bIsWater)
			{
				numWater++;
			}
			UpdateCorner(corner);
		}
		center.CenterData.bIsWater = center.CenterData.bIsOcean || numWater > center.Corners.Num() * IslandData.LakeThreshold;
		UpdateCenter(center);
	}

	while (!centerQueue.IsEmpty())
	{
		int32 centerIndex;
		centerQueue.Dequeue(centerIndex);
		FMapCenter center = GetCenter(centerIndex);
		for (int32 i = 0; i < center.Neighbors.Num(); i++)
		{
			FMapCenter neighbor = GetCenter(center.Neighbors[i]);
			if (neighbor.CenterData.bIsWater && !neighbor.CenterData.bIsOcean)
			{
				neighbor.CenterData.bIsOcean = true;
				centerQueue.Enqueue(neighbor.Index);
			}
			UpdateCenter(neighbor);
		}
	}

	// Set the polygon attribute 'coast' based on its neighbors. If
	// it has at least one ocean and at least one land neighbor,
	// then this is a coastal polygon.
	for (int32 i = 0; i < GetCenterNum(); i++)
	{
		uint16 numOcean = 0;
		uint16 numLand = 0;
		FMapCenter center = GetCenter(i);
		for (int j = 0; j < center.Neighbors.Num(); j++)
		{
			FMapCenter neighbor = GetCenter(center.Neighbors[j]);
			if (neighbor.CenterData.bIsOcean)
			{
				numOcean++;
			}
			else if (!neighbor.CenterData.bIsWater)
			{
				numLand++;
			}
			if (numOcean > 0 && numLand > 0)
			{
				break;
			}
		}
		center.CenterData.bIsCoast = numOcean > 0 && numLand > 0;
		UpdateCenter(center);
	}

	// Set the corner attributes based on the computed polygon
	// attributes. If all polygons connected to this corner are
	// ocean, then it's ocean; if all are land, then it's land;
	// otherwise it's coast.
	for (int32 i = 0; i < GetCornerNum(); i++)
	{
		uint16 numOcean = 0;
		uint16 numLand = 0;
		FMapCorner corner = GetCorner(i);
		for (int j = 0; j < corner.Touches.Num(); j++)
		{
			FMapCenter neighbor = GetCenter(corner.Touches[j]);
			if (neighbor.CenterData.bIsOcean)
			{
				numOcean++;
			}
			else if (!neighbor.CenterData.bIsWater)
			{
				numLand++;
			}
		}
		corner.CornerData.bIsOcean = numOcean == corner.Touches.Num();
		corner.CornerData.bIsCoast = numOcean > 0 && numLand > 0;
		corner.CornerData.bIsWater = corner.CornerData.bIsBorder || (numLand != corner.Touches.Num() && !corner.CornerData.bIsCoast);
		UpdateCorner(corner);
	}
}

void AIslandMapGenerator::RedistributeElevations(TArray<int32> landCorners)
{
	TArray<FMapCorner> mapCorners;

	for (int i = 0; i < landCorners.Num(); i++)
	{
		mapCorners.Add(GetCorner(landCorners[i]));
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

		// Now we have to solve for x, given the known y.
		//  *  y = 1 - (1-x)^2
		//  *  y = 1 - (1 - 2x + x^2)
		//  *  y = 2x - x^2
		//  *  x^2 - 2x + y = 0
		// From this we can use the quadratic equation to get:
		float x = FMath::Sqrt(IslandData.ScaleFactor) - FMath::Sqrt(IslandData.ScaleFactor*(1.0f - y));
		if (x > maxElevation)
		{
			maxElevation = x;
		}
		FMapCorner corner = GetCorner(mapCorners[i].Index);
		corner.CornerData.Elevation = x;
		UpdateCorner(corner);
	}

	for (int i = 0; i < mapCorners.Num(); i++)
	{
		FMapCorner corner = GetCorner(mapCorners[i].Index);
		corner.CornerData.Elevation /= maxElevation;
		UpdateCorner(corner);
	}
}

void AIslandMapGenerator::FlattenWaterElevations()
{
	for (int i = 0; i < GetCornerNum(); i++)
	{
		FMapCorner corner = GetCorner(i);
		if (corner.CornerData.bIsOcean || corner.CornerData.bIsCoast)
		{
			corner.CornerData.Elevation = 0.0f;
		}
		UpdateCorner(corner);
	}
}

void AIslandMapGenerator::AssignPolygonElevations()
{
	float topElevation = -1.0f;
	FMapCenter topCenter;
	for (int i = 0; i < GetCenterNum(); i++)
	{
		float sumElevation = 0.0f;
		FMapCenter center = GetCenter(i);
		for (int32 j = 0; j < center.Corners.Num(); j++)
		{
			FMapCorner corner = GetCorner(center.Corners[j]);
			sumElevation += corner.CornerData.Elevation;
		}
		float elevation = sumElevation / center.Corners.Num();
		if (elevation > topElevation)
		{
			topCenter = center;
			topElevation = elevation;
		}
		center.CenterData.Elevation = elevation;
		UpdateCenter(center);
	}

	// The center at the top of the map gets turned into the volcano's caldera
	topCenter.CenterData.Tags.Add(UPolygonMap::TAG_Volcano);
	topCenter.CenterData.Tags.Add(UPolygonMap::TAG_VolcanoCaldera);
	topCenter.CenterData.Elevation *= 0.25f;

	/*for (int i = 0; i < topCenter.Corners.Num(); i++)
	{
	FMapCorner topCorner = GetCorner(topCenter.Corners[i]);
	calderaHeight = FMath::Min(calderaHeight, topCorner.CornerData.Elevation);
	}*/
	for (int i = 0; i < topCenter.Corners.Num(); i++)
	{
		FMapCorner topCorner = GetCorner(topCenter.Corners[i]);
		topCenter.CenterData.Tags.Add(UPolygonMap::TAG_Volcano);
		topCenter.CenterData.Tags.Add(UPolygonMap::TAG_VolcanoCaldera);
		//topCorner.CornerData.Elevation *= 0.4f;
		UpdateCorner(topCorner);
	}

	// All neighboring centers are still part of the volcano, but not part of the caldera
	for (int i = 0; i < topCenter.Neighbors.Num(); i++)
	{
		FMapCenter neighbor = GetCenter(topCenter.Neighbors[i]);
		topCenter.CenterData.Tags.Add(UPolygonMap::TAG_Volcano);
		for (int j = 0; j < neighbor.Corners.Num(); j++)
		{
			FMapCorner neighborCorner = GetCorner(neighbor.Corners[j]);
			if (neighborCorner.CornerData.Tags.Contains(UPolygonMap::TAG_Volcano))
			{
				continue;
			}
			topCenter.CenterData.Tags.Add(UPolygonMap::TAG_Volcano);
			UpdateCorner(neighborCorner);
		}
		UpdateCenter(neighbor);
	}

	UpdateCenter(topCenter);
}



void AIslandMapGenerator::AssignMoisture()
{
	// Moisture distribution
	// Determine downslope paths.
	CalculateDownslopes();

	// Determine watersheds: for every corner, where does it flow
	// out into the ocean? 
	CalculateWatersheds();

	// Create rivers.
	CreateRivers();

	// Determine moisture at corners, starting at rivers
	// and lakes, but not oceans. Then redistribute
	// moisture to cover the entire range evenly from 0.0
	// to 1.0. Then assign polygon moisture as the average
	// of the corner moisture.
	AssignCornerMoisture();
	RedistributeMoisture(MapGraph->FindLandCorners());
	AssignPolygonMoisture();
}

void AIslandMapGenerator::CalculateDownslopes()
{
	for (int i = 0; i < GetCornerNum(); i++)
	{
		FMapCorner corner = GetCorner(i);
		int32 downslopeIndex = corner.Index;
		for (int j = 0; j < corner.Adjacent.Num(); j++)
		{
			FMapCorner adjacent = GetCorner(corner.Adjacent[j]);
			if (adjacent.CornerData.Elevation <= GetCorner(downslopeIndex).CornerData.Elevation)
			{
				downslopeIndex = adjacent.Index;
			}
		}
		corner.Downslope = downslopeIndex;
		UpdateCorner(corner);
	}
}

void AIslandMapGenerator::CalculateWatersheds()
{
	// Initially the watershed pointer points downslope one step.  
	for (int i = 0; i < GetCornerNum(); i++)
	{
		FMapCorner corner = GetCorner(i);
		corner.Watershed = corner.Index;
		if (!corner.CornerData.bIsOcean && !corner.CornerData.bIsCoast)
		{
			corner.Watershed = corner.Downslope;
		}
		UpdateCorner(corner);
	}

	// Follow the downslope pointers to the coast. Limit to 100
	// iterations although most of the time with numPoints==2000 it
	// only takes 20 iterations because most points are not far from
	// a coast.
	for (int i = 0; i < 100; i++)
	{
		bool bChanged = false;
		for (int j = 0; j < GetCornerNum(); j++)
		{
			FMapCorner corner = GetCorner(j);
			FMapCorner watershed = GetCorner(corner.Watershed);
			if (!corner.CornerData.bIsOcean && !corner.CornerData.bIsCoast && !watershed.CornerData.bIsCoast)
			{
				FMapCorner downstreamWatershed = GetCorner(watershed.Watershed);
				if (!downstreamWatershed.CornerData.bIsOcean)
				{
					corner.Watershed = downstreamWatershed.Index;
					UpdateCorner(corner);
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
	for (int i = 0; i < GetCornerNum(); i++)
	{
		FMapCorner corner = GetCorner(i);
		FMapCorner watershed = GetCorner(corner.Watershed);
		watershed.WatershedSize += 1;
		UpdateCorner(watershed);
	}
}

void AIslandMapGenerator::CreateRivers()
{
	for (int i = 0; i < IslandData.Size / 2; i++)
	{
		int cornerIndex = RandomGenerator.RandRange(0, GetCornerNum() - 1);
		FMapCorner riverSource = GetCorner(cornerIndex);
		if (riverSource.CornerData.bIsOcean || riverSource.CornerData.Tags.Contains(UPolygonMap::TAG_Volcano))
		{
			continue;
		}

		while (!riverSource.CornerData.bIsCoast)
		{
			FMapCorner downslopeCorner = GetCorner(riverSource.Downslope);
			if (downslopeCorner.Index == riverSource.Index || downslopeCorner.Index < 0)
			{
				break;
			}
			FMapEdge edge = FindEdgeFromCorners(riverSource, downslopeCorner);
			edge.RiverVolume++;
			riverSource.RiverSize++;
			riverSource.CornerData.bIsRiver = true;
			riverSource.CornerData.Elevation -= 0.05f;
			downslopeCorner.RiverSize++;
			downslopeCorner.CornerData.bIsRiver = true;
			downslopeCorner.CornerData.Elevation -= 0.05f;

			UpdateEdge(edge);
			UpdateCorner(riverSource);
			UpdateCorner(downslopeCorner);

			riverSource = downslopeCorner;
		}
	}
}

void AIslandMapGenerator::AssignCornerMoisture()
{
	TQueue<FMapCorner> moistureQueue;
	for (int i = 0; i < GetCornerNum(); i++)
	{
		FMapCorner corner = GetCorner(i);
		if ((corner.CornerData.bIsWater || corner.CornerData.bIsRiver) && !corner.CornerData.bIsOcean)
		{
			corner.CornerData.Moisture = corner.CornerData.bIsRiver ? FMath::Min(3.0f, (0.2f * corner.RiverSize)) : 1.0f;
			moistureQueue.Enqueue(corner);
		}
		else
		{
			corner.CornerData.Moisture = 0.0f;
			UpdateCorner(corner);
		}
	}

	while (!moistureQueue.IsEmpty())
	{
		FMapCorner corner;
		moistureQueue.Dequeue(corner);
		for (int i = 0; i < corner.Adjacent.Num(); i++)
		{
			FMapCorner neighbor = GetCorner(corner.Adjacent[i]);
			float newMoisture = corner.CornerData.Moisture * 0.9f;
			if (newMoisture > neighbor.CornerData.Moisture)
			{
				neighbor.CornerData.Moisture = newMoisture;
				moistureQueue.Enqueue(neighbor);
			}
		}
		UpdateCorner(corner);
	}

	// Saltwater
	for (int i = 0; i < GetCornerNum(); i++)
	{
		FMapCorner corner = GetCorner(i);
		if (corner.CornerData.bIsCoast || corner.CornerData.bIsOcean)
		{
			corner.CornerData.Moisture = 1.0f;
			UpdateCorner(corner);
		}
	}
}

void AIslandMapGenerator::RedistributeMoisture(TArray<int32> landCorners)
{
	float maxMoisture = -1.0f;
	for (int i = 0; i < landCorners.Num(); i++)
	{
		FMapCorner corner = GetCorner(landCorners[i]);
		if (corner.CornerData.Moisture > maxMoisture)
		{
			maxMoisture = corner.CornerData.Moisture;
		}
	}

	for (int i = 0; i < landCorners.Num(); i++)
	{
		FMapCorner corner = GetCorner(landCorners[i]);
		corner.CornerData.Moisture /= maxMoisture;
		UpdateCorner(corner);
	}
}

void AIslandMapGenerator::AssignPolygonMoisture()
{
	for (int i = 0; i < GetCenterNum(); i++)
	{
		FMapCenter center = GetCenter(i);
		float sumMoisture = 0.0f;
		for (int j = 0; j < center.Corners.Num(); j++)
		{
			FMapCorner corner = GetCorner(center.Corners[j]);
			sumMoisture = corner.CornerData.Moisture;
		}
		center.CenterData.Moisture = sumMoisture / center.Corners.Num();
		UpdateCenter(center);
	}
}

void AIslandMapGenerator::FinalizeAllPoints()
{
	for (int i = 0; i < GetCornerNum(); i++)
	{
		FMapCorner corner = GetCorner(i);
		corner.CornerData.Elevation = FMath::Clamp(corner.CornerData.Elevation, 0.0f, 1.0f);
		corner.CornerData.Moisture = FMath::Clamp(corner.CornerData.Moisture, 0.0f, 1.0f);

		if (!corner.CornerData.bIsOcean)
		{
			corner.CornerData.Elevation = 0.1f + (corner.CornerData.Elevation * 0.9f);
		}
		UpdateCorner(corner);
	}

	for (int i = 0; i < GetCenterNum(); i++)
	{
		FMapCenter center = GetCenter(i);
		center.CenterData.Elevation = FMath::Clamp(center.CenterData.Elevation, 0.0f, 1.0f);
		center.CenterData.Moisture = FMath::Clamp(center.CenterData.Moisture, 0.0f, 1.0f);

		if (!center.CenterData.bIsOcean)
		{
			center.CenterData.Elevation = 0.1f + (center.CenterData.Elevation * 0.9f);
		}
		UpdateCenter(center);
	}

	// Compile to get ready to make pixels
	MapGraph->CompileMapData();
}