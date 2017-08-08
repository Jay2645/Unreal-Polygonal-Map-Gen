// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Maps/Elevations/PolygonalMapHeightmap.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MapTextureRenderer.generated.h"

/**
 * 
 */
UCLASS()
class POLYGONALMAPGENERATOR_API UMapTextureRenderer : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	// Creates a heightmap texture on disk.
	// The data is pulled from the supplied Map Heightmap, and the file is saved as a .png in your GameSavedDir.
	// On Windows, this will be //InstallDir/WindowsNoEditor/GameName/Saved.
	// In the Editor, this will be //ProjectDirectory/Saved.
	// Specify the filename you want to save the heightmap as, without any extension.
	UFUNCTION(BlueprintCallable, Category = "World Generation|Island Generation|Map")
	static bool SaveTextureFromHeightmap(UPolygonalMapHeightmap* MapHeightmap, FString Filename);

	// Creates a heightmap texture on disk.
	// The data is pulled from the supplied array of floats, and the file is saved as a .png in your GameSavedDir.
	// The floats are assumed to be in the range 0-1, where 0 is black and 1 is white. The heightmap will use the RGB channels,
	// with alpha set to 255.
	// On Windows, the image will be saved under //InstallDir/WindowsNoEditor/GameName/Saved.
	// In the Editor, the image will be saved under //ProjectDirectory/Saved.
	// Specify the filename you want to save the heightmap as, without any extension.
	UFUNCTION(BlueprintCallable, Category = "World Generation|Island Generation|Map")
	static bool SaveTextureFromFloatArray(TArray<float> HeightArray, int32 HeightmapSideLength, FString Filename);
};
