// Original Work Copyright (c) 2010 Amit J Patel, Modified Work Copyright (c) 2016 Jay M Stevens

#pragma once

#include "PointGenerator.h"
#include "SquarePointGenerator.generated.h"
/**
* The SquarePointGenerator generates points in a grid pattern.
*/
UCLASS(Blueprintable)
class POLYGONALMAPGENERATOR_API USquarePointGenerator : public UPointGenerator
{
	GENERATED_BODY()

public:
	USquarePointGenerator() { bNeedsMoreRandomness = true; };
	~USquarePointGenerator() {};

protected:
	virtual TArray<FVector2D> GeneratePoints_Implementation(int32 numberOfPoints) override;
};
