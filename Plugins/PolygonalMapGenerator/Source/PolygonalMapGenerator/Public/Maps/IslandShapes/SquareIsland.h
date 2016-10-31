// Original Work Copyright (c) 2010 Amit J Patel, Modified Work Copyright (c) 2016 Jay M Stevens

#pragma once

#include "IslandShape.h"
#include "SquareIsland.generated.h"

/**
* The Square Island is an island where all points are land.
*/
UCLASS(Blueprintable)
class POLYGONALMAPGENERATOR_API USquareIsland : public UIslandShape
{
	GENERATED_BODY()

protected:
	// Whether the given point is land or not.
	// By default, all points will be considered land.
	virtual bool IsPointLand_Implementation(FVector2D point) override;
};
