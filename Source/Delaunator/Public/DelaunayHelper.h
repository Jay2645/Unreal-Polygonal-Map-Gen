// Unreal Engine 4 Delaunay implementation.
// Source based on https://github.com/delfrrr/delaunator-cpp
// Used under the MIT License.

#pragma once

#include <limits>

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GenericPlatform.h"
#include "DelaunayHelper.generated.h"


//#define KCPPPWPLog( Level, Text ) UE_LOG( LogKeshUE4FundamentalTypeWrapperPlugin, Level, TEXT( Text ) )
//#define KCPPPWPLogF( Level, Format, ... ) UE_LOG( LogKeshUE4FundamentalTypeWrapperPlugin, Level, TEXT( Format ), __VA_ARGS__ )

// Invalid index is set to max value of size_t
// We purposely underflow it to get the max value
constexpr SIZE_T INVALID_DELAUNAY_INDEX = (SIZE_T)-1;

#define PACKED
#pragma pack(push,1)
USTRUCT(BlueprintType)
struct DELAUNATOR_API FSideIndex
{
	GENERATED_BODY()

public:
	SIZE_T Value;
	operator SIZE_T() const { return Value; }

	FSideIndex() { this->Value = INVALID_DELAUNAY_INDEX; }
	FSideIndex(const SIZE_T& Value) { this->Value = Value; }

	FSideIndex& operator=(const FSideIndex& Other)
	{
		Value = Other.Value;
		return *this;
	}

	FSideIndex& operator++()       // Prefix increment operator.
	{
		++Value;
		return *this;
	}
	FSideIndex operator++(int)     // Postfix increment operator.
	{
		FSideIndex temp = *this;
		++*this;
		return temp;
	}

	FSideIndex& operator--()       // Prefix decrement operator.
	{
		--Value;
		return *this;
	}
	FSideIndex operator--(int)     // Postfix decrement operator.
	{
		FSideIndex temp = *this;
		--*this;
		return temp;
	}

	FSideIndex& operator+=(const int& RHS)
	{
		this->Value += RHS;
		return *this;
	}
	FSideIndex& operator+=(const FSideIndex& RHS)
	{
		this->Value += RHS.Value;
		return *this;
	}
	FSideIndex& operator-=(const int& RHS)
	{
		this->Value -= RHS;
		return *this;
	}
	FSideIndex& operator-=(const FSideIndex& RHS)
	{
		this->Value -= RHS.Value;
		return *this;
	}

	bool IsValid() const { return Value != INVALID_DELAUNAY_INDEX; }

};
#pragma pack(pop)

#pragma pack(push,1)
USTRUCT(BlueprintType)
struct DELAUNATOR_API FTriangleIndex
{
	GENERATED_BODY()

public:
	SIZE_T Value;
	operator SIZE_T() const { return Value; }

	FTriangleIndex() { this->Value = INVALID_DELAUNAY_INDEX; }
	FTriangleIndex(const SIZE_T& Value) { this->Value = Value; }

	FTriangleIndex& operator=(const FTriangleIndex& Other)
	{
		Value = Other.Value;
		return *this;
	}

	FTriangleIndex& operator++()       // Prefix increment operator.
	{
		++Value;
		return *this;
	}
	FTriangleIndex operator++(int)     // Postfix increment operator.
	{
		FTriangleIndex temp = *this;
		++*this;
		return temp;
	}

	FTriangleIndex& operator--()       // Prefix decrement operator.
	{
		--Value;
		return *this;
	}
	FTriangleIndex operator--(int)     // Postfix decrement operator.
	{
		FTriangleIndex temp = *this;
		--*this;
		return temp;
	}

	FTriangleIndex& operator+=(const int& RHS)
	{
		this->Value += RHS;
		return *this;
	}
	FTriangleIndex& operator+=(const FTriangleIndex& RHS)
	{
		this->Value += RHS.Value;
		return *this;
	}
	FTriangleIndex& operator-=(const int& RHS)
	{
		this->Value -= RHS;
		return *this;
	}
	FTriangleIndex& operator-=(const FTriangleIndex& RHS)
	{
		this->Value -= RHS.Value;
		return *this;
	}

	bool IsValid() const { return Value != INVALID_DELAUNAY_INDEX; }

};
#pragma pack(pop)

