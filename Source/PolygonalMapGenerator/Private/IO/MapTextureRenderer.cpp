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
	UE_LOG(LogTemp, Warning, TEXT("Attempting to generate map heightmap!"));

	uint32 dtWidth = MapHeightmap->HeightmapSize;
	uint32 dtHeight = MapHeightmap->HeightmapSize;
	uint32 pixelCount = dtWidth * dtHeight;
	uint32 dtBytesPerPixel = 4;
	uint32 dtBufferSize = dtWidth * dtHeight * dtBytesPerPixel;
	uint32 dtBufferSizeSqrt = dtWidth * dtBytesPerPixel;

	TArray<FColor> colors;
	colors.SetNumZeroed(pixelCount);

	for (uint32 x = 0; x < dtWidth; x++)
	{
		for (uint32 y = 0; y < dtHeight; y++)
		{
			FMapData pointData = MapHeightmap->GetMapPoint(x, y);
			colors[dtWidth * y + x] = FColor(uint8(pointData.Elevation * 255), uint8(pointData.Elevation * 255), uint8(pointData.Elevation * 255), 255);
		}
	}

	FIntPoint destSize(dtWidth, dtHeight);
	FString resultPath;
	FHighResScreenshotConfig& HighResScreenshotConfig = GetHighResScreenshotConfig();
	
	FString filePath = FPaths::Combine<FString>(FPaths::GameSavedDir(), Filename);

	return HighResScreenshotConfig.SaveImage(filePath, colors, destSize, &resultPath);
}

