// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/NoExportTypes.h"
#include "PolygonMap.h"
#include "PolygonalMapHeightmap.h"
#include "River.generated.h"

class URiver;

/*USTRUCT(BlueprintType)
struct FFeederRiver
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
	TArray<URiver*> FeederRivers;
};*/

USTRUCT(BlueprintType)
struct FRiverFeedLocation
{
	GENERATED_BODY()
public:
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
	URiver();
	//~URiver();

	/*UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
	FString RiverName;*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
	TArray<FMapCorner> RiverCorners;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graph")
	UPolygonMap* MapGraph;
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
	//TMap<int, FFeederRiver> FeederRivers;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
	FRiverFeedLocation FeedsInto;

	UFUNCTION(BlueprintPure, Category = "River")
	bool IsTributary() const;
	UFUNCTION(BlueprintPure, Category = "River")
	bool Terminates() const;
	UFUNCTION(BlueprintPure, Category = "River|Graph")
	FMapCorner GetCorner(const int32 Index) const;
	UFUNCTION(BlueprintPure, Category = "River|Graph")
	FVector2D GetPointAtIndex(const int32 Index) const;
	UFUNCTION(BlueprintPure, Category = "River|Graph")
	FVector2D GetCenter() const;
	UFUNCTION(BlueprintPure, Category = "River|Graph")
	FMapEdge GetDownstreamEdge(const int32 Index) const;

	UFUNCTION(BlueprintCallable, Category = "River")
	FMapCorner AddCorner(FMapCorner Corner, const int32 IncreaseRiverAmount = 1);
	UFUNCTION(BlueprintCallable, Category = "River")
	bool JoinRiver(URiver* FeederRiver, FMapCorner JoinLocation, bool bIncreaseRiverVolume = true);

	UFUNCTION(BlueprintCallable, Category = "River")
	void MakeTributary();
	UFUNCTION(BlueprintCallable, Category = "River")
	void Clear();
	UFUNCTION(BlueprintCallable, Category = "River")
	static void ClearRiverLookupCache();

	UFUNCTION(BlueprintCallable, Category = "River")
	void MoveRiverToHeightmap(UPolygonalMapHeightmap* MapHeightmap);

	FORCEINLINE	bool operator==(const URiver& Other) const
	{
		return Other.RiverID == RiverID;		
	}

private:
	int32 RiverID;
	//int32 Size;
	int32 FeederRiverCount;
	bool bIsTributary;

	static int32 CurrentRiverID;
	static TMap<int32, URiver*> RiverLookup;

	UFUNCTION()
	void DrawBeizerCurve(UPolygonalMapHeightmap* MapHeightmap, FVector2D v0, FVector2D control0, FVector2D v1, FVector2D control1);
	UFUNCTION()
	FVector2D CalculateBezierPoint(float t, FVector2D p0, FVector2D p1, FVector2D p2, FVector2D p3);
	UFUNCTION()
	void DrawLineOnHeightmap(UPolygonalMapHeightmap* MapHeightmap, const FVector2D& point1, const FVector2D& point2);
};