#pragma pack(push,1)
USTRUCT(BlueprintType)
struct DELAUNATOR_API FPointIndex
{
	GENERATED_BODY()

private:

public:
	SIZE_T Value;
	operator SIZE_T() const { return Value; }

	FPointIndex() { this->Value = INVALID_DELAUNAY_INDEX; }
	FPointIndex(const SIZE_T& Value) { this->Value = Value; }

	FPointIndex& operator=(const FPointIndex& Other)
	{
		Value = Other.Value;
		return *this;
	}

	FPointIndex& operator++()       // Prefix increment operator.
	{
		++Value;
		return *this;
	}
	FPointIndex operator++(int)     // Postfix increment operator.
	{
		FPointIndex temp = *this;
		++*this;
		return temp;
	}

	FPointIndex& operator--()       // Prefix decrement operator.
	{
		--Value;
		return *this;
	}
	FPointIndex operator--(int)     // Postfix decrement operator.
	{
		FPointIndex temp = *this;
		--*this;
		return temp;
	}

	FPointIndex& operator+=(const int& RHS)
	{
		this->Value += RHS;
		return *this;
	}
	FPointIndex& operator+=(const FPointIndex& RHS)
	{
		this->Value += RHS.Value;
		return *this;
	}
	FPointIndex& operator-=(const int& RHS)
	{
		this->Value -= RHS;
		return *this;
	}
	FPointIndex& operator-=(const FPointIndex& RHS)
	{
		this->Value -= RHS.Value;
		return *this;
	}

	bool IsValid() const { return Value != INVALID_DELAUNAY_INDEX; }

};
#pragma pack(pop)
#undef PACKED

FORCEINLINE uint32 GetTypeHash(const FSideIndex& Other)
{
	return (uint32)Other.Value;
}
FORCEINLINE uint32 GetTypeHash(const FTriangleIndex& Other)
{
	return (uint32)Other.Value;
}
FORCEINLINE uint32 GetTypeHash(const FPointIndex& Other)
{
	return (uint32)Other.Value;
}

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
	FPointIndex AIndex;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector2D B;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FPointIndex BIndex;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector2D C;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FPointIndex CIndex;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float ABLength;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float BCLength;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float CALength;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FPointIndex Index;

public:
	FDelaunayTriangle()
	{
		A = FVector2D::ZeroVector;
		B = FVector2D::ZeroVector;
		C = FVector2D::ZeroVector;

		AIndex = FPointIndex();
		BIndex = FPointIndex();
		CIndex = FPointIndex();

		Index = FPointIndex();

		ABLength = -1.0f;
		BCLength = -1.0f;
		CALength = -1.0f;
	}

	FDelaunayTriangle(const FVector2D& PointA, const FVector2D& PointB, const FVector2D& PointC, 
					  FPointIndex PointAIndex, FPointIndex PointBIndex, FPointIndex PointCIndex)
	{
		A = PointA;
		B = PointB;
		C = PointC;

		AIndex = PointAIndex;
		BIndex = PointBIndex;
		CIndex = PointCIndex;

		Index = AIndex;

		ABLength = FVector2D::Distance(A, B);
		BCLength = FVector2D::Distance(B, C);
		CALength = FVector2D::Distance(C, A);
	}

public:
	float GetArea() const;
	float GetCircumradius() const;
	FVector2D GetCircumcenter() const;

	bool IsValid() const
	{
		return Index.IsValid();
	}

	FString ToString() const
	{
		FString output = "";
		output += "(" + FString::SanitizeFloat(A.X) + ", " + FString::SanitizeFloat(A.Y) + "), ";
		output += "(" + FString::SanitizeFloat(B.X) + ", " + FString::SanitizeFloat(B.Y) + "), ";
		output += "(" + FString::SanitizeFloat(C.X) + ", " + FString::SanitizeFloat(C.Y) + ")";
		return output;
	}
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
	// If you access this using an edge ID as an index,
	// you will get the opposite edge in the adjacent triangle,
	// or an invalid side if that triangle doesn't exist.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FSideIndex> HalfEdges;

	// An index mapping point IDs to half-edge IDs
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TMap<FPointIndex, FSideIndex> PointToEdge;

	// Starting triangle for the hull.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, AdvancedDisplay)
	FTriangleIndex HullStart;
	// Triangles making up the hull.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, AdvancedDisplay)
	TArray<FTriangleIndex> HullTriangles;
	// Previous triangle in the hull.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, AdvancedDisplay)
	TArray<FTriangleIndex> HullPrevious;
	// Next triangle in the hull.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, AdvancedDisplay)
	TArray<FTriangleIndex> HullNext;

	// The indices of the generated delaunay triangles.
	// If you access this array using an edge ID, you will get the point 
	// where that half-edge starts.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FPointIndex> DelaunayTriangles;

