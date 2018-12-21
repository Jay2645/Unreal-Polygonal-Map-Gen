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

#include "IslandMap.h"
#include "DualMeshBuilder.h"
#include "IslandMapUtils.h"
#include "DrawDebugHelpers.h"

// Sets default values
AIslandMap::AIslandMap()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	Seed = 0;
	MapSize = FVector2D(107500.0, 107500.0);
	PoissonSize = FVector2D(100000.0, 100000.0);
	BoundarySpacing = 1000;
	PoissonSpacing = 1075.0f;
	PoissonSamples = 30;
	NumRivers = 30;
}

// Called when the game starts or when spawned
void AIslandMap::BeginPlay()
{
	if (Water == NULL || Elevation == NULL || Rivers == NULL || Moisture == NULL || Biomes == NULL)
	{
		UE_LOG(LogMapGen, Error, TEXT("IslandMap not properly set up!"));
		return;
	}

	Rng = FRandomStream();
	Rng.Initialize(Seed);
	RiverRng = FRandomStream();
	RiverRng.Initialize(RiverSeed);
	DrainageRng = FRandomStream();
	DrainageRng.Initialize(DrainageSeed);

	Persistence = FMath::Pow(0.5f, 1.0 + Smoothing);
	Shape.Amplitudes.SetNum(Shape.Octaves);
	for (int i = 0; i < Shape.Amplitudes.Num(); i++)
	{
		Shape.Amplitudes[i] = FMath::Pow(Persistence, i);
	}

	Super::BeginPlay();
	
	UDualMeshBuilder* builder = NewObject<UDualMeshBuilder>();
	builder->Initialize(MapSize, BoundarySpacing);
	builder->AddPoisson(Rng, MapSize - PoissonSize, PoissonSpacing, PoissonSamples);
	Mesh = builder->Create();

	// Water
	Water->assign_r_water(r_water, Rng, Mesh, Shape);
	Water->assign_r_ocean(r_ocean, Mesh, r_water);

	UE_LOG(LogMapGen, Log, TEXT("Generated map water."));

	// Elevation
	Elevation->assign_t_elevation(t_elevation, t_coastdistance, t_downslope_s, Mesh, r_ocean, r_water, DrainageRng);
	//Elevation->redistribute_t_elevation(t_elevation, Mesh);
	Elevation->assign_r_elevation(r_elevation, Mesh, t_elevation, r_ocean);

	Draw();

	UE_LOG(LogMapGen, Log, TEXT("Generated map elevation."));
	return;

	// Rivers
	spring_t = Rivers->find_spring_t(Mesh, r_water, t_elevation, t_downslope_s);
	UE_LOG(LogMapGen, Log, TEXT("Created %d springs."), spring_t.Num());
	UIslandMapUtils::RandomShuffle(spring_t, RiverRng);
	river_t.SetNum(NumRivers < spring_t.Num() ? NumRivers : spring_t.Num());
	for (int i = 0; i < river_t.Num(); i++)
	{
		river_t[i] = spring_t[i];
	}
	UE_LOG(LogMapGen, Log, TEXT("Created %d rivers."), river_t.Num());
	Rivers->assign_s_flow(s_flow, Mesh, t_downslope_s, river_t);

	UE_LOG(LogMapGen, Log, TEXT("Generated map rivers."));

	// Moisture
	Moisture->assign_r_moisture(r_moisture, r_waterdistance, Mesh, r_water, Moisture->find_moisture_seeds_r(Mesh, s_flow, r_ocean, r_water));
	Moisture->redistribute_r_moisture(r_moisture, Mesh, r_water, BiomeBias.Rainfall, 1.0f + BiomeBias.Rainfall);

	UE_LOG(LogMapGen, Log, TEXT("Generated map moisture."));

	// Biomes
	Biomes->assign_r_coast(r_coast, Mesh, r_ocean);
	Biomes->assign_r_temperature(r_temperature, Mesh, r_ocean, r_water, r_elevation, r_moisture, BiomeBias.NorthernTemperature, BiomeBias.SouthernTemperature);
	Biomes->assign_r_biome(r_biome, Mesh, r_ocean, r_water, r_coast, r_temperature, r_moisture);

	UE_LOG(LogMapGen, Log, TEXT("Generated map biomes."));
}

void AIslandMap::Draw() const
{
	UWorld* world = GetWorld();
	const TArray<FSideIndex>& _halfedges = Mesh->GetHalfEdges();
	const FDualMesh& mesh = Mesh->GetRawMesh();
	const TArray<FVector2D>& _r_vertex = Mesh->GetPoints();
	for (FSideIndex e = 0; e < _halfedges.Num(); e++)
	{
		if (e < _halfedges[e])
		{
			FPointIndex first = UDelaunayHelper::GetPointIndexFromHalfEdge(mesh, e);
			FPointIndex second = UDelaunayHelper::GetPointIndexFromHalfEdge(mesh, UDelaunayHelper::NextHalfEdge(e));

			if (Mesh->r_ghost(first) || Mesh->r_ghost(second))
			{
				continue;
			}

			const FVector2D p = _r_vertex[first];
			const FVector2D q = _r_vertex[second];
			float pZCoord = r_elevation.IsValidIndex(first) ? r_elevation[first] : -1000.0f;
			float qZCoord = r_elevation.IsValidIndex(second) ? r_elevation[second] : -1000.0f;
			FVector pVector = FVector(p.X, p.Y, pZCoord * 10000);
			FVector qVector = FVector(q.X, q.Y, qZCoord * 10000);

			FColor color = FColor::Magenta;
			if (r_water.IsValidIndex(first) && r_water.IsValidIndex(second))
			{
				if (r_water[first] || r_water[second])
				{
					color = FColor::Blue;
				}
			}
			if (r_coast.IsValidIndex(first) && r_coast.IsValidIndex(second))
			{
				if (r_coast[first] || r_coast[second])
				{
					color = FColor::Yellow;
				}
			}

			DrawDebugLine(world, pVector, qVector, color, false, 999.0f);
		}
	}
}