// Work Copyright (c) 2017 Jay M Stevens

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "PolygonMap.h"
#include "MapDataHelper.generated.h"

/**
 * A helper class for FMapData objects.
 * Mostly used to control tags and such.
 */
UCLASS(BlueprintType)
class POLYGONALMAPGENERATOR_API UMapDataHelper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Returns whether a MapData object has a particular tag.
	UFUNCTION(BlueprintPure, Category = "Map Data|Tags")
	static bool HasTag(const FMapData& MapData, const FGameplayTag& Tag);
	// Returns true if the supplied MapData object is tagged with "Border".
	UFUNCTION(BlueprintPure, Category = "Map Data|Tags")
	static bool IsBorder(const FMapData& MapData);
	// Returns true if the supplied MapData object is tagged with "Freshwater".
	UFUNCTION(BlueprintPure, Category = "Map Data|Tags")
	static bool IsFreshwater(const FMapData& MapData);
	// Returns true if the supplied MapData object is tagged with "Water".
	UFUNCTION(BlueprintPure, Category = "Map Data|Tags")
	static bool IsWater(const FMapData& MapData);
	// Returns true if the supplied MapData object is tagged with "Saltwater".
	UFUNCTION(BlueprintPure, Category = "Map Data|Tags")
	static bool IsOcean(const FMapData& MapData);
	// Returns true if the supplied MapData object is tagged with "Coast".
	UFUNCTION(BlueprintPure, Category = "Map Data|Tags")
	static bool IsCoast(const FMapData& MapData);
	// Returns true if the supplied MapData object is tagged with "River".
	UFUNCTION(BlueprintPure, Category = "Map Data|Tags")
	static bool IsRiver(const FMapData& MapData);

	// Gives a MapData object a tag, provided it doesn't have it already.
	UFUNCTION(BlueprintCallable, Category = "Map Data|Tags")
	static FMapData SetMapTag(FMapData& MapData, const FGameplayTag& Tag);
	// Gives a MapData object the tag "Border".
	UFUNCTION(BlueprintCallable, Category = "Map Data|Tags")
	static FMapData SetBorder(FMapData& MapData);
	// Gives a MapData object the tag "Water".
	UFUNCTION(BlueprintCallable, Category = "Map Data|Tags")
	static FMapData SetFreshwater(FMapData& MapData);
	// Gives a MapData object the tag "Ocean".
	UFUNCTION(BlueprintCallable, Category = "Map Data|Tags")
	static FMapData SetOcean(FMapData& MapData);
	// Gives a MapData object the tag "Coast".
	UFUNCTION(BlueprintCallable, Category = "Map Data|Tags")
	static FMapData SetCoast(FMapData& MapData);
	// Gives a MapData object the tag "River".
	UFUNCTION(BlueprintCallable, Category = "Map Data|Tags")
	static FMapData SetRiver(FMapData& MapData);

	// Removes a tag from a MapData object, if it has one.
	UFUNCTION(BlueprintCallable, Category = "Map Data|Tags")
	static FMapData RemoveMapTag(FMapData& MapData, const FGameplayTag& Tag);
	// Removes the tag "Border" from a MapData object.
	UFUNCTION(BlueprintCallable, Category = "Map Data|Tags")
	static FMapData RemoveBorder(FMapData& MapData);
	// Removes the tag "Freshwater" from a MapData object.
	UFUNCTION(BlueprintCallable, Category = "Map Data|Tags")
	static FMapData RemoveFreshwater(FMapData& MapData);
	// Removes the tag "Saltwater" from a MapData object.
	UFUNCTION(BlueprintCallable, Category = "Map Data|Tags")
	static FMapData RemoveOcean(FMapData& MapData);
	// Removes the tag "Coast" from a MapData object.
	UFUNCTION(BlueprintCallable, Category = "Map Data|Tags")
	static FMapData RemoveCoast(FMapData& MapData);
	// Removes the tag "River" from a MapData object.
	UFUNCTION(BlueprintCallable, Category = "Map Data|Tags")
	static FMapData RemoveRiver(FMapData& MapData);
};