// Work Copyright (c) 2017 Jay M Stevens

#include "PolygonalMapGeneratorPrivatePCH.h"
#include "MapDataHelper.h"

//const FName UMapDataHelper::TAG_Border = TEXT("Border");
//const FName UMapDataHelper::TAG_Water = TEXT("Water");
//const FName UMapDataHelper::TAG_Ocean = TEXT("Ocean");
//const FName UMapDataHelper::TAG_Coast = TEXT("Coast");
//const FName UMapDataHelper::TAG_River = TEXT("River");

bool UMapDataHelper::HasTag(const FMapData& MapData, const FGameplayTag& Tag)
{
	return MapData.Tags.HasTag(Tag);
	//return MapData.Tags.Contains(Tag);
}

bool UMapDataHelper::IsBorder(const FMapData& MapData)
{
	return HasTag(MapData, FGameplayTag::RequestGameplayTag(TEXT("MapData.MetaData.Border")));
}

bool UMapDataHelper::IsFreshwater(const FMapData& MapData)
{
	return HasTag(MapData, FGameplayTag::RequestGameplayTag(TEXT("MapData.MetaData.Water.Freshwater")));
}

bool UMapDataHelper::IsWater(const FMapData& MapData)
{
	return HasTag(MapData, FGameplayTag::RequestGameplayTag(TEXT("MapData.MetaData.Water")));
}

bool UMapDataHelper::IsOcean(const FMapData& MapData)
{
	return HasTag(MapData, FGameplayTag::RequestGameplayTag(TEXT("MapData.MetaData.Water.Saltwater")));
}

bool UMapDataHelper::IsCoast(const FMapData& MapData)
{
	return HasTag(MapData, FGameplayTag::RequestGameplayTag(TEXT("MapData.MetaData.Coast")));
}

bool UMapDataHelper::IsRiver(const FMapData& MapData)
{
	return HasTag(MapData, FGameplayTag::RequestGameplayTag(TEXT("MapData.MetaData.Water.River")));
}

FMapData UMapDataHelper::SetMapTag(FMapData& MapData, const FGameplayTag& Tag)
{
	MapData.Tags.AddTag(Tag);
	return MapData;
}

FMapData UMapDataHelper::SetBorder(FMapData& MapData)
{
	return SetMapTag(MapData, FGameplayTag::RequestGameplayTag(TEXT("MapData.MetaData.Border")));
}

FMapData UMapDataHelper::SetFreshwater(FMapData& MapData)
{
	return SetMapTag(MapData, FGameplayTag::RequestGameplayTag(TEXT("MapData.MetaData.Water.Freshwater")));
}

FMapData UMapDataHelper::SetOcean(FMapData& MapData)
{
	return SetMapTag(MapData, FGameplayTag::RequestGameplayTag(TEXT("MapData.MetaData.Water.Saltwater")));
}

FMapData UMapDataHelper::SetCoast(FMapData& MapData)
{
	return SetMapTag(MapData, FGameplayTag::RequestGameplayTag(TEXT("MapData.MetaData.Coast")));
}

FMapData UMapDataHelper::SetRiver(FMapData& MapData)
{
	return SetMapTag(MapData, FGameplayTag::RequestGameplayTag(TEXT("MapData.MetaData.Water.River")));
}

FMapData UMapDataHelper::RemoveMapTag(FMapData& MapData, const FGameplayTag& Tag)
{
	MapData.Tags.RemoveTag(Tag);
	return MapData;
}

FMapData UMapDataHelper::RemoveBorder(FMapData& MapData)
{
	return RemoveMapTag(MapData, FGameplayTag::RequestGameplayTag(TEXT("MapData.MetaData.Border")));
}

FMapData UMapDataHelper::RemoveFreshwater(FMapData& MapData)
{
	return RemoveMapTag(MapData, FGameplayTag::RequestGameplayTag(TEXT("MapData.MetaData.Water.Freshwater")));
}

FMapData UMapDataHelper::RemoveOcean(FMapData& MapData)
{
	return RemoveMapTag(MapData, FGameplayTag::RequestGameplayTag(TEXT("MapData.MetaData.Water.Saltwater")));
}

FMapData UMapDataHelper::RemoveCoast(FMapData& MapData)
{
	return RemoveMapTag(MapData, FGameplayTag::RequestGameplayTag(TEXT("MapData.MetaData.Coast")));
}

FMapData UMapDataHelper::RemoveRiver(FMapData& MapData)
{
	return RemoveMapTag(MapData, FGameplayTag::RequestGameplayTag(TEXT("MapData.MetaData.Water.River")));
}