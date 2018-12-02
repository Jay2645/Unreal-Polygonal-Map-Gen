// Unreal Engine 4 Delaunay implementation.
// Source based on https://github.com/delfrrr/delaunator-cpp
// Used under the MIT License.

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
	int32 AIndex;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector2D B;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 BIndex;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector2D C;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 CIndex;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 Index;

public:
	FDelaunayTriangle()
	{
		A = FVector2D::ZeroVector;
		B = FVector2D::ZeroVector;
		C = FVector2D::ZeroVector;

		AIndex = -1;
		BIndex = -1;
		CIndex = -1;

		Index = -1;
	}

	FDelaunayTriangle(const FVector2D& PointA, const FVector2D& PointB, const FVector2D& PointC, int32 PointAIndex, int32 PointBIndex, int32 PointCIndex)
	{
		A = PointA;
		B = PointB;
		C = PointC;

		AIndex = PointAIndex;
		BIndex = PointBIndex;
		CIndex = PointCIndex;

		Index = AIndex;
	}

public:
	float GetArea() const;

	FVector2D GetCircumcenter() const;
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
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	//TArray<FDelaunayTriangle> Triangles;

	// Starting coordinate for the hull.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, AdvancedDisplay)
	int32 HullStart;
	// Coordinates making up the hull.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, AdvancedDisplay)
	TArray<int32> HullTriangles;
	// Previous coordinates in the hull.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, AdvancedDisplay)
	TArray<int32> HullPrevious;
	// Next coordinates in the hull.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, AdvancedDisplay)
	TArray<int32> HullNext;

	// The indices of the generated delaunay triangles.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<int32> DelaunayTriangles;

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
	float GetHullArea(float& OutErrorAmount) const;
	// Returns the Kahan and Babuska of an array of floats.
	// Adapted from the Delaunator HPP file.
	float Sum(const TArray<float>& Area, float& OutErrorAmount) const;
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
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Procedural Generation|Delaunator")
	static FDelaunayMesh CreateDelaunayTriangulation(const TArray<FVector2D>& Points);

	// Gets the circumcenter of a triangle, useful for creating Voronoi maps.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Procedural Generation|Delaunator|Triangles")
	static FVector2D GetTriangleCircumcenter(const FDelaunayTriangle& Triangle);
	// Gets the area of a triangle.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Procedural Generation|Delaunator|Triangles")
	static float GetTriangleArea(const FDelaunayTriangle& Triangle);
	// Gets the Vector2D associated with a value from the DelaunayTriangles array.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Procedural Generation|Delaunator|Triangles")
	static FVector2D GetTrianglePoint(const FDelaunayMesh& Triangulation, int32 TriangleIndex);

	// Construct a FDelaunayTriangle struct from the given index.
	// Note that the Triangle Index is the location inside the Triangulation's Triangle array --
	// NOT the actual location itself, just the index.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Procedural Generation|Delaunator|Triangles")
	static FDelaunayTriangle ConvertTriangleIDToTriangle(const FDelaunayMesh& Triangulation, int32 TriangleIndex);

	// Given a Triangle ID, returns an array of triangle edges
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Procedural Generation|Delaunator|Triangles")
	static TArray<int32> EdgesOfTriangle(int32 TriangleIndex);
	// Given a Triangle ID, returns an array of point IDs
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Procedural Generation|Delaunator|Triangles")
	static TArray<int32> PointsOfTriangle(const FDelaunayMesh& Triangulation, int32 TriangleIndex);

	// Gets a FVector2D given a half-edge.
	// If the half-edge is invalid, returns (-1, -1).
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Procedural Generation|Delaunator|Half-Edge")
	static FVector2D GetPointFromHalfEdge(const FDelaunayMesh& Triangulation, int32 HalfEdge);
	// Gets an index for a point given a half-edge. This index can then be used in the
	// Triangulation.Coordinates array to look up the associated point.
	// If the half-edge is invalid, returns -1.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Procedural Generation|Delaunator|Half-Edge")
	static int32 GetPointIndexFromHalfEdge(const FDelaunayMesh& Triangulation, int32 HalfEdge);
	// Gets a triangle struct given a half-edge.
	// If the half-edge is invalid, returns a triangle with all values set to -1.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Procedural Generation|Delaunator|Half-Edge")
	static FDelaunayTriangle GetTriangleFromHalfEdge(const FDelaunayMesh& Triangulation, int32 HalfEdge);
	// Gets an index for a triangle given a half-edge. This index can then be used in the
	// Triangulation.Triangles array to look up the associated triangle.
	// If the half-edge is invalid, returns -1.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Procedural Generation|Delaunator|Half-Edge")
	static int32 GetTriangleIndexFromHalfEdge(const FDelaunayMesh& Triangulation, int32 HalfEdge);


	// Given a Half-edge index, gets the next half-edge
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Procedural Generation|Delaunator|Half-Edge")
	static int32 NextHalfEdge(int32 HalfEdge);
	// Given a Half-edge index, gets the previous half-edge
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Procedural Generation|Delaunator|Half-Edge")
	static int32 PreviousHalfEdge(int32 HalfEdge);

	// Given a half-edge leading to a point, gets all other half-edges connected to that point
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Procedural Generation|Delaunator|Half-Edge")
	static TArray<int32> EdgesAroundPoint(const FDelaunayMesh& Triangulation, int32 PointIndex);
};