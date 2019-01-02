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
#include "IslandMapUtils.h"
#include "RandomSampling/SimplexNoise.h"
#include "DrawDebugHelpers.h"
#include "IslandMap.h"

void UIslandMapUtils::RandomShuffle(TArray<FTriangleIndex>& OutShuffledArray, FRandomStream& Rng)
{
	for (int i = OutShuffledArray.Num() - 1; i > 0; i--)
	{
		int32 j = Rng.RandRange(0, i);
		int32 swap = OutShuffledArray[i];
		OutShuffledArray[i] = OutShuffledArray[j];
		OutShuffledArray[j] = swap;
	}
}

float UIslandMapUtils::FBMNoise(const TArray<float>& Amplitudes, const FVector2D& Position)
{
	float sum = 0.0f;
	float sumOfAmplitudes = 0.0f;
	FCustomSimplexNoise noise;
	for (size_t octave = 0; octave < Amplitudes.Num(); octave++)
	{
		size_t frequency = ((size_t)1) << octave;
		sum += Amplitudes[octave] * noise.fractal(octave, Position * frequency);
		sumOfAmplitudes += Amplitudes[octave];
	}

	if (sumOfAmplitudes == 0.0f)
	{
		return 0.0f;
	}
	return sum / sumOfAmplitudes;
}

