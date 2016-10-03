// Original Work Copyright (c) 2010 Amit J Patel

#pragma once

#include "PointGenerator.h"
#include "RandomPointGenerator.generated.h"

/**
 * The Random Point Selector will create an array of completely random points.
 */
UCLASS()
class URandomPointGenerator : public UPointGenerator
{
	GENERATED_BODY()

public:
	URandomPointGenerator() { bNeedsMoreRandomness = false; };
	~URandomPointGenerator() {};

	virtual TArray<FVector2D> GeneratePoints(int32 numberOfPoints) override;
};
