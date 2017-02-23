// Original Work Copyright (c) 2010 Amit J Patel, Modified Work Copyright (c) 2016 Jay M Stevens

#include "PolygonalMapGeneratorPrivatePCH.h"
#include "Maps/MapDataHelper.h"
#include "MapDebugVisualizer.h"
#include "Maps/IslandMapGenerator.h"

AIslandMapGenerator::AIslandMapGenerator()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bTickEvenWhenPaused = true;
}

void AIslandMapGenerator::Tick(float deltaSeconds)
{
	Super::Tick(deltaSeconds);
	GenerateMap();
}

void AIslandMapGenerator::SetData(FIslandData islandData)
{
	IslandData = islandData;
}

void AIslandMapGenerator::ResetMap()
{
	if (IslandData.IslandType == NULL)
	{
		UE_LOG(LogWorldGen, Error, TEXT("IslandShape was null!"));
		IslandShape = NewObject<UIslandShape>(this, TEXT("Island Shape"));
	}
	else
	{
		IslandShape = NewObject<UIslandShape>(this, IslandData.IslandType, TEXT("Island Shape"));
	}
	if (IslandData.IslandPointSelector == NULL)
	{
		UE_LOG(LogWorldGen, Error, TEXT("PointSelector was null!"));
		PointSelector = NewObject<UPointGenerator>(this, TEXT("Point Generator"));
	}
	else
	{
		PointSelector = NewObject<UPointGenerator>(this, IslandData.IslandPointSelector, TEXT("Point Generator"));
	}

	if (IslandData.BiomeManager == NULL)
	{
		UE_LOG(LogWorldGen, Error, TEXT("Biome Manager was null!"));
		BiomeManager = NewObject<UBiomeManager>(this,TEXT("Biome Manager"));
	}
	else
	{
		BiomeManager = NewObject<UBiomeManager>(this,IslandData.BiomeManager,TEXT("Biome Manager"));
	}

	if (IslandData.ElevationDistributor == NULL)
	{
		UE_LOG(LogWorldGen, Error, TEXT("Elevation Distributor was null!"));
		ElevationDistributor = NewObject<UElevationDistributor>(this, TEXT("Elevation Distributor"));
	}
	else
	{
		ElevationDistributor = NewObject<UElevationDistributor>(this, IslandData.ElevationDistributor, TEXT("Elevation Distributor"));
	}

	if (IslandData.MoistureDistributor == NULL)
	{
		UE_LOG(LogWorldGen, Error, TEXT("Moisture Distributor was null!"));
		MoistureDistributor = NewObject<UMoistureDistributor>(this, TEXT("Moisture Distributor"));
	}
	else
	{
		MoistureDistributor = NewObject<UMoistureDistributor>(this, IslandData.MoistureDistributor, TEXT("Moisture Distributor"));
	}

	RandomGenerator.Initialize(IslandData.Seed);
}

void AIslandMapGenerator::CreateMap(const FIslandGeneratorDelegate onComplete)
{
	OnGenerationComplete = onComplete;
	AddMapSteps();
	GenerateMap();
}

void AIslandMapGenerator::GenerateMap()
{
	if (IslandGeneratorSteps.IsEmpty() && OnGenerationComplete.IsBound())
	{
		// Done with all steps, execute onComplete delegate
		OnGenerationComplete.Execute();
		OnGenerationComplete.Unbind();
		return;
	}

	FIslandGeneratorDelegate mapDelegate;
	IslandGeneratorSteps.Dequeue(mapDelegate);
	if (mapDelegate.IsBound())
	{
		mapDelegate.Execute();
		mapDelegate.Unbind();
	}
}

