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

// Sets default values
AIslandMap::AIslandMap()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	Seed = 0;
	MapSize = FVector2D(1000.0f, 1000.0f);
	BoundarySpacing = 0;
	PoissonSpacing = 10.0f;
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
	IslandShapeAmplitudes.SetNum(Shape.Octaves);
	for (int i = 0; i < IslandShapeAmplitudes.Num(); i++)
	{
		IslandShapeAmplitudes[i] = FMath::Pow(Persistence, i);
	}

	Super::BeginPlay();
	
	UDualMeshBuilder* builder = NewObject<UDualMeshBuilder>();
	builder->Initialize(MapSize, BoundarySpacing);
	builder->AddPoisson(Rng, PoissonSpacing, PoissonSamples);
	Mesh = builder->Create();

	// Water
	Water->assign_r_water(r_water, Mesh, Shape);
	Water->assign_r_ocean(r_ocean, Mesh, r_water);

	// Elevation
	Elevation->assign_t_elevation(t_elevation, t_coastdistance, t_downslope_s, Mesh, r_ocean, r_water, DrainageRng);
	Elevation->redistribute_t_elevation(t_elevation, Mesh);
	Elevation->assign_r_elevation(r_elevation, Mesh, t_elevation, r_ocean);

	// Rivers
	spring_t = Rivers->find_spring_t(Mesh, r_water, t_elevation, t_downslope_s);
	UIslandMapUtils::RandomShuffle(spring_t, RiverRng);
	river_t.SetNum(NumRivers < spring_t.Num() ? NumRivers : spring_t.Num());
	for (int i = 0; i < river_t.Num(); i++)
	{
		river_t[i] = spring_t[i];
	}
	Rivers->assign_s_flow(s_flow, Mesh, t_downslope_s, river_t, t_elevation);

	// Moisture
	Moisture->assign_r_moisture(r_moisture, r_waterdistance, Mesh, r_water, Moisture->find_moisture_seeds_r(Mesh, s_flow, r_ocean, r_water));
	Moisture->redistribute_r_moisture(r_moisture, Mesh, r_water, BiomeBias.Rainfall, 1.0f + BiomeBias.Rainfall);

	// Biomes
	Biomes->assign_r_coast(r_coast, Mesh, r_ocean);
	Biomes->assign_r_temperature(r_temperature, Mesh, r_ocean, r_water, r_elevation, r_moisture, BiomeBias.NorthernTemperature, BiomeBias.SouthernTemperature);
	Biomes->assign_r_biome(r_biome, Mesh, r_ocean, r_water, r_coast, r_temperature, r_moisture);
}