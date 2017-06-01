// Original Work Copyright (c) 2010 Amit J Patel, Modified Work Copyright (c) 2016 Jay M Stevens

#pragma once

#include "Components/ActorComponent.h"
#include "PointGenerator.generated.h"
// This fixes issues with UPROPERTY in Visual Studio's Intellisense with VAssistX
using namespace UP;

/**
* The PointSelector class generates the points that the rest of the map is based around.
* More uniform points (such as a Hexagonal Selector or a Square Selector) will create more uniform maps.
*/
UCLASS(Blueprintable)
class POLYGONALMAPGENERATOR_API UPointGenerator : public UActorComponent
{
	GENERATED_BODY()

public:
	UPointGenerator() {};
	~UPointGenerator() {};

	UFUNCTION(BlueprintCallable, Category = "Island Generation|Graph|Points")
		void InitializeSelector(int32 mapSize, int32 seed, int32 border = 10);

	// Generates the given number of points with the given seed.
	// By default, all points will be at 0,0.
	UFUNCTION(BlueprintNativeEvent, Category = "Island Generation|Graph|Points")
		TArray<FVector2D> GeneratePoints(int32 numberOfPoints);
	// As the Hexagon and Square points don't have any inherent randomness, the randomness must come from elsewhere.
	// This function is shorthand for detecting if this generator is a Hexagon or Square generator.
	// If this is true, more randomness will be added in the actual map generation.
	UFUNCTION(BlueprintPure, Category = "Island Generation|Graph|Points")
		bool NeedsMoreRandomness();

	// The lowest possible value our points can reach
	UFUNCTION(BlueprintPure, Category = "Island Generation|Graph|Points")
	virtual int32 MinPoint();
	// The highest possible value our points can reach
	UFUNCTION(BlueprintPure, Category = "Island Generation|Graph|Points")
	virtual int32 MaxPoint();

	// Will return true if the given point is on or beyond our border.
	UFUNCTION(BlueprintPure, Category = "Island Generation|Graph|Points")
		bool PointIsOnBorder(FVector2D point);
protected:
	UPROPERTY(Category = "Island", BlueprintReadWrite, EditAnywhere)
		int32 MapSize;
	UPROPERTY(Category = "Island", BlueprintReadWrite, EditAnywhere)
		FRandomStream RandomGenerator;
	UPROPERTY(Category = "Points", BlueprintReadWrite, EditAnywhere)
		int32 Border;
	UPROPERTY(Category = "Points", BlueprintReadWrite, EditAnywhere)
		bool bNeedsMoreRandomness;

	virtual TArray<FVector2D> GeneratePoints_Implementation(int32 numberOfPoints);
};