FBiomeData UIslandMapUtils::GetBiome(const UDataTable* BiomeData, bool bIsOcean, bool bIsWater, bool bIsCoast, float Temperature, float Moisture)
{
	if (BiomeData == NULL)
	{
		UE_LOG(LogMapGen, Error, TEXT("Passed in an empty Biome Data table! Can't determine any biomes."));
		return FBiomeData();
	}

	Moisture = FMath::Clamp(Moisture, 0.0f, 1.0f);
	Temperature = FMath::Clamp(Temperature, 0.0f, 1.0f);

	TArray<FBiomeData> possibleBiomes;
	for (auto it : BiomeData->GetRowMap())
	{
		FName rowName = it.Key;
		if (it.Value == NULL)
		{
			// Should never happen
			checkNoEntry();
		}
		possibleBiomes.Add(*((FBiomeData*)it.Value));
	}

	TArray<FBiomeData> newPossibleBiomes;

	// Check ocean
	if (bIsOcean)
	{
		for (FBiomeData biome : possibleBiomes)
		{
			if (biome.bIsOcean)
			{
				newPossibleBiomes.Add(biome);
			}
		}
		possibleBiomes = newPossibleBiomes;
		if (possibleBiomes.Num() == 1)
		{
			return possibleBiomes[0];
		}
		else if (possibleBiomes.Num() == 0)
		{
			UE_LOG(LogMapGen, Error, TEXT("Could not find any ocean biomes!"));
			return FBiomeData();
		}
		// Recycle the new biomes array
		newPossibleBiomes.Empty();
	}
	else
	{
		for (FBiomeData biome : possibleBiomes)
		{
			if (!biome.bIsOcean)
			{
				newPossibleBiomes.Add(biome);
			}
		}
		possibleBiomes = newPossibleBiomes;
		if (possibleBiomes.Num() == 1)
		{
			return possibleBiomes[0];
		}
		else if (possibleBiomes.Num() == 0)
		{
			UE_LOG(LogMapGen, Error, TEXT("Could not find any ocean biomes!"));
			return FBiomeData();
		}
		// Recycle the new biomes array
		newPossibleBiomes.Empty();
	}

	// Check water
	if (bIsWater)
	{
		for (FBiomeData biome : possibleBiomes)
		{
			if (biome.bIsWater)
			{
				newPossibleBiomes.Add(biome);
			}
		}
		possibleBiomes = newPossibleBiomes;
		if (possibleBiomes.Num() == 1)
		{
			return possibleBiomes[0];
		}
		else if (possibleBiomes.Num() == 0)
		{
			UE_LOG(LogMapGen, Error, TEXT("Could not find any water biomes!"));
			return FBiomeData();
		}
		newPossibleBiomes.Empty();
	}
	else
	{
		for (FBiomeData biome : possibleBiomes)
		{
			if (!biome.bIsWater)
			{
				newPossibleBiomes.Add(biome);
			}
		}
		possibleBiomes = newPossibleBiomes;
		if (possibleBiomes.Num() == 1)
		{
			return possibleBiomes[0];
		}
		else if (possibleBiomes.Num() == 0)
		{
			UE_LOG(LogMapGen, Error, TEXT("Could not find any water biomes!"));
			return FBiomeData();
		}
		newPossibleBiomes.Empty();
	}

	// Check coast
	if (bIsCoast)
	{
		for (FBiomeData biome : possibleBiomes)
		{
			if (biome.bIsCoast)
			{
				newPossibleBiomes.Add(biome);
			}
		}
		possibleBiomes = newPossibleBiomes;
		if (possibleBiomes.Num() == 1)
		{
			return possibleBiomes[0];
		}
		else if (possibleBiomes.Num() == 0)
		{
			UE_LOG(LogMapGen, Error, TEXT("Could not find any coastal biomes!"));
			return FBiomeData();
		}
		newPossibleBiomes.Empty();
	}
	else
	{
		for (FBiomeData biome : possibleBiomes)
		{
			if (!biome.bIsCoast)
			{
				newPossibleBiomes.Add(biome);
			}
		}
		possibleBiomes = newPossibleBiomes;
		if (possibleBiomes.Num() == 1)
		{
			return possibleBiomes[0];
		}
		else if (possibleBiomes.Num() == 0)
		{
			UE_LOG(LogMapGen, Error, TEXT("Could not find any coastal biomes!"));
			return FBiomeData();
		}
		newPossibleBiomes.Empty();
	}
	
	// Check moisture
	for (FBiomeData biome : possibleBiomes)
	{
		if ((biome.MinMoisture < Moisture || biome.MinMoisture == 0.0f && Moisture == 0.0f) && biome.MaxMoisture >= Moisture)
		{
			newPossibleBiomes.Add(biome);
		}
	}
	possibleBiomes = newPossibleBiomes;
	if (possibleBiomes.Num() == 1)
	{
		return possibleBiomes[0];
	}
	else if (possibleBiomes.Num() == 0)
	{
		UE_LOG(LogMapGen, Error, TEXT("Could not find any biomes with moisture %f! Is ocean? %d Is coast? %d Is water? %d"), Moisture, (uint8)bIsOcean, (uint8)bIsCoast, (uint8)bIsWater);
		return FBiomeData();
	}
	newPossibleBiomes.Empty();

	// Check temperature
	for (FBiomeData biome : possibleBiomes)
	{
		if ((biome.MinTemperature < Temperature || biome.MinTemperature == 0.0f && Temperature == 0.0f) && biome.MaxTemperature >= Temperature)
		{
			newPossibleBiomes.Add(biome);
		}
	}
	possibleBiomes = newPossibleBiomes;
	if (possibleBiomes.Num() == 1)
	{
		return possibleBiomes[0];
	}
	else if (possibleBiomes.Num() == 0)
	{
		UE_LOG(LogMapGen, Error, TEXT("Could not find any biomes with temperature %f! Is ocean? %d; Is coast? %d; Is water? %d; Moisture: %f"), Temperature, (uint8)bIsOcean, (uint8)bIsCoast, (uint8)bIsWater, Moisture);
		return FBiomeData();
	}

	UE_LOG(LogMapGen, Warning, TEXT("Had %d possible candidates for temperature %f and moisture %f."), possibleBiomes.Num(), Temperature, Moisture);

	return possibleBiomes[0];
}

void UIslandMapUtils::DrawDelaunayFromMap(AIslandMap* Map)
{
	if (Map == NULL)
	{
		return;
	}
	DrawDelaunayMesh(Map, Map->Mesh, Map->r_elevation, Map->s_flow, Map->CreatedRivers, Map->t_elevation, Map->r_biome);
}

void UIslandMapUtils::DrawVoronoiFromMap(class AIslandMap* Map)
{
	if (Map == NULL)
	{
		return;
	}
	DrawVoronoiMesh(Map, Map->Mesh, Map->GetVoronoiPolygons(), Map->s_flow, Map->CreatedRivers, Map->t_elevation);
}

