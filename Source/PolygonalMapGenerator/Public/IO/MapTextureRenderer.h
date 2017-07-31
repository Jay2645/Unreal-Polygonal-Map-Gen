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

	UFUNCTION(BlueprintCallable, Category = "World Generation|Island Generation|Map")
	static bool SaveTextureFromHeightmap(UPolygonalMapHeightmap* MapHeightmap, FString Filename);
	
	/*UFUNCTION(BlueprintCallable, Category = "World Generation|Island Generation|Map")
	static void CreateTextureFromHeightmap(UPolygonalMapHeightmap* MapHeightmap, UTextureRenderTarget2D* IslandRenderTarget, UMaterialInstanceDynamic* IslandMaterialInstanceDynamic);

	UFUNCTION(BlueprintCallable, Category = "World Generation|Island Generation|Map")
	static void CreateTexture(UTexture2D* TargetTexture, TArray<FColor> Colors);

	UFUNCTION(BlueprintCallable, Category = "World Generation|Island Generation|Map")
	static bool SaveTextureToDisk(UTexture2D* Texture, FString Filename);

	static void UpdateTextureRegions(UTexture2D* Texture, int32 MipIndex, uint32 NumRegions, FUpdateTextureRegion2D* Regions, uint32 SrcPitch, uint32 SrcBpp, uint8* SrcData, bool bFreeData);*/
};
