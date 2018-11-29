// Copyright 2018 Schemepunk Studios

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DelaunayHelper.generated.h"


/**
* A representation of a triangle.
* The Unreal Engine actually *has* an implementation of a Delaunay Traingle in
* AnimationBlendSpaceHelpers.h, but it can't be used outside of that module or
* in Blueprint.
*/
USTRUCT(BlueprintType)
struct DELAUNATOR_API FDelaunayTriangle
{
	GENERATED_BODY()
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		FVector2D A;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		FVector2D B;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		FVector2D C;

public:
	FDelaunayTriangle()
	{
		A = FVector2D::ZeroVector;
		B = FVector2D::ZeroVector;
		C = FVector2D::ZeroVector;
	}

	FDelaunayTriangle(const FVector2D& PointA, const FVector2D& PointB, const FVector2D& PointC)
	{
		A = PointA;
		B = PointB;
		C = PointC;
	}

public:
	float GetArea() const;
};

/**
* A struct containing all the data about the Delaunay Triangulation.
*/
USTRUCT(BlueprintType)
struct DELAUNATOR_API FDelaunayMesh
{
	GENERATED_BODY()
public:
	// All our generated delaunay coordinates.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FVector2D> Coordinates;
	
	// The generated delaunay half-edges.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<int32> HalfEdges;

	// An easily accessible way to look at the generated triangles.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FDelaunayTriangle> Triangles;

	// The raw float values returned by the Delaunator.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, AdvancedDisplay)
	TArray<float> DelaunayCoords;

	// The indices of the generated delaunay triangles.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, AdvancedDisplay)
	TArray<int32> DelaunayTriangles;
	
	// Starting triangle for the hull.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, AdvancedDisplay)
	int32 HullStart;
	// Triangles making up the hull.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, AdvancedDisplay)
	TArray<int32> HullTriangles;
	// Previous triangles in the hull.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, AdvancedDisplay)
	TArray<int32> HullPrevious;
	// Next triangles in the hull.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, AdvancedDisplay)
	TArray<int32> HullNext;

public:
	FDelaunayMesh()
	{
		HullStart = -1;
	}

	FDelaunayMesh(const TArray<FVector2D>& GivenPoints)
	{
		HullStart = -1;
		CreatePoints(GivenPoints);
	}

public:
	// Generates the actual triangulation
	void CreatePoints(const TArray<FVector2D>& GivenPoints);
	// Gets the area of the Delaunay hull.
	float GetHullArea() const;
	// Returns the Kahan and Babuska of an array of floats.
	// Adapted from the Delaunator HPP file.
	float Sum(const TArray<float>& Area) const;
};

/**
 * A class that allows Blueprints to create a Delaunay triangulation or 
 * access data provided from one.
 */
UCLASS()
class DELAUNATOR_API UDelaunayHelper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Generates a Delaunay triangulation from the given list of points.
	UFUNCTION(BlueprintCallable, Category = "Procedural Generation|Points|Delaunay")
	static FDelaunayMesh CreateDelaunayTriangulation(const TArray<FVector2D>& Points);

	// Gets the Vector2D associated with a value from the DelaunayTriangles array.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Procedural Generation|Points|Delaunay")
	static FVector2D GetTrianglePoint(const FDelaunayMesh& Triangulation, int32 TriangleID);
};