void AIslandMapGenerator::AddMapSteps_Implementation()
{
	UE_LOG(LogWorldGen, Log, TEXT("Generating a new map at %f."), GetWorld()->GetRealTimeSeconds());

	FIslandGeneratorDelegate resetMap;
	resetMap.BindDynamic(this, &AIslandMapGenerator::ResetMap);
	AddGenerationStep(resetMap);

	/*if (IslandShape == NULL || PointSelector == NULL)
	{
	UE_LOG(LogWorldGen, Error, TEXT("IslandShape or PointSelector were null!"));
	return;
	}*/

	// Initialization
	FIslandGeneratorDelegate initialization;
	initialization.BindDynamic(this, &AIslandMapGenerator::InitializeMap);
	AddGenerationStep(initialization);
	//InitializeMap();

	FIslandGeneratorDelegate buildGraph;
	buildGraph.BindDynamic(this, &AIslandMapGenerator::BuildGraph);
	AddGenerationStep(buildGraph);
	//BuildGraph();

	FIslandGeneratorDelegate assignElevation;
	assignElevation.BindDynamic(this, &AIslandMapGenerator::AssignElevation);
	AddGenerationStep(assignElevation);
	//AssignElevation();

	FIslandGeneratorDelegate assignMoisture;
	assignMoisture.BindDynamic(this, &AIslandMapGenerator::AssignMoisture);
	AddGenerationStep(assignMoisture);
	//AssignMoisture();

	FIslandGeneratorDelegate postProcess;
	postProcess.BindDynamic(this, &AIslandMapGenerator::DoPointPostProcess);
	AddGenerationStep(postProcess);

	// Package the map up to send to the voxel engine
	FIslandGeneratorDelegate finalizePoints;
	finalizePoints.BindDynamic(this, &AIslandMapGenerator::FinalizeAllPoints);
	AddGenerationStep(finalizePoints);
	//FinalizeAllPoints();

	//MapGraph->DrawDebugVoronoiGrid(IslandData.GameWorld);
	//MapGraph->DrawDebugDelaunayGrid(IslandData.GameWorld);
	//PixelMap->DrawDebugPixelGrid(IslandData.GameWorld);
}

void AIslandMapGenerator::AddGenerationStep(const FIslandGeneratorDelegate step)
{
	IslandGeneratorSteps.Enqueue(step);
}

void AIslandMapGenerator::ClearAllGenerationSteps()
{
	IslandGeneratorSteps.Empty();
}

void AIslandMapGenerator::InitializeMap()
{
	MapGraph = NewObject<UPolygonMap>();
	MapHeightmap = NewObject<UPolygonalMapHeightmap>();
	IslandShape->SetSeed(IslandData.Seed, IslandData.Size);
	PointSelector->InitializeSelector(IslandData.Size, IslandData.Seed);
}

void AIslandMapGenerator::BuildGraph()
{
	if (MapGraph == NULL)
	{
		return;
	}
	// Place points
	MapGraph->CreatePoints(PointSelector, IslandData.NumberOfPoints);

	// Build graph
	MapGraph->BuildGraph(IslandData.Size, IslandData.PolygonMapSettings);
	MapGraph->ImproveCorners();
}

UPolygonMap* AIslandMapGenerator::GetGraph() const
{
	return MapGraph;
}

UPolygonalMapHeightmap* AIslandMapGenerator::GetHeightmap() const
{
	return MapHeightmap;
}

int32 AIslandMapGenerator::GetCenterNum() const
{
	if (MapGraph == NULL)
	{
		return -1;
	}
	return MapGraph->GetCenterNum();
}
int32 AIslandMapGenerator::GetCornerNum() const
{
	if (MapGraph == NULL)
	{
		return -1;
	}
	return MapGraph->GetCornerNum();
}
int32 AIslandMapGenerator::GetEdgeNum() const
{
	if (MapGraph == NULL)
	{
		return -1;
	}
	return MapGraph->GetEdgeNum();
}

FMapCenter AIslandMapGenerator::GetCenter(const int32 index) const
{
	return MapGraph->GetCenter(index);
}
FMapCorner AIslandMapGenerator::GetCorner(const int32 index) const
{
	return MapGraph->GetCorner(index);
}
FMapEdge AIslandMapGenerator::GetEdge(const int32 index) const
{
	return MapGraph->GetEdge(index);
}
FMapEdge AIslandMapGenerator::FindEdgeFromCenters(const FMapCenter& v0, const FMapCenter& v1) const
{
	return MapGraph->FindEdgeFromCenters(v0, v1);
}

FMapEdge AIslandMapGenerator::FindEdgeFromCorners(const FMapCorner& v0, const FMapCorner& v1) const
{
	return MapGraph->FindEdgeFromCorners(v0, v1);
}

void AIslandMapGenerator::UpdateCenter(const FMapCenter& center)
{
	if (MapGraph == NULL)
	{
		return;
	}
	MapGraph->UpdateCenter(center);
}
void AIslandMapGenerator::UpdateCorner(const FMapCorner& corner)
{
	if (MapGraph == NULL)
	{
		return;
	}
	MapGraph->UpdateCorner(corner);
}
void AIslandMapGenerator::UpdateEdge(const FMapEdge& edge)
{
	if (MapGraph == NULL)
	{
		return;
	}
	MapGraph->UpdateEdge(edge);
}