void UIslandMapUtils::DrawDelaunayMesh(AActor* Context, UTriangleDualMesh* Mesh, const TArray<float>& RegionElevations, const TArray<int32>& SideFlow, const TArray<URiver*>& Rivers, const TArray<float> &TriangleElevations, const TArray<FBiomeData>& RegionBiomes)
{
	if (Context == NULL || Mesh == NULL)
	{
		return;
	}
#if !UE_BUILD_SHIPPING
	FDateTime startTime = FDateTime::UtcNow();
#endif

	UWorld* world = Context->GetWorld();
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
			float pZCoord = RegionElevations.IsValidIndex(first) ? RegionElevations[first] : -1000.0f;
			float qZCoord = RegionElevations.IsValidIndex(second) ? RegionElevations[second] : -1000.0f;
			FVector pVector = FVector(p.X, p.Y, pZCoord * 10000);
			FVector qVector = FVector(q.X, q.Y, qZCoord * 10000);
			FLinearColor color = FMath::Lerp(RegionBiomes[first].DebugColor.ReinterpretAsLinear(), RegionBiomes[second].DebugColor.ReinterpretAsLinear(), 0.5f);
			DrawDebugLine(world, pVector, qVector, color.ToFColor(false), false, 999.0f);
		}
	}

#if !UE_BUILD_SHIPPING
	FDateTime finishedTime = FDateTime::UtcNow();
	FTimespan difference = finishedTime - startTime;
	UE_LOG(LogMapGen, Log, TEXT("Drawing delaunay map took %f seconds."), difference.GetTotalSeconds());
#endif

	DrawRivers(Context, Mesh, Rivers, SideFlow, TriangleElevations);
}

void UIslandMapUtils::DrawVoronoiMesh(AActor* Context, UTriangleDualMesh* Mesh, const TArray<FIslandPolygon>& Polygons, const TArray<int32>& SideFlow, const TArray<URiver*>& Rivers, const TArray<float>& TriangleElevations)
{
	if (Context == NULL)
	{
		return;
	}
#if !UE_BUILD_SHIPPING
	FDateTime startTime = FDateTime::UtcNow();
#endif

	UWorld* world = Context->GetWorld();
	for (int i = 0; i < Polygons.Num(); i++)
	{
		const FIslandPolygon& polygon = Polygons[i];
		for (int j = 0; j < polygon.VertexPoints.Num(); j++)
		{
			FVector point = polygon.VertexPoints[j];
			FTriangleIndex first = polygon.Vertices[j];
			FVector next = polygon.VertexPoints[(j + 1) % Polygons.Num()];
			FTriangleIndex second = polygon.Vertices[(j + 1) % Polygons.Num()];
			DrawDebugLine(world, point, next, polygon.Biome.DebugColor, false, 999.0f);
		}
	}

#if !UE_BUILD_SHIPPING
	FDateTime finishedTime = FDateTime::UtcNow();
	FTimespan difference = finishedTime - startTime;
	UE_LOG(LogMapGen, Log, TEXT("Drawing voronoi map took %f seconds."), difference.GetTotalSeconds());
#endif

	DrawRivers(Context, Mesh, Rivers, SideFlow, TriangleElevations);
}

void UIslandMapUtils::DrawRivers(AActor* Context, UTriangleDualMesh* Mesh, const TArray<URiver*>& Rivers, const TArray<int32>& SideFlow, const TArray<float> &TriangleElevations)
{
	if (Context == NULL || Mesh == NULL)
	{
		return;
	}

#if !UE_BUILD_SHIPPING
	FDateTime startTime = FDateTime::UtcNow();
#endif

	UWorld* world = Context->GetWorld();
	for (URiver* river : Rivers)
	{
		if (river->RiverTriangles.Num() <= 1)
		{
			UE_LOG(LogMapGen, Warning, TEXT("Created a very short river!"));
			continue;
		}
		for (int i = 0; i < river->RiverTriangles.Num() - 1; i++)
		{
			FTriangleIndex t1 = river->RiverTriangles[i];
			int32 flow = SideFlow[river->Downslopes[i]];

			FVector2D first2D = Mesh->t_pos(t1);
			float z1 = TriangleElevations.IsValidIndex(t1) ? TriangleElevations[t1] : -1000.0f;
			FVector first3D = FVector(first2D.X, first2D.Y, z1 * 10000);

			FTriangleIndex t2 = river->RiverTriangles[i + 1];
			FVector2D second2D = Mesh->t_pos(t2);
			float z2 = TriangleElevations.IsValidIndex(t2) ? TriangleElevations[t2] : -1000.0f;
			FVector second3D = FVector(second2D.X, second2D.Y, z2 * 10000);

			DrawDebugLine(world, first3D, second3D, FColor::Blue, false, 999.0f, (uint8)'\000', 100.0f * flow);
		}
	}

#if !UE_BUILD_SHIPPING
	FDateTime finishedTime = FDateTime::UtcNow();
	FTimespan difference = finishedTime - startTime;
	UE_LOG(LogMapGen, Log, TEXT("Drawing rivers took %f seconds."), difference.GetTotalSeconds());
#endif

}

