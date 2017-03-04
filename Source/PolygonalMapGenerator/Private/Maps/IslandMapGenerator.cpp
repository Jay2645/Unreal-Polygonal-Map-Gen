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
	bCurrentStepIsDone = true;
	bHasGeneratedHeightmap = false;

	MapGraph = CreateDefaultSubobject<UPolygonMap>(TEXT("Polygon Map"));
}

void AIslandMapGenerator::Tick(float deltaSeconds)
{
	Super::Tick(deltaSeconds);
	ExecuteNextMapStep();
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
	MapStartGenerationTime = FPlatformTime::Seconds();
	OnGenerationComplete = onComplete;
	AddMapSteps();
	ExecuteNextMapStep();
}

void AIslandMapGenerator::ExecuteNextMapStep()
{
	if (!bCurrentStepIsDone)
	{
		return;
	}
	if (IslandGeneratorSteps.IsEmpty() && OnGenerationComplete.IsBound())
	{
		UE_LOG(LogWorldGen, Log, TEXT("Finished map generation. Total time to create map was %f seconds."), FPlatformTime::Seconds() - MapStartGenerationTime);
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
	UE_LOG(LogWorldGen, Log, TEXT("Generating a new map at %f."), FPlatformTime::Seconds());

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
	initialization.BindDynamic(this, &AIslandMapGenerator::InitializeMapClasses);
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

	FIslandGeneratorDelegate normalizePoints;
	normalizePoints.BindDynamic(this, &AIslandMapGenerator::NormalizePoints);
	AddGenerationStep(normalizePoints);

	FIslandGeneratorDelegate determineBiomes;
	determineBiomes.BindDynamic(this, &AIslandMapGenerator::DetermineBiomes);
	AddGenerationStep(determineBiomes);

	//FIslandGeneratorDelegate createHeightmap;
	//createHeightmap.BindDynamic(this, &AIslandMapGenerator::CreateHeightmap);
	//AddGenerationStep(createHeightmap);
}

void AIslandMapGenerator::AddGenerationStep(const FIslandGeneratorDelegate step)
{
	IslandGeneratorSteps.Enqueue(step);
}

void AIslandMapGenerator::ClearAllGenerationSteps()
{
	IslandGeneratorSteps.Empty();
}

void AIslandMapGenerator::InitializeMapClasses()
{
	CurrentGenerationTime = FPlatformTime::Seconds();

	//MapGraph = NewObject<UPolygonMap>();
	MapGraph->Corners.Empty();
	MapGraph->Edges.Empty();
	MapGraph->Centers.Empty();
	MapGraph->Points.Empty();

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
	CurrentGenerationTime = FPlatformTime::Seconds();
	// Place points
	MapGraph->CreatePoints(PointSelector, IslandData.NumberOfPoints);

	// Build graph
	MapGraph->BuildGraph(IslandData.Size, IslandData.PolygonMapSettings);
	MapGraph->ImproveCorners();
	UE_LOG(LogWorldGen, Log, TEXT("Created graph in %f seconds."), FPlatformTime::Seconds() - CurrentGenerationTime);
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
	CurrentGenerationTime = FPlatformTime::Seconds();

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

	UE_LOG(LogWorldGen, Log, TEXT("Elevations assigned in %f seconds."), FPlatformTime::Seconds() - CurrentGenerationTime);
}


void AIslandMapGenerator::AssignMoisture()
{
	if (MapGraph == NULL)
	{
		return;
	}
	CurrentGenerationTime = FPlatformTime::Seconds();

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

	UE_LOG(LogWorldGen, Log, TEXT("Moisture distributed in %f seconds."), FPlatformTime::Seconds() - CurrentGenerationTime);
}

void AIslandMapGenerator::DoPointPostProcess()
{
	// Intentionally left blank
}

void AIslandMapGenerator::NormalizePoints()
{
	if (MapGraph == NULL)
	{
		return;
	}
	CurrentGenerationTime = FPlatformTime::Seconds();

	for (int i = 0; i < GetCornerNum(); i++)
	{
		FMapCorner corner = GetCorner(i);
		corner.CornerData.Elevation = FMath::Clamp(corner.CornerData.Elevation, 0.0f, 1.0f);
		corner.CornerData.Moisture = FMath::Clamp(corner.CornerData.Moisture, 0.0f, 1.0f);

		if (!UMapDataHelper::IsOcean(corner.CornerData))
		{
			// If this point is not ocean, raise it above ocean level
			corner.CornerData.Elevation = FMath::Clamp(0.1f + (corner.CornerData.Elevation * 0.9f), 0.0f, 1.0f);
		}

		UpdateCorner(corner);
	}

	for (int i = 0; i < GetCenterNum(); i++)
	{
		FMapCenter center = GetCenter(i);
		center.CenterData.Elevation = FMath::Clamp(center.CenterData.Elevation, 0.0f, 1.0f);
		center.CenterData.Moisture = FMath::Clamp(center.CenterData.Moisture, 0.0f, 1.0f);

		if (!UMapDataHelper::IsOcean(center.CenterData))
		{
			// If this point is not ocean, raise it above ocean level
			center.CenterData.Elevation = FMath::Clamp(0.1f + (center.CenterData.Elevation * 0.9f), 0.0f, 1.0f);
		}
		UpdateCenter(center);
	}

	UE_LOG(LogWorldGen, Log, TEXT("Points normalized in %f seconds."), FPlatformTime::Seconds() - CurrentGenerationTime);
}

void AIslandMapGenerator::DetermineBiomes()
{
	if (MapGraph == NULL)
	{
		return;
	}

	CurrentGenerationTime = FPlatformTime::Seconds();
	
	for (int i = 0; i < GetCornerNum(); i++)
	{
		FMapCorner corner = GetCorner(i);
		FGameplayTag biome = BiomeManager->DetermineBiome(corner.CornerData);
		corner.CornerData.Biome = biome;
		UpdateCorner(corner);
	}

	for (int i = 0; i < GetCenterNum(); i++)
	{
		FMapCenter center = GetCenter(i);
		FGameplayTag biome = BiomeManager->DetermineBiome(center.CenterData);
		center.CenterData.Biome = biome;
		UpdateCenter(center);
	}

	UE_LOG(LogWorldGen, Log, TEXT("Biomes determined in %f seconds."), FPlatformTime::Seconds() - CurrentGenerationTime);
}
void AIslandMapGenerator::CreateHeightmap(const int32 HeightmapSize, const FIslandGeneratorDelegate OnHeightmapGenerationFinished)
{
	if (MapGraph == NULL)
	{
		return;
	}
	// Make the initial heightmap
	CurrentGenerationTime = FPlatformTime::Seconds();

	bHasGeneratedHeightmap = false;
	MapGraph->CompileMapData();

	OnHeightmapComplete = OnHeightmapGenerationFinished;

	FIslandGeneratorDelegate finalizationFinished;
	finalizationFinished.BindDynamic(this, &AIslandMapGenerator::OnHeightmapFinished);
	MapHeightmap->CreateHeightmap(MapGraph, BiomeManager, MoistureDistributor, HeightmapSize, finalizationFinished);
}

void AIslandMapGenerator::OnHeightmapFinished()
{
	bHasGeneratedHeightmap = true;
	if (OnHeightmapComplete.IsBound())
	{
		OnHeightmapComplete.Execute();
		OnHeightmapComplete.Unbind();
	}
	UE_LOG(LogWorldGen, Log, TEXT("Heightmap created in %f seconds."), FPlatformTime::Seconds() - CurrentGenerationTime);
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
	if (MapHeightmap == NULL || !bHasGeneratedHeightmap)
	{
		return;
	}
	UMapDebugVisualizer::DrawDebugPixelGrid(this, IslandData.PolygonMapSettings, MapHeightmap->GetMapData(), IslandData.Size, PixelSize);
}