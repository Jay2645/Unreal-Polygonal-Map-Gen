// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Object.h"
#include "PointGenerator.generated.h"

/**
 * The PointSelector class generates the points that the rest of the map is based around.
 * More uniform points (such as a Hexagonal Selector or a Square Selector) will create more uniform maps.
 */

UCLASS()
class UPointGenerator : public UObject
{
	GENERATED_BODY()

public:
	UPointGenerator() {};
	~UPointGenerator() {};

	void InitializeSelector(int32 mapSize, int32 seed, int32 border = 10);

	// Generates the given number of points with the given seed.
	// By default, all points will be at 0,0.
	virtual TArray<FVector2D> GeneratePoints(int32 numberOfPoints);

	// As the Hexagon and Square points don't have any inherent randomness, the randomness must come from elsewhere.
	// This function is shorthand for detecting if this generator is a Hexagon or Square generator.
	// If this is true, more randomness will be added in the actual map generation.
	bool NeedsMoreRandomness();

	// Will return true if the given point is on or beyond our border.
	bool PointIsOnBorder(FVector2D point);
protected:
	int32 MapSize;
	FRandomStream RandomGenerator;
	int32 Border;
	bool bNeedsMoreRandomness;
};