// Original Work Copyright (c) 2010 Amit J Patel, Modified Work Copyright (c) 2016 Jay M Stevens

#pragma once

#include "Object.h"
#include "IslandShape.generated.h"

/**
 * The IslandShape class is a class which takes a 2D point and
 * returns a value indicating whether the point is land or water.
 */
UCLASS(Blueprintable)
class UIslandShape : public UObject
{
	GENERATED_BODY()
public:
	UIslandShape() {};
	~UIslandShape() {};

	virtual void SetSeed(int32 seed, int32 size);

	// Whether the given point is land or not.
	// By default, all points will be considered water.
	virtual bool IsPointLand(FVector2D point);

	// 1.0 means no small islands; 2.0 leads to a lot
	UPROPERTY(Category = "Island Size", BlueprintReadWrite, EditAnywhere)
	float IslandFactor = 1.25f;

protected:
	FRandomStream StreamRNG;
	int32 Size;
};