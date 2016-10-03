// Original Work Copyright (c) 2010 Amit J Patel

#pragma once

#include "PointGenerator.h"
#include "SquarePointGenerator.generated.h"

/**
 * 
 */
UCLASS()
class USquarePointGenerator : public UPointGenerator
{
	GENERATED_BODY()

public:
	USquarePointGenerator() { bNeedsMoreRandomness = true; };
	~USquarePointGenerator() {};

	virtual TArray<FVector2D> GeneratePoints(int32 numberOfPoints) override;
};
