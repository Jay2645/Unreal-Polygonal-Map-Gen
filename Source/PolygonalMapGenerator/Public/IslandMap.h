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
#include "IslandMeshBuilder.h"
#include "IslandBiome.h"
#include "IslandElevation.h"
#include "IslandMoisture.h"
#include "IslandRivers.h"
#include "IslandWater.h"

#include "IslandMap.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnIslandGenerationComplete);

UCLASS()
class POLYGONALMAPGENERATOR_API AIslandMap : public AActor
{
	GENERATED_BODY()
	friend class UIslandMapUtils;

protected:
	UPROPERTY()
	TArray<bool> r_water;
	UPROPERTY()
	TArray<bool> r_ocean;
	UPROPERTY()
	TArray<bool> r_coast;
	UPROPERTY()
	TArray<float> r_elevation;
	UPROPERTY()
	TArray<int32> r_waterdistance;
	UPROPERTY()
	TArray<float> r_moisture;
	UPROPERTY()
	TArray<float> r_temperature;
	UPROPERTY()
	TArray<FBiomeData> r_biome;

	UPROPERTY()
	TArray<int32> t_coastdistance;
	UPROPERTY()
	TArray<float> t_elevation;
	UPROPERTY()
	TArray<FSideIndex> t_downslope_s;
	UPROPERTY()
	TArray<int32> s_flow;
	UPROPERTY()
	TArray<FTriangleIndex> spring_t;
	UPROPERTY()
	TArray<FTriangleIndex> river_t;

	// Note -- will be compiled when GetVoronoiPolygons is first called.
	// This will take a long time to compile and use a lot of memory. Use with caution!
	UPROPERTY()
	TArray<FIslandPolygon> VoronoiPolygons;

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
	const UIslandMeshBuilder* PointGenerator;
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

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Map")
	TArray<URiver*> CreatedRivers;


	UPROPERTY(BlueprintAssignable)
	FOnIslandGenerationComplete OnIslandPointGenerationComplete;
	UPROPERTY(BlueprintAssignable)
	FOnIslandGenerationComplete OnIslandWaterGenerationComplete;
	UPROPERTY(BlueprintAssignable)
	FOnIslandGenerationComplete OnIslandElevationGenerationComplete;
	UPROPERTY(BlueprintAssignable)
	FOnIslandGenerationComplete OnIslandRiverGenerationComplete;
	UPROPERTY(BlueprintAssignable)
	FOnIslandGenerationComplete OnIslandMoistureGenerationComplete;
	UPROPERTY(BlueprintAssignable)
	FOnIslandGenerationComplete OnIslandBiomeGenerationComplete;
	UPROPERTY(BlueprintAssignable)
	FOnIslandGenerationComplete OnIslandGenerationComplete;

public:	
	// Sets default values for this actor's properties
	AIslandMap();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Procedural Generation|Island Generation")
	void OnPointGenerationComplete();
	virtual void OnPointGenerationComplete_Implementation();
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Procedural Generation|Island Generation")
	void OnWaterGenerationComplete();
	virtual void OnWaterGenerationComplete_Implementation();
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Procedural Generation|Island Generation")
	void OnElevationGenerationComplete();
	virtual void OnElevationGenerationComplete_Implementation();
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Procedural Generation|Island Generation")
	void OnRiverGenerationComplete();
	virtual void OnRiverGenerationComplete_Implementation();
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Procedural Generation|Island Generation")
	void OnMoistureGenerationComplete();
	virtual void OnMoistureGenerationComplete_Implementation();
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Procedural Generation|Island Generation")
	void OnBiomeGenerationComplete();
	virtual void OnBiomeGenerationComplete_Implementation();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Procedural Generation|Island Generation")
	void OnIslandGenComplete();
	virtual void OnIslandGenComplete_Implementation();

public:
	// Creates the island using all the current parameters.
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Procedural Generation|Island Generation")
	void GenerateIsland();
	virtual void GenerateIsland_Implementation();

	// WARNING: This will take a long time to compile and will use a lot of memory.
	// Use with caution!
	UFUNCTION()
	TArray<FIslandPolygon>& GetVoronoiPolygons();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Procedural Generation|Island Generation|Water")
	TArray<bool>& GetWaterRegions();
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Procedural Generation|Island Generation|Water")
	bool IsPointWater(FPointIndex Region) const;
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Procedural Generation|Island Generation|Ocean")
	TArray<bool>& GetOceanRegions();
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Procedural Generation|Island Generation|Ocean")
	bool IsPointOcean(FPointIndex Region) const;
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Procedural Generation|Island Generation|Ocean")
	TArray<bool>& GetCoastalRegions();
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Procedural Generation|Island Generation|Ocean")
	bool IsPointCoast(FPointIndex Region) const;
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Procedural Generation|Island Generation|Elevation")
	TArray<float>& GetRegionElevations();
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Procedural Generation|Island Generation|Elevation")
	float GetPointElevation(FPointIndex Region) const;
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Procedural Generation|Island Generation|Moisture")
	TArray<int32>& GetRegionWaterDistance();
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Procedural Generation|Island Generation|Moisture")
	int32 GetPointWaterDistance(FPointIndex Region) const;
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Procedural Generation|Island Generation|Moisture")
	TArray<float>& GetRegionMoisture();
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Procedural Generation|Island Generation|Moisture")
	float GetPointMoisture(FPointIndex Region) const;
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Procedural Generation|Island Generation|Temperature")
	TArray<float>& GetRegionTemperature();
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Procedural Generation|Island Generation|Temperature")
	float GetPointTemperature(FPointIndex Region) const;
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Procedural Generation|Island Generation|Biomes")
	TArray<FBiomeData>& GetRegionBiomes();
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Procedural Generation|Island Generation|Moisture")
	FBiomeData GetPointBiome(FPointIndex Region) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Procedural Generation|Island Generation|Ocean")
	TArray<int32>& GetTriangleCoastDistances();
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Procedural Generation|Island Generation|Ocean")
	int32 GetTriangleCoastDistance(FTriangleIndex Triangle) const;
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Procedural Generation|Island Generation|Elevation")
	TArray<float>& GetTriangleElevations();
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Procedural Generation|Island Generation|Elevation")
	float GetTriangleElevation(FTriangleIndex Triangle) const;
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Procedural Generation|Island Generation|Rivers")
	TArray<FSideIndex>& GetTriangleDownslopes();
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Procedural Generation|Island Generation|Rivers")
	TArray<int32>& GetSideFlow();
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Procedural Generation|Island Generation|Rivers")
	TArray<FTriangleIndex>& GetSpringTriangles();
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Procedural Generation|Island Generation|Rivers")
	bool IsTriangleSpring(FTriangleIndex Triangle) const;
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Procedural Generation|Island Generation|Rivers")
	TArray<FTriangleIndex>& GetRiverTriangles();
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Procedural Generation|Island Generation|Rivers")
	bool IsTriangleRiver(FTriangleIndex Triangle) const;
};
