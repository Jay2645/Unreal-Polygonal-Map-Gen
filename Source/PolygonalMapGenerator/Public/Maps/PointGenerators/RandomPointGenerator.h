// Original Work Copyright (c) 2010 Amit J Patel, Modified Work Copyright (c) 2016 Jay M Stevens

#pragma once

#include "PointGenerator.h"
#include "RandomPointGenerator.generated.h"

/**
* The Random Point Selector will create an array of completely random points.
*/
UCLASS(Blueprintable)
class POLYGONALMAPGENERATOR_API URandomPointGenerator : public UPointGenerator
{
	GENERATED_BODY()

public:
	URandomPointGenerator() { bNeedsMoreRandomness = false; };
	~URandomPointGenerator() {};

protected:
	virtual TArray<FVector2D> GeneratePoints_Implementation(int32 numberOfPoints) override;
};
