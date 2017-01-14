// Work Copyright (c) 2017 Jay M Stevens

#include "PolygonalMapGeneratorPrivatePCH.h"
#include "MapDataHelper.h"

const FName UMapDataHelper::TAG_Border = TEXT("Border");
const FName UMapDataHelper::TAG_Water = TEXT("Water");
const FName UMapDataHelper::TAG_Ocean = TEXT("Ocean");
const FName UMapDataHelper::TAG_Coast = TEXT("Coast");
const FName UMapDataHelper::TAG_River = TEXT("River");

bool UMapDataHelper::HasTag(const FMapData& MapData, const FName& Tag)
{
	return MapData.Tags.Contains(Tag);
}

bool UMapDataHelper::IsBorder(const FMapData& MapData)
{
	return HasTag(MapData, TAG_Border);
}

bool UMapDataHelper::IsWater(const FMapData& MapData)
{
	return HasTag(MapData, TAG_Water);
}

bool UMapDataHelper::IsOcean(const FMapData& MapData)
{
	return HasTag(MapData, TAG_Ocean);
}

bool UMapDataHelper::IsCoast(const FMapData& MapData)
{
	return HasTag(MapData, TAG_Coast);
}

bool UMapDataHelper::IsRiver(const FMapData& MapData)
{
	return HasTag(MapData, TAG_River);
}

FMapData UMapDataHelper::SetMapTag(FMapData& MapData, const FName& Tag)
{
	MapData.Tags.AddUnique(Tag);
	return MapData;
}

FMapData UMapDataHelper::SetBorder(FMapData& MapData)
{
	return SetMapTag(MapData, TAG_Border);
}

FMapData UMapDataHelper::SetWater(FMapData& MapData)
{
	return SetMapTag(MapData, TAG_Water);
}

FMapData UMapDataHelper::SetOcean(FMapData& MapData)
{
	return SetMapTag(MapData, TAG_Ocean);
}

FMapData UMapDataHelper::SetCoast(FMapData& MapData)
{
	return SetMapTag(MapData, TAG_Coast);
}

FMapData UMapDataHelper::SetRiver(FMapData& MapData)
{
	return SetMapTag(MapData, TAG_River);
}

FMapData UMapDataHelper::RemoveMapTag(FMapData& MapData, const FName& Tag)
{
	MapData.Tags.Remove(Tag);
	return MapData;
}

FMapData UMapDataHelper::RemoveBorder(FMapData& MapData)
{
	return RemoveMapTag(MapData, TAG_Border);
}

FMapData UMapDataHelper::RemoveWater(FMapData& MapData)
{
	return RemoveMapTag(MapData, TAG_Water);
}

FMapData UMapDataHelper::RemoveOcean(FMapData& MapData)
{
	return RemoveMapTag(MapData, TAG_Ocean);
}

FMapData UMapDataHelper::RemoveCoast(FMapData& MapData)
{
	return RemoveMapTag(MapData, TAG_Coast);
}

FMapData UMapDataHelper::RemoveRiver(FMapData& MapData)
{
	return RemoveMapTag(MapData, TAG_River);
}