void UIslandMapUtils::GenerateMesh(class AIslandMap* Map, UProceduralMeshComponent* MapMesh, float ZScale)
{
	if (Map == NULL)
	{
		return;
	}
	GenerateMapMeshMultiMaterial(Map->Mesh, MapMesh, ZScale, Map->r_elevation, Map->r_coast, Map->r_biome);
}

void UIslandMapUtils::GenerateMapMeshSingleMaterial(UTriangleDualMesh* Mesh, UProceduralMeshComponent* MapMesh, float ZScale, const TArray<float>& RegionElevation)
{
	if (Mesh == NULL || MapMesh == NULL)
	{
		return;
	}
	const TArray<FVector2D>& points = Mesh->GetPoints();
	const FDualMesh& rawMesh = Mesh->GetRawMesh();

	FMapMeshData meshData;

	meshData.Vertices.SetNumZeroed(points.Num());
	meshData.VertexColors.SetNum(meshData.Vertices.Num());
	meshData.Triangles.SetNumZeroed(rawMesh.DelaunayTriangles.Num());
	meshData.Normals.SetNumZeroed(meshData.Triangles.Num());
	meshData.UV0.SetNumZeroed(meshData.Triangles.Num());
	meshData.Tangents.SetNumZeroed(meshData.Triangles.Num());

	for (FPointIndex r = 0; r < meshData.Vertices.Num(); r++)
	{
		float z = Mesh->r_ghost(r) ? -10 * ZScale : RegionElevation[r] * ZScale;
		meshData.Vertices[r] = FVector(points[r].X, points[r].Y, z);
		meshData.VertexColors[r] = FLinearColor(0.75, 0.75, 0.75, 1.0);
	}
	for (FTriangleIndex t = 0; t < meshData.Triangles.Num(); t++)
	{
		meshData.Triangles[t] = (int32)rawMesh.DelaunayTriangles[t];
		FDelaunayTriangle triangle = UDelaunayHelper::ConvertTriangleIDToTriangle(rawMesh, t);
		FVector a = FVector(triangle.A.X, triangle.A.Y, RegionElevation[triangle.AIndex]);
		FVector b = FVector(triangle.B.X, triangle.B.Y, RegionElevation[triangle.BIndex]);
		FVector c = FVector(triangle.C.X, triangle.C.Y, RegionElevation[triangle.CIndex]);
		meshData.Normals[t] = FVector::CrossProduct(c - a, b - a).GetSafeNormal();
		meshData.UV0[t] = FVector2D::ZeroVector;
		meshData.Tangents[t] = FProcMeshTangent((a - b).GetSafeNormal(), true);
	}

	MapMesh->CreateMeshSection_LinearColor(0, meshData.Vertices, meshData.Triangles, meshData.Normals, meshData.UV0, meshData.VertexColors, meshData.Tangents, true);

	// Enable collision data
	MapMesh->ContainsPhysicsTriMeshData(true);
}

