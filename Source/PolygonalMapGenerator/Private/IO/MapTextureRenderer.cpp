// Fill out your copyright notice in the Description page of Project Settings.

#include "PolygonalMapGeneratorPrivatePCH.h"
#include "Engine/Texture2D.h"
#include "Runtime/Engine/Public/TextureResource.h"
#include "HighResScreenshot.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Misc/Paths.h"
#include "MapTextureRenderer.h"

bool UMapTextureRenderer::SaveTextureFromHeightmap(UPolygonalMapHeightmap* MapHeightmap, FString Filename)
{
	if (MapHeightmap->GetMapData().Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Did not create heightmap yet!"));
		// Didn't generate heightmap yet
		return false;
	}
	
	uint32 dtWidth = MapHeightmap->HeightmapSize;
	uint32 dtHeight = MapHeightmap->HeightmapSize;
	uint32 pixelCount = dtWidth * dtHeight;

	TArray<float> heights;
	heights.SetNumZeroed(pixelCount);

	for (uint32 x = 0; x < dtWidth; x++)
	{
		for (uint32 y = 0; y < dtHeight; y++)
		{
			FMapData pointData = MapHeightmap->GetMapPoint(x, y);
			heights[x + (y * dtHeight)] = pointData.Elevation;
		}
	}

	return SaveTextureFromFloatArray(heights, MapHeightmap->HeightmapSize, Filename);
}

bool UMapTextureRenderer::SaveTextureFromFloatArray(TArray<float> HeightArray, int32 HeightmapSideLength, FString Filename)
{
	if (HeightArray.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("No floats to make an image out of!"));
		return false;
	}
	else if (HeightmapSideLength < 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Side length must be positive!"));
		return false;
	}

	uint32 dtWidth = (uint32)HeightmapSideLength;
	uint32 dtHeight = (uint32)HeightmapSideLength;;
	uint32 pixelCount = dtWidth * dtHeight;

	TArray<FColor> colors;
	colors.SetNumZeroed(pixelCount);
	for(uint32 i = 0; i < pixelCount; i++)
	{
		colors[i] = FColor(uint8(HeightArray[i] * 255), uint8(HeightArray[i] * 255), uint8(HeightArray[i] * 255), 255);
	}

	FIntPoint destSize(dtWidth, dtHeight);
	FString resultPath;
	FHighResScreenshotConfig& HighResScreenshotConfig = GetHighResScreenshotConfig();

	FString filenameWithExtension = Filename + ".png";
	FString filePath = FPaths::Combine<FString>(FPaths::GameSavedDir(), filenameWithExtension);

	return HighResScreenshotConfig.SaveImage(filePath, colors, destSize, &resultPath);
}