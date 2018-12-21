// Copyright 2018 Schemepunk Studios

#include "IslandMapMesh.h"

AIslandMapMesh::AIslandMapMesh()
{
	ZScale = 10000.0f;

	MapMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("GeneratedMesh"));
	RootComponent = MapMesh;
	// New in UE 4.17, multi-threaded PhysX cooking.
	MapMesh->bUseAsyncCooking = true;
}

void AIslandMapMesh::BeginPlay()
{
	Super::BeginPlay();

	CreateDynmaicMesh();
}

void AIslandMapMesh::CreateDynmaicMesh()
{
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
		vertices[r] = FVector(points[r].X, points[r].Y, r_elevation[r] * ZScale);
		vertexColors[r] = FLinearColor(0.75, 0.75, 0.75, 1.0);
	}
	for (FTriangleIndex t = 0; t < triangles.Num(); t++)
	{
		triangles[t] = (int32)rawMesh.DelaunayTriangles[t];
		normals[t] = FVector(1.0f, 0.0f, 0.0f);
		uv0[t] = FVector2D::ZeroVector;
		tangents[t] = FProcMeshTangent(0, 1, 0);
	}

	MapMesh->CreateMeshSection_LinearColor(0, vertices, triangles, normals, uv0, vertexColors, tangents, true);

	// Enable collision data
	MapMesh->ContainsPhysicsTriMeshData(true);
}