/*void UMapTextureRenderer::CreateTextureFromHeightmap(UPolygonalMapHeightmap* MapHeightmap, UTextureRenderTarget2D* IslandRenderTarget, UMaterialInstanceDynamic* IslandMaterialInstanceDynamic)
{
	if (MapHeightmap->GetMapData().Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Did not create heightmap yet!"));
		// Didn't generate heightmap yet
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("Attempting to generate map heightmap!"));

	uint32 dtWidth = MapHeightmap->HeightmapSize;
	uint32 dtHeight = MapHeightmap->HeightmapSize;

	//IslandRenderTarget->bHDR = 0;
	//IslandRenderTarget->InitAutoFormat(dtWidth, dtHeight);
	//IslandRenderTarget->UpdateResource();
	UE_LOG(LogTemp, Log, TEXT("Set map render target!"));

	UTexture2D* dtTexture = UTexture2D::CreateTransient(dtWidth, dtHeight);
	dtTexture->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
	dtTexture->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
	dtTexture->SRGB = 0;
	dtTexture->AddToRoot();      // Guarantee no garbage collection by adding it as a root reference
	dtTexture->UpdateResource(); // Update the texture with new variable values.

	int32 pixelCount = dtWidth * dtHeight;

	TArray<FColor> colors;
	colors.SetNumZeroed(pixelCount);

	for (uint32 x = 0; x < dtWidth; x++)
	{
		for (uint32 y = 0; y < dtHeight; y++)
		{
			FMapData pointData = MapHeightmap->GetMapPoint(x, y);
			colors[dtWidth * y + x] = FColor(uint8(pointData.Elevation * 255), uint8(pointData.Elevation * 255), uint8(pointData.Elevation * 255), 255);
			//colors[dtWidth * y + x] = FColor::Red;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Created colors!"));

	if (IslandMaterialInstanceDynamic)
	{
		UE_LOG(LogTemp, Warning, TEXT("Time to generate texture!"));
		CreateTexture(dtTexture, colors);
		UE_LOG(LogTemp, Log, TEXT("All done, setting the material instance."));
		IslandMaterialInstanceDynamic->SetTextureParameterValue(FName("DynamicTextureParam"), dtTexture);

		//UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, IslandRenderTarget, dtMaterialInstanceDynamic);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Could not initialize material instance!"));
	}
}

void UMapTextureRenderer::CreateTexture(UTexture2D* TargetTexture, TArray<FColor> Colors)
{
	if (TargetTexture)
	{
		UE_LOG(LogTemp, Warning, TEXT("Target texture is valid."));

		uint32 dtWidth = TargetTexture->GetSizeX();
		uint32 dtHeight = TargetTexture->GetSizeY();
		UE_LOG(LogTemp, Warning, TEXT("Got size!"));

		uint32 dtBytesPerPixel = 4;

		uint32 dtBufferSize = dtWidth * dtHeight * dtBytesPerPixel;

		UE_LOG(LogTemp, Log, TEXT("Color length: %d; Size of buffer: %d, Texture dimensions: %d x %d."), Colors.Num(), dtBufferSize, dtWidth, dtHeight);
		if ((uint32)Colors.Num() > dtBufferSize)
		{
			UE_LOG(LogTemp, Error, TEXT("Buffer size too small!"));
			return;
		}
		else if ((uint32)Colors.Num() * dtBytesPerPixel < dtBufferSize)
		{
			UE_LOG(LogTemp, Warning, TEXT("Not enough colors in buffer!"));
		}

		uint32 dtBufferSizeSqrt = dtWidth * dtBytesPerPixel;
		// This is the data that we Memcpy into the dynamic texture
		uint8* dtBuffer = new uint8[dtBufferSize];
		FUpdateTextureRegion2D* updateTextureRegion = new FUpdateTextureRegion2D(0, 0, 0, 0, dtWidth, dtHeight);

		for (int i = 0; i < Colors.Num(); i++)
		{
			int iBlue = i * 4 + 0;
			int iGreen = i * 4 + 1;
			int iRed = i * 4 + 2;
			int iAlpha = i * 4 + 3;

			dtBuffer[iBlue] = Colors[i].B;
			dtBuffer[iGreen] = Colors[i].G;
			dtBuffer[iRed] = Colors[i].R;
			dtBuffer[iAlpha] = Colors[i].A;
		}
		UE_LOG(LogTemp, Log, TEXT("Done parsing colors, moving on to updating region."));

		UpdateTextureRegions(TargetTexture, 0, 1, updateTextureRegion, dtBufferSizeSqrt, dtBytesPerPixel, dtBuffer, false);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid texture!"));
	}
}

void UMapTextureRenderer::UpdateTextureRegions(UTexture2D* Texture, int32 MipIndex, uint32 NumRegions, FUpdateTextureRegion2D* Regions, uint32 SrcPitch, uint32 SrcBpp, uint8* SrcData, bool bFreeData)
{
	if (Texture && Texture->Resource && Regions)
	{
		struct FUpdateTextureRegionsData
		{
			FTexture2DResource* Texture2DResource;
			int32 MipIndex;
			uint32 NumRegions;
			FUpdateTextureRegion2D* Regions;
			uint32 SrcPitch;
			uint32 SrcBpp;
			uint8* SrcData;
		};
		UE_LOG(LogTemp, Log, TEXT("Creating region data!"));

		FUpdateTextureRegionsData* RegionData = new FUpdateTextureRegionsData;

		RegionData->Texture2DResource = (FTexture2DResource*)Texture->Resource;
		RegionData->MipIndex = MipIndex;
		RegionData->NumRegions = NumRegions;
		RegionData->Regions = Regions;
		RegionData->SrcPitch = SrcPitch;
		RegionData->SrcBpp = SrcBpp;
		RegionData->SrcData = SrcData;

		UE_LOG(LogTemp, Log, TEXT("Handing off to the GPU!"));

		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
			UpdateTextureRegionsData,
			FUpdateTextureRegionsData*, RegionData, RegionData,
			bool, bFreeData, bFreeData,
			{
				for (uint32 RegionIndex = 0; RegionIndex < RegionData->NumRegions; ++RegionIndex)
				{
					int32 CurrentFirstMip = RegionData->Texture2DResource->GetCurrentFirstMip();
					if (RegionData->MipIndex >= CurrentFirstMip)
					{
						RHIUpdateTexture2D(
							RegionData->Texture2DResource->GetTexture2DRHI(),
							RegionData->MipIndex - CurrentFirstMip,
							RegionData->Regions[RegionIndex],
							RegionData->SrcPitch,
							RegionData->SrcData
							+ RegionData->Regions[RegionIndex].SrcY * RegionData->SrcPitch
							+ RegionData->Regions[RegionIndex].SrcX * RegionData->SrcBpp
						);
					}
				}
		if (bFreeData)
		{
			FMemory::Free(RegionData->Regions);
			FMemory::Free(RegionData->SrcData);
		}
		delete RegionData;
			});
		UE_LOG(LogTemp, Warning, TEXT("GPU is done creating texture!"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Attempted to access an invalid texture!"));
	}
}

bool UMapTextureRenderer::SaveTextureToDisk(UTexture2D* Texture, FString Filename)
{
	Texture->UpdateResource();
	FTexture2DMipMap* MM = &Texture->PlatformData->Mips[0];

	TArray<FColor> OutBMP;
	int w = MM->SizeX;
	int h = MM->SizeY;

	OutBMP.InsertZeroed(0, w*h);

	FByteBulkData* RawImageData = &MM->BulkData;

	FColor* FormatedImageData = static_cast<FColor*>(RawImageData->Lock(LOCK_READ_ONLY));

	for (int i = 0; i < (w*h); ++i)
	{
		OutBMP[i] = FormatedImageData[i];
		OutBMP[i].A = 255;
	}

	RawImageData->Unlock();
	FIntPoint DestSize(w, h);

	FString ResultPath;
	FHighResScreenshotConfig& HighResScreenshotConfig = GetHighResScreenshotConfig();
	return HighResScreenshotConfig.SaveImage(Filename, OutBMP, DestSize, &ResultPath);
}*/