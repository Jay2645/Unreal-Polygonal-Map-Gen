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

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"

#include "RandomSampling/SimplexNoise.h"
#include "TriangleDualMesh.h"

#include "IslandMapUtils.h"
#include "IslandBiome.h"
#include "IslandElevation.h"
#include "IslandMoisture.h"
#include "IslandRivers.h"
#include "IslandWater.h"

#include "IslandMap.generated.h"

UCLASS()
class POLYGONALMAPGENERATOR_API AIslandMap : public AActor
{
	GENERATED_BODY()

public:
	// The random seed to use for the island.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "RNG")
	int32 Seed;
	// Modifies how we calculate drainage.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "RNG")
	int32 DrainageSeed;
	// Modifies how we calculate drainage.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "RNG")
	int32 RiverSeed;
	// The size of our map, starting at (0, 0).
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Map")
	FVector2D MapSize;
	// The size of our map, starting at (0, 0).
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Map")
	FVector2D PoissonSize;
	// The amount of spacing on the edge of the map.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Edges", meta = (ClampMin = "0"))
	int32 BoundarySpacing;
	// How much spacing between Poisson points.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Points", meta = (ClampMin = "0.0"))
	float PoissonSpacing;
	// Maximum samples to generate each step.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Points", meta = (ClampMin = "0"))
	int32 PoissonSamples;
	// Modifies the types of biomes we produce.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Map")
	FBiomeBias BiomeBias;
	// Modifies the shape of the island we generate.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Map")
	FIslandShape Shape;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Edges", meta = (ClampMin = "0"))
	int32 NumRivers;
	// How "smooth" the island is.
	// Higher values are more smooth and create fewer bays and lakes.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Map", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float Smoothing;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Map")
	float Persistence;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Mesh")
	UTriangleDualMesh* Mesh;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "RNG")
	FRandomStream Rng;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "RNG")
	FRandomStream RiverRng;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "RNG")
	FRandomStream DrainageRng;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Map")
	const UIslandBiome* Biomes;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Map")
	const UIslandElevation* Elevation;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Map")
	const UIslandMoisture* Moisture;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Map")
	const UIslandRivers* Rivers;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Map")
	const UIslandWater* Water;

	UPROPERTY()
	TArray<bool> r_water;
	UPROPERTY()
	TArray<bool> r_ocean;
	UPROPERTY()
	TArray<int32> t_coastdistance;
	UPROPERTY()
	TArray<float> t_elevation;
	UPROPERTY()
	TArray<FSideIndex> t_downslope_s;
	UPROPERTY()
	TArray<float> r_elevation;
	UPROPERTY()
	TArray<int32> s_flow;
	UPROPERTY()
	TArray<int32> r_waterdistance;
	UPROPERTY()
	TArray<int32> r_moisture;
	UPROPERTY()
	TArray<bool> r_coast;
	UPROPERTY()
	TArray<int32> r_temperature;
	UPROPERTY()
	TArray<FBiomeData> r_biome;
	UPROPERTY()
	TArray<FTriangleIndex> spring_t;
	UPROPERTY()
	TArray<FTriangleIndex> river_t;

public:	
	// Sets default values for this actor's properties
	AIslandMap();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Procedural Generation|Island Generation")
	void GenerateIsland();
	virtual void GenerateIsland_Implementation();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Procedural Generation|Island Generation")
	void OnIslandGenComplete();
	virtual void OnIslandGenComplete_Implementation();

public:
	void Draw() const;
};
