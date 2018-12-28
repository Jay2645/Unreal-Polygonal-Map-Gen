// Copyright 2018 Schemepunk Studios

#include "IslandMapMesh.h"

AIslandMapMesh::AIslandMapMesh()
{
	ZScale = 10000.0f;

	MapMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("GeneratedMesh"));
	RootComponent = MapMesh;
	MapMesh->bUseAsyncCooking = true;
}

void AIslandMapMesh::OnIslandGenComplete_Implementation()
{
	CreateIslandMesh();
}

void AIslandMapMesh::CreateIslandMesh()
{
	UIslandMapUtils::GenerateMapMesh(Mesh, MapMesh, ZScale, r_elevation);

	if (GroundMaterial)
	{
		MapMesh->SetMaterial(0, GroundMaterial);
	}
}
