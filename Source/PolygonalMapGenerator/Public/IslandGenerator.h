// Copyright 2018 Schemepunk Studios

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IslandGenerator.generated.h"

UCLASS()
class POLYGONALMAPGENERATOR_API AIslandGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AIslandGenerator();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