void AIslandMapGenerator::AssignElevation()
{
	ElevationDistributor->SetGraph(GetGraph());
	MoistureDistributor->SetGraph(MapGraph, IslandData.Size);
	// Assign elevations
	ElevationDistributor->AssignCornerElevations(IslandShape, PointSelector->NeedsMoreRandomness(), RandomGenerator);
	// Determine polygon and corner type: ocean, coast, land.
	MoistureDistributor->AssignOceanCoastAndLand();

	// Change the overall distribution of elevations so that lower
	// elevations are more common than higher
	// elevations. Specifically, we want elevation X to have frequency
	// (1-X).  To do this we will sort the corners, then set each
	// corner to its desired elevation.
	ElevationDistributor->RedistributeElevations(MapGraph->FindLandCorners());
	ElevationDistributor->FlattenWaterElevations();
	ElevationDistributor->AssignPolygonElevations();
}


void AIslandMapGenerator::AssignMoisture()
{
	if (MapGraph == NULL)
	{
		return;
	}
	// Moisture distribution
	// Determine downslope paths.
	ElevationDistributor->CalculateDownslopes();

	// Determine watersheds: for every corner, where does it flow
	// out into the ocean? 
	MoistureDistributor->CalculateWatersheds();

	// Create rivers.
	MoistureDistributor->CreateRivers(RandomGenerator);

	// Determine moisture at corners, starting at rivers
	// and lakes, but not oceans. Then redistribute
	// moisture to cover the entire range evenly from 0.0
	// to 1.0. Then assign polygon moisture as the average
	// of the corner moisture.
	MoistureDistributor->AssignCornerMoisture();
	MoistureDistributor->RedistributeMoisture(MapGraph->FindLandCorners());
	MoistureDistributor->AssignPolygonMoisture();
}

void AIslandMapGenerator::DoPointPostProcess()
{
	// Intentionally left blank
}

void AIslandMapGenerator::FinalizeAllPoints()
{
	if (MapGraph == NULL)
	{
		return;
	}
	for (int i = 0; i < GetCornerNum(); i++)
	{
		FMapCorner corner = GetCorner(i);
		corner.CornerData.Elevation = FMath::Clamp(corner.CornerData.Elevation, 0.0f, 1.0f);
		corner.CornerData.Moisture = FMath::Clamp(corner.CornerData.Moisture, 0.0f, 1.0f);

		if (!UMapDataHelper::IsOcean(corner.CornerData))
		{
			corner.CornerData.Elevation = 0.1f + (corner.CornerData.Elevation * 0.9f);
		}

		FGameplayTag biome = BiomeManager->DetermineBiome(corner.CornerData);
		corner.CornerData.Biome = biome;
		UpdateCorner(corner);
	}

	for (int i = 0; i < GetCenterNum(); i++)
	{
		FMapCenter center = GetCenter(i);
		center.CenterData.Elevation = FMath::Clamp(center.CenterData.Elevation, 0.0f, 1.0f);
		center.CenterData.Moisture = FMath::Clamp(center.CenterData.Moisture, 0.0f, 1.0f);

		if (!UMapDataHelper::IsOcean(center.CenterData))
		{
			center.CenterData.Elevation = 0.1f + (center.CenterData.Elevation * 0.9f);
		}
		FGameplayTag biome = BiomeManager->DetermineBiome(center.CenterData);
		center.CenterData.Biome = biome;
		UpdateCenter(center);
	}

	// Compile to get ready to make heightmap pixels
	MapGraph->CompileMapData();
	// Make the initial heightmap
	MapHeightmap->CreateHeightmap(MapGraph,BiomeManager,MoistureDistributor, IslandData.Size);
}

void AIslandMapGenerator::DrawVoronoiGraph()
{
	if (MapGraph == NULL)
	{
		return;
	}
	UMapDebugVisualizer::DrawDebugVoronoiGrid(this, IslandData.PolygonMapSettings, MapGraph->Corners, MapGraph->Edges, IslandData.Size);
}

void AIslandMapGenerator::DrawDelaunayGraph()
{
	if (MapGraph == NULL)
	{
		return;
	}
	UMapDebugVisualizer::DrawDebugDelaunayGrid(this, IslandData.PolygonMapSettings, MapGraph->Centers, MapGraph->Edges, IslandData.Size);
}

void AIslandMapGenerator::DrawHeightmap(float PixelSize)
{
	if (MapHeightmap == NULL)
	{
		return;
	}
	UMapDebugVisualizer::DrawDebugPixelGrid(this, IslandData.PolygonMapSettings, MapHeightmap->GetMapData(), IslandData.Size, PixelSize);
}