public:
	FDelaunayMesh()
	{
		HullStart = FTriangleIndex();
	}

	FDelaunayMesh(const TArray<FVector2D>& GivenPoints)
	{
		HullStart = FTriangleIndex();
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
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Procedural Generation|Delaunator")
	static bool PointIsValid(const FPointIndex& Point);
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Procedural Generation|Delaunator")
	static bool SideIsValid(const FSideIndex& Side);
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Procedural Generation|Delaunator")
	static bool TriangleIsValid(const FTriangleIndex& Triangle);

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
	static FVector2D GetTrianglePoint(const FDelaunayMesh& Triangulation, FTriangleIndex TriangleIndex);

	// Construct a FDelaunayTriangle struct from the given index.
	// Note that the Triangle Index is the location inside the Triangulation's Triangle array --
	// NOT the actual location itself, just the index.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Procedural Generation|Delaunator|Triangles")
	static FDelaunayTriangle ConvertTriangleIDToTriangle(const FDelaunayMesh& Triangulation, FTriangleIndex TriangleIndex);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Procedural Generation|Delaunator|Triangles")
	static FSideIndex TriangleIndexToEdge(FTriangleIndex TriangleIndex);
	// Given a Triangle ID, returns an array of triangle edges
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Procedural Generation|Delaunator|Triangles")
	static TArray<FSideIndex> EdgesOfTriangle(FTriangleIndex TriangleIndex);
	// Given a Triangle ID, returns an array of point IDs
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Procedural Generation|Delaunator|Triangles")
	static TArray<FPointIndex> PointsOfTriangle(const FDelaunayMesh& Triangulation, FTriangleIndex TriangleIndex);

	// Gets a FVector2D given a half-edge.
	// If the half-edge is invalid, returns (-1, -1).
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Procedural Generation|Delaunator|Half-Edge")
	static FVector2D GetPointFromHalfEdge(const FDelaunayMesh& Triangulation, FSideIndex HalfEdge);
	// Gets an index for a point given a half-edge. This index can then be used in the
	// Triangulation.Coordinates array to look up the associated point.
	// If the half-edge is invalid, returns -1.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Procedural Generation|Delaunator|Half-Edge")
	static FPointIndex GetPointIndexFromHalfEdge(const FDelaunayMesh& Triangulation, FSideIndex HalfEdge);
	// Gets a triangle struct given a half-edge.
	// If the half-edge is invalid, returns a triangle with all values set to invalid indices.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Procedural Generation|Delaunator|Half-Edge")
	static FDelaunayTriangle GetTriangleFromHalfEdge(const FDelaunayMesh& Triangulation, FSideIndex HalfEdge);
	// Gets an index for a triangle given a half-edge. This index can then be used in the
	// Triangulation.Triangles array to look up the associated triangle.
	// If the half-edge is invalid, returns an invalid triangle index.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Procedural Generation|Delaunator|Half-Edge")
	static FTriangleIndex GetTriangleIndexFromHalfEdge(FSideIndex HalfEdge);


	// Given a Half-edge index, gets the next half-edge
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Procedural Generation|Delaunator|Half-Edge")
	static FSideIndex NextHalfEdge(FSideIndex HalfEdge);
	// Given a Half-edge index, gets the previous half-edge
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Procedural Generation|Delaunator|Half-Edge")
	static FSideIndex PreviousHalfEdge(FSideIndex HalfEdge);
	// Given a Half-edge index, gets the opposite half-edge
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Procedural Generation|Delaunator|Half-Edge")
	static FSideIndex OppositeHalfEdge(const FDelaunayMesh& Triangulation, FSideIndex HalfEdge);

	// Given a half-edge leading to a point, gets all other half-edges connected to that point
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Procedural Generation|Delaunator|Half-Edge")
	static TArray<FSideIndex> EdgesFromIncomingEdge(const FDelaunayMesh& Triangulation, FSideIndex PointIndex);
};