void UIslandMapUtils::GenerateMapMeshMultiMaterial(UTriangleDualMesh* Mesh, UProceduralMeshComponent* MapMesh, float ZScale, const TArray<float>& RegionElevation, const TArray<bool>& CostalRegions, const TArray<FBiomeData> RegionBiomes)
{
	if (Mesh == NULL || MapMesh == NULL)
	{
		return;
	}
	const TArray<FVector2D>& points = Mesh->GetPoints();
	const FDualMesh& rawMesh = Mesh->GetRawMesh();

	TMap<FName, FMapMeshData> meshLookup;
	TMap<FName, UMaterialInterface*> materialLookup;

	for (FTriangleIndex t = 0; t < rawMesh.DelaunayTriangles.Num(); t += 3)
	{
		// Get triangle
		FDelaunayTriangle triangle = UDelaunayHelper::ConvertTriangleIDToTriangle(rawMesh, t);		
		// Determine which biome to use
		// If we're on the boundary, use the boundary biome
		// If we're part coast, use the coast biome (this prevents jagged triangles along the water)
		// If 2+ points use the same biome, make the whole triangle that biome
		// Otherwise, just use point A's biome
		FBiomeData biome;
		if (Mesh->r_boundary(triangle.AIndex))
		{
			biome = RegionBiomes[triangle.AIndex];
		}
		else if (Mesh->r_boundary(triangle.BIndex))
		{
			biome = RegionBiomes[triangle.BIndex];
		}
		else if (Mesh->r_boundary(triangle.CIndex))
		{
			biome = RegionBiomes[triangle.CIndex];
		}
		else if (CostalRegions[triangle.AIndex])
		{
			// Coastal regions get handled after boundary regions
			// This way, the boundary remains the same no matter what
			biome = RegionBiomes[triangle.AIndex];
		}
		else if (CostalRegions[triangle.BIndex])
		{
			biome = RegionBiomes[triangle.BIndex];
		}
		else if (CostalRegions[triangle.CIndex])
		{
			biome = RegionBiomes[triangle.CIndex];
		}
		else if (RegionBiomes[triangle.AIndex].Tag == RegionBiomes[triangle.BIndex].Tag)
		{
			// Finally, handle it based on biomes
			biome = RegionBiomes[triangle.AIndex];
		}
		else if (RegionBiomes[triangle.BIndex].Tag == RegionBiomes[triangle.CIndex].Tag)
		{
			biome = RegionBiomes[triangle.BIndex];
		}
		else if (RegionBiomes[triangle.CIndex].Tag == RegionBiomes[triangle.AIndex].Tag)
		{
			biome = RegionBiomes[triangle.CIndex];
		}
		else
		{
			biome = RegionBiomes[triangle.AIndex];
		}

		// Grab mesh info from the biome
		FMapMeshData meshData;
		if (meshLookup.Contains(biome.Tag))
		{
			meshData = meshLookup[biome.Tag];
		}
		else
		{
			materialLookup.Add(biome.Tag, biome.BiomeMaterial);
			meshLookup.Add(biome.Tag, meshData);
		}

		// Create points
		float aZ = Mesh->r_ghost(triangle.AIndex) ? -10 * ZScale : RegionElevation[triangle.AIndex] * ZScale;
		float bZ = Mesh->r_ghost(triangle.BIndex) ? -10 * ZScale : RegionElevation[triangle.BIndex] * ZScale;
		float cZ = Mesh->r_ghost(triangle.CIndex) ? -10 * ZScale : RegionElevation[triangle.CIndex] * ZScale;
		FVector a = FVector(triangle.A.X, triangle.A.Y, aZ);
		FVector b = FVector(triangle.B.X, triangle.B.Y, bZ);
		FVector c = FVector(triangle.C.X, triangle.C.Y, cZ);

		// Add to triangle
		meshData.Triangles.Add(meshData.Vertices.Add(a));
		meshData.Triangles.Add(meshData.Vertices.Add(b));
		meshData.Triangles.Add(meshData.Vertices.Add(c));

		// Calculate the tangents of our triangle
		const FVector edge1 = b - c;
		const FVector edge2 = a - c;
		const FVector tangentX = edge1.GetSafeNormal();
		FVector tangentZ = (edge1 ^ edge2).GetSafeNormal();
		for (uint8 i = 0; i < 3; i++)
		{
			meshData.Tangents.Add(FProcMeshTangent(tangentX, false));
			meshData.Normals.Add(tangentZ);
			meshData.UV0.Add(FVector2D::ZeroVector);
		}

		// Vertex colors from the original biome data
		meshData.VertexColors.Add(RegionBiomes[triangle.AIndex].DebugColor.ReinterpretAsLinear());
		meshData.VertexColors.Add(RegionBiomes[triangle.BIndex].DebugColor.ReinterpretAsLinear());
		meshData.VertexColors.Add(RegionBiomes[triangle.CIndex].DebugColor.ReinterpretAsLinear());

		// Update the map with the latest version of the mesh
		meshLookup[biome.Tag] = meshData;
	}

	// Create the actual meshes
	uint8 index = 0;
	for (auto kvp : meshLookup)
	{
		FMapMeshData meshData = kvp.Value;
		MapMesh->CreateMeshSection_LinearColor(index, meshData.Vertices, meshData.Triangles, meshData.Normals, meshData.UV0, meshData.VertexColors, meshData.Tangents, true);
		if (materialLookup.Contains(kvp.Key) && materialLookup[kvp.Key] != NULL)
		{
			MapMesh->SetMaterial(index, materialLookup[kvp.Key]);
		}
		index++;
	}

	// Enable collision data
	MapMesh->ContainsPhysicsTriMeshData(true);
}
