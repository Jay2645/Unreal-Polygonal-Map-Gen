// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/NoExportTypes.h"
#include "PolygonMap.h"
#include "Engine/DataTable.h"
#include "RandomStream.h"
#include "PolygonalMapHeightmap.h"
#include "River.generated.h"

class URiver;

/*
* Where the river feeds into.
* Provides access to the URiver pointer that this river feeds into as well
* as the index in the URiver corners array showing where the river feeds.
*/
USTRUCT(BlueprintType)
struct FRiverFeedLocation
{
	GENERATED_BODY()
public:
	// The river this river feeds into, or NULL if it doesn't feed into anything.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
	URiver* Key;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
	int32 Value;
};

/**
 * 
 */
UCLASS(BlueprintType)
class POLYGONALMAPGENERATOR_API URiver : public UObject
{
	GENERATED_BODY()
public:
	~URiver();

	/*UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
	FString RiverName;*/
	// A list of all corners making up this River object.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
	TArray<FMapCorner> RiverCorners;
	// The name of this River
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "River")
	FString RiverName;
	// The MapGraph that serves as the source for all data regarding this river.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graph")
	UPolygonMap* MapGraph;
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
	//TMap<int, FFeederRiver> FeederRivers;
	// A reference to the river that this river feeds into, if it exists.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
	FRiverFeedLocation FeedsInto;

	// Whether the river is a tributary.
	UFUNCTION(BlueprintPure, Category = "River")
	bool IsTributary() const;
	// Does this river terminate properly?
	UFUNCTION(BlueprintPure, Category = "River")
	bool Terminates() const;
	// Returns a copy of the FMapCorner at the given index.
	// Be sure to call UpdateCorner() on the UPolygonMap if the corner gets modified at all.
	UFUNCTION(BlueprintPure, Category = "River|Graph")
	FMapCorner GetCorner(const int32 Index) const;
	// Returns the 2D location of the FMapCorner at the given index.
	UFUNCTION(BlueprintPure, Category = "River|Graph")
	FVector2D GetPointAtIndex(const int32 Index) const;
	// Returns the point representing the "center" of the river.
	UFUNCTION(BlueprintPure, Category = "River|Graph")
	FVector2D GetCenter() const;
	// Gets the edge downstream from the given index.
	// Be sure to call UpdateEdge() on the UPolygonMap if the edge gets modified.
	UFUNCTION(BlueprintPure, Category = "River|Graph")
	FMapEdge GetDownstreamEdge(const int32 Index) const;
	
	// Sets the River's name and adds it to the global river lookup
	UFUNCTION(BlueprintCallable, Category = "River")
	void InitializeRiver(UDataTable* NameDataTable, FRandomStream& RandomGenerator);

	// Adds a corner to the river.
	// All points downstream get increased by the IncreaseRiverAmount.
	UFUNCTION(BlueprintCallable, Category = "River")
	FMapCorner AddCorner(FMapCorner Corner, const int32 IncreaseRiverAmount = 1);
	// Joins the feeder river to this river.
	// Also sets the FeedsInto data for the feeder river.
	UFUNCTION(BlueprintCallable, Category = "River")
	bool JoinRiver(URiver* FeederRiver, FMapCorner JoinLocation, bool bIncreaseRiverVolume = true);

	// Makes this river into a tributary.
	UFUNCTION(BlueprintCallable, Category = "River")
	void MakeTributary();
	// Clears the river entirely.
	UFUNCTION(BlueprintCallable, Category = "River")
	void Clear();
	// Clears the cache used to look up rivers.
	UFUNCTION(BlueprintCallable, Category = "River")
	static void ClearRiverLookupCache();

	// "Draw" this river on the given heightmap.
	UFUNCTION(BlueprintCallable, Category = "River")
	void MoveRiverToHeightmap(UPolygonalMapHeightmap* MapHeightmap);

	FORCEINLINE	bool operator==(const URiver& Other) const
	{
		return Other.RiverName.ToLower() == RiverName.ToLower();		
	}

private:
	//int32 Size;
	int32 FeederRiverCount;
	bool bIsTributary;
	
	static TMap<FString, URiver*> RiverLookup;

	UFUNCTION()
	void DrawBeizerCurve(UPolygonalMapHeightmap* MapHeightmap, FVector2D v0, FVector2D control0, FVector2D v1, FVector2D control1);
	UFUNCTION()
	FVector2D CalculateBezierPoint(float t, FVector2D p0, FVector2D p1, FVector2D p2, FVector2D p3);
	UFUNCTION()
	void DrawLineOnHeightmap(UPolygonalMapHeightmap* MapHeightmap, const FVector2D& point1, const FVector2D& point2);
};
