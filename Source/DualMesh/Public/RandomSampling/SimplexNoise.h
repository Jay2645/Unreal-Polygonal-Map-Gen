/**
* @file    SimplexNoise.h
* @brief   A Perlin Simplex Noise C++ Implementation (1D, 2D, 3D).
*
* Copyright (c) 2014-2018 Sebastien Rombauts (sebastien.rombauts@gmail.com)
*
* Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
* or copy at http://opensource.org/licenses/MIT)
*/
#pragma once

#include <cstddef>  // size_t
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SimplexNoise.generated.h"

/**
* @brief A Perlin Simplex Noise C++ Implementation (1D, 2D, 3D, 4D).
*/
UCLASS()
class DUALMESH_API USimplexNoise : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	// 1D Perlin simplex noise
	static float noise(float x);
	// 2D Perlin simplex noise
	static float noise(float x, float y);
	// 3D Perlin simplex noise
	static float noise(float x, float y, float z);

	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (DisplayName = "Get Simplex Noise"), Category = "Procedural Generation|Random Sampling|Distribution")
	static float GetNoise(float x);
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (DisplayName = "Get 2D Simplex Noise"), Category = "Procedural Generation|Random Sampling|Distribution")
	static float Get2DNoise(const FVector2D& Position);
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (DisplayName = "Get 3D Simplex Noise"), Category = "Procedural Generation|Random Sampling|Distribution")
	static float Get3DNoise(const FVector& Position);
};

/**
* @brief A Perlin Simplex Noise C++ Implementation (1D, 2D, 3D, 4D).
*/
USTRUCT(BlueprintType)
struct DUALMESH_API FCustomSimplexNoise
{
	GENERATED_BODY()
public:
	// Fractal/Fractional Brownian Motion (fBm) noise summation
	float fractal(size_t octaves, float x, float frequency = 1.0f, float amplitude = 1.0f, float lacunarity = 2.0f, float persistence = 0.5f) const;
	float fractal(size_t octaves, FVector2D Position, float frequency = 1.0f, float amplitude = 1.0f, float lacunarity = 2.0f, float persistence = 0.5f) const;
	float fractal(size_t octaves, FVector Position, float frequency = 1.0f, float amplitude = 1.0f, float lacunarity = 2.0f, float persistence = 0.5f) const;

	/**
	* Constructor of to initialize a fractal noise summation
	*
	* @param[in] frequency    Frequency ("width") of the first octave of noise (default to 1.0)
	* @param[in] amplitude    Amplitude ("height") of the first octave of noise (default to 1.0)
	* @param[in] lacunarity   Lacunarity specifies the frequency multiplier between successive octaves (default to 2.0).
	* @param[in] persistence  Persistence is the loss of amplitude between successive octaves (usually 1/lacunarity)
	*/
	FCustomSimplexNoise() { }
};
