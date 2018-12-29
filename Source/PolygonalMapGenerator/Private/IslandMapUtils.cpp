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
	DrawDelaunayMesh(Map, Map->Mesh, Map->r_elevation, Map->s_flow, Map->r_biome);
}

void UIslandMapUtils::DrawVoronoiFromMap(class AIslandMap* Map)
{
	DrawVoronoiMesh(Map, Map->Polygons);
}

void UIslandMapUtils::DrawDelaunayMesh(AActor* Context, UTriangleDualMesh* Mesh, const TArray<float>& r_elevation, const TArray<int32>& s_flow, const TArray<FBiomeData>& r_biome)
{
	if (Context == NULL || Mesh == NULL)
	{
		return;
	}

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
			float pZCoord = r_elevation.IsValidIndex(first) ? r_elevation[first] : -1000.0f;
			float qZCoord = r_elevation.IsValidIndex(second) ? r_elevation[second] : -1000.0f;
			FVector pVector = FVector(p.X, p.Y, pZCoord * 10000);
			FVector qVector = FVector(q.X, q.Y, qZCoord * 10000);

			int flow = FMath::Max(s_flow[e], s_flow[UDelaunayHelper::NextHalfEdge(e)]);
			if (flow == 0)
			{
				FLinearColor color = FMath::Lerp(r_biome[first].DebugColor.ReinterpretAsLinear(), r_biome[second].DebugColor.ReinterpretAsLinear(), 0.5f);
				DrawDebugLine(world, pVector, qVector, color.ToFColor(false), false, 999.0f);
			}
			else
			{
				DrawDebugLine(world, pVector, qVector, FColor::Blue, false, 999.0f, (uint8)'\000', 100.0f * flow);
			}
		}
	}
}

void UIslandMapUtils::DrawVoronoiMesh(AActor* Context, const TArray<FIslandPolygon>& Polygons)
{
	if (Context == NULL)
	{
		return;
	}

	UWorld* world = Context->GetWorld();
	for (int i = 0; i < Polygons.Num(); i++)
	{
		const FIslandPolygon& polygon = Polygons[i];
		for (int j = 0; j < polygon.VertexPoints.Num(); j++)
		{
			FVector point = polygon.VertexPoints[j];
			FTriangleIndex first = polygon.Vertices[j];
			FVector next;
			FTriangleIndex second;
			if (j == polygon.VertexPoints.Num() - 1)
			{
				next = polygon.VertexPoints[0];
				second = polygon.Vertices[0];
			}
			else
			{
				next = polygon.VertexPoints[j + 1];
				second = polygon.Vertices[j + 1];
			}
			DrawDebugLine(world, point, next, polygon.Biome.DebugColor, false, 999.0f);
		}
	}
}

void UIslandMapUtils::GenerateMesh(class AIslandMap* Map, UProceduralMeshComponent* MapMesh, float ZScale)
{
	if (Map == NULL)
	{
		return;
	}
	GenerateMapMesh(Map->Mesh, MapMesh, ZScale, Map->r_elevation);
}

void UIslandMapUtils::GenerateMapMesh(UTriangleDualMesh* Mesh, UProceduralMeshComponent* MapMesh, float ZScale, const TArray<float>& RegionElevation)
{
	if (Mesh == NULL || MapMesh == NULL)
	{
		return;
	}
	const TArray<FVector2D>& points = Mesh->GetPoints();
	const FDualMesh& rawMesh = Mesh->GetRawMesh();
	TArray<FVector> vertices;
	TArray<FLinearColor> vertexColors;
	TArray<int32> triangles;
	TArray<FVector> normals;
	TArray<FVector2D> uv0;
	TArray<FProcMeshTangent> tangents;

	vertices.SetNumZeroed(points.Num());
	vertexColors.SetNum(vertices.Num());
	triangles.SetNumZeroed(rawMesh.DelaunayTriangles.Num());
	normals.SetNumZeroed(triangles.Num());
	uv0.SetNumZeroed(triangles.Num());
	tangents.SetNumZeroed(triangles.Num());

	for (FPointIndex r = 0; r < vertices.Num(); r++)
	{
		float z = Mesh->r_ghost(r) ? -10 * ZScale : RegionElevation[r] * ZScale;
		vertices[r] = FVector(points[r].X, points[r].Y, z);
		vertexColors[r] = FLinearColor(0.75, 0.75, 0.75, 1.0);
	}
	for (FTriangleIndex t = 0; t < triangles.Num(); t++)
	{
		triangles[t] = (int32)rawMesh.DelaunayTriangles[t];
		FDelaunayTriangle triangle = UDelaunayHelper::ConvertTriangleIDToTriangle(rawMesh, t);
		FVector a = FVector(triangle.A.X, triangle.A.Y, RegionElevation[triangle.AIndex]);
		FVector b = FVector(triangle.B.X, triangle.B.Y, RegionElevation[triangle.BIndex]);
		FVector c = FVector(triangle.C.X, triangle.C.Y, RegionElevation[triangle.CIndex]);
		normals[t] = FVector::CrossProduct(c - a, b - a).GetSafeNormal();
		uv0[t] = FVector2D::ZeroVector;
		tangents[t] = FProcMeshTangent((a - b).GetSafeNormal(), true);
	}

	MapMesh->CreateMeshSection_LinearColor(0, vertices, triangles, normals, uv0, vertexColors, tangents, true);

	// Enable collision data
	MapMesh->ContainsPhysicsTriMeshData(true);
}