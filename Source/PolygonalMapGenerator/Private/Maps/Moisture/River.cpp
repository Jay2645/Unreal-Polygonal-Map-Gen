// Fill out your copyright notice in the Description page of Project Settings.

#include "PolygonalMapGeneratorPrivatePCH.h"
#include "MapDataHelper.h"
#include "Naming/ProceduralNameGenerator.h"
#include "River.h"

TMap<FString, URiver*> URiver::RiverLookup = TMap<FString, URiver*>();

URiver::~URiver()
{
	//Clear();
}

bool URiver::IsTributary() const
{
	return bIsTributary;
}

bool URiver::Terminates() const
{
	return RiverCorners.Num() > 0 && (UMapDataHelper::IsOcean(RiverCorners[RiverCorners.Num() - 1].CornerData) || (FeedsInto.Key != NULL && FeedsInto.Key->Terminates()));
}

FMapCorner URiver::GetCorner(const int32 Index) const
{
	return RiverCorners[Index];
}

FVector2D URiver::GetPointAtIndex(const int32 Index) const
{
	return GetCorner(Index).CornerData.Point;
}

FVector2D URiver::GetCenter() const
{
	if (RiverCorners.Num() % 2 == 0)
	{
		// Even number of points
		int32 halfLength = RiverCorners.Num() / 2;
		FMapEdge edge = GetDownstreamEdge(halfLength - 1); // Subtract one because array starts at 0
		return edge.Midpoint;
	}
	else
	{
		// Odd number of points
		int32 oddCount = FMath::FloorToInt(RiverCorners.Num() / 2.0f);
		return GetPointAtIndex(oddCount);
	}
}

FMapEdge URiver::GetDownstreamEdge(const int32 Index) const
{
	if (Index + 1 >= RiverCorners.Num())
	{
		if (FeedsInto.Key != NULL)
		{
			return FeedsInto.Key->GetDownstreamEdge(FeedsInto.Value + 1);
		}
		else
		{
			return FMapEdge();
		}
	}
	else
	{
		return MapGraph->FindEdgeFromCorners(RiverCorners[Index], RiverCorners[Index + 1]);
	}
}

void URiver::InitializeRiver(UDataTable* NameDataTable, FRandomStream& RandomGenerator)
{
	TArray<FText> textArray;
	FString contextString;
	TArray<FName> rowNames;
	rowNames = NameDataTable->GetRowNames();

	for ( auto& name : rowNames )
	{
		FMarkovData* row = NameDataTable->FindRow<FMarkovData>(name, contextString);
		if (row)
		{
			FText rowText = row->MarkovText;
			textArray.Add(rowText);
		}
	}
	
	const uint8 randomOrder = 2;
	const uint8 minLength = 3;
	const uint8 maxLength = 16;
	const bool bConvertToTitleCase = true;
	const bool bUseNamePattern = false;
	
	RiverName = UProceduralNameGenerator::GenerateRandomWord(textArray, RandomGenerator, randomOrder, minLength, maxLength, bConvertToTitleCase, bUseNamePattern);
	FString lastAttempt = RiverName;
	while(RiverLookup.Contains(RiverName))
	{
		int32 nextSeed = RandomGenerator.RandRange(0, 2147483647);
		RandomGenerator.Initialize(nextSeed);
		RiverName = UProceduralNameGenerator::GenerateRandomWord(textArray, RandomGenerator, randomOrder, minLength, maxLength, bConvertToTitleCase, bUseNamePattern);
		if (RiverName.Equals(lastAttempt))
		{
			UE_LOG(LogWorldGen, Error, TEXT("Generated the same name (%s) twice in a row! Random seed: %d"), *lastAttempt, nextSeed);
			break;
		}
		lastAttempt = RiverName;
	}
	RiverLookup.Add(RiverName, this);
}

FMapCorner URiver::AddCorner(FMapCorner Corner, const int32 IncreaseRiverAmount /*= 1*/)
{
	if (RiverCorners.Contains(Corner))
	{
		return Corner;
	}
	Corner.River = this;
	Corner.RiverSize += IncreaseRiverAmount;
	Corner.RiverIndex = RiverCorners.Num();
	//c.AddToRiver(this, Length, increaseRiverVolumeAmount);
	//if (Length > 1)
	//{
	//Edge edge = graph.lookupEdgeFromCorner(corners[Length - 1], c);
	//edge.river = edge.river + 1;
	//}
	RiverCorners.Add(Corner);

	if (!IsTributary())
	{
		/*if (nameSuffixID <= 0 && corners.Count > MIN_RIVER_NAME_SIZE / 2)
		{
			nameSuffixID = 1;
		}
		else if (nameSuffixID == 1 && corners.Count > MIN_RIVER_NAME_SIZE)
		{
			nameSuffixID = 2;
		}*/
	}
	MapGraph->UpdateCorner(Corner);
	return Corner;
}

bool URiver::JoinRiver(URiver* FeederRiver, FMapCorner JoinLocation, bool bIncreaseRiverVolume /*= true*/)
{
	if (FeederRiver == NULL || FeederRiver == this || FeederRiver == FeedsInto.Key)
	{
		return false;
	}

	// The join index is the location along the river that 
	int joinIndex = JoinLocation.RiverIndex;
	if (bIncreaseRiverVolume)
	{
		for (int i = joinIndex; i < RiverCorners.Num(); i++)
		{
			RiverCorners[i].RiverSize++;
			MapGraph->UpdateCorner(RiverCorners[i]);
		}
	}

	FeederRiver->FeedsInto.Key = this;
	FeederRiver->FeedsInto.Value = joinIndex;

	/*FFeederRiver feederRiverList = FeederRivers.FindOrAdd(joinIndex);
	feederRiverList.FeederRivers.Add(FeederRiver);
	FeederRivers[joinIndex] = feederRiverList;*/
	FeederRiverCount++;
	/*if (nameSuffixID == 0 && feederRiverCount >= MIN_RIVER_NAME_TRIBUTARY_COUNT / 2)
	{
		nameSuffixID = 1;
	}
	else if (nameSuffixID == 1 && feederRiverCount >= MIN_RIVER_NAME_TRIBUTARY_COUNT)
	{
		nameSuffixID = 2;
	}*/
	return true;
}

void URiver::MakeTributary()
{
	bIsTributary = true;
	//nameSuffixID = Random.Range(0, 2);
}

void URiver::Clear()
{
	for (int i = 0; i < RiverCorners.Num(); i++)
	{
		FMapCorner corner = MapGraph->GetCorner(RiverCorners[i].Index);
		corner.RiverIndex = -1;
		corner.River = NULL;
		corner.RiverSize -= 1;
		MapGraph->UpdateCorner(corner);
	}
	/*foreach(Corner c in corners)
	{
		c.AddToRiver(null, -1);
	}
	riverNameLookup.Remove(Name);*/
}

void URiver::ClearRiverLookupCache()
{
	//riverNameLookup.Clear();
}

void URiver::MoveRiverToHeightmap(UPolygonalMapHeightmap* MapHeightmap)
{
	if (RiverCorners.Num() == 0)
	{
		return;
	}
	FMapEdge lastEdge;
	FMapEdge currentEdge;
	FMapEdge nextEdge;
	for (int i = 0; i < RiverCorners.Num() - 2; i++)
	{
		FMapCorner v0 = RiverCorners[i];
		FMapCorner v1 = RiverCorners[i + 1];
		FMapCorner v2 = RiverCorners[i + 2];
		lastEdge = currentEdge;
		currentEdge = MapGraph->FindEdgeFromCorners(v0, v1);
		nextEdge = MapGraph->FindEdgeFromCorners(v1, v2);
		if (lastEdge.Index != -1 && nextEdge.Index != -1)
		{
			DrawBeizerCurve(MapHeightmap, lastEdge.Midpoint, v0.CornerData.Point, nextEdge.Midpoint, v1.CornerData.Point);
		}
	}
}

void URiver::DrawBeizerCurve(UPolygonalMapHeightmap* MapHeightmap, FVector2D v0, FVector2D control0, FVector2D v1, FVector2D control1)
{
	FVector2D q0 = CalculateBezierPoint(0, v0, control0, control1, v1);

	for (int i = 1; i <= 25; i++)
	{
		float t = i / 25.0f;
		FVector2D q1 = CalculateBezierPoint(t, v0, control0, control1, v1);

		DrawLineOnHeightmap(MapHeightmap, q0, q1);
		q0 = q1;
	}
}

FVector2D URiver::CalculateBezierPoint(float t, FVector2D p0, FVector2D p1, FVector2D p2, FVector2D p3)
{
	float u = 1 - t;
	float tt = t * t;
	float uu = u * u;
	float uuu = uu * u;
	float ttt = tt * t;

	FVector2D p = uuu * p0; //first term

	p += 3 * uu * t * p1; //second term
	p += 3 * u * tt * p2; //third term
	p += ttt * p3; //fourth term

	return p;
}

void URiver::DrawLineOnHeightmap(UPolygonalMapHeightmap* MapHeightmap, const FVector2D& point1, const FVector2D& point2)
{
	float deltaX = point2.X - point1.X;
	if (deltaX == 0)
	{
		// Line is vertical
		int32 x = FMath::RoundToInt(point1.X);
		for (int32 y = FMath::RoundToInt(point1.Y); y <= FMath::RoundToInt(point2.Y); y++)
		{
			for(int xOffset = -1; xOffset <= 1; xOffset++)
			{
				for (int yOffset = -1; yOffset <= 1; yOffset++)
				{
					FMapData point = MapHeightmap->GetMapPoint(x + xOffset, y + yOffset);
					if (point.Elevation == 0.0f && point.Point == FVector2D::ZeroVector && point.Moisture == 0.0f)
					{
						// Empty point
						continue;
					}
					point = UMapDataHelper::SetRiver(point);
					MapHeightmap->SetMapPoint(x + xOffset, y + yOffset, point);
				}
			}
		}
		return;
	}
	float deltaY = point2.Y - point1.Y;
	float deltaError = FMath::Abs(deltaY / deltaX);
	float error = deltaError - 0.5f;
	int32 y = FMath::RoundToInt(point1.Y);
	for (int x = FMath::RoundToInt(point1.X); x < FMath::RoundToInt(point2.X); x++)
	{
		for (int xOffset = -1; xOffset <= 1; xOffset++)
		{
			for (int yOffset = -1; yOffset <= 1; yOffset++)
			{
				FMapData point = MapHeightmap->GetMapPoint(x + xOffset, y + yOffset);
				if (point.Elevation == 0.0f && point.Point == FVector2D::ZeroVector && point.Moisture == 0.0f)
				{
					// Empty point
					continue;
				}
				point = UMapDataHelper::SetRiver(point);
				MapHeightmap->SetMapPoint(x + xOffset, y + yOffset, point);
			}
		}

		error += deltaError;
		if (error >= 0.5f)
		{
			y++;
			error -= 1.0f;
		}
	}
}