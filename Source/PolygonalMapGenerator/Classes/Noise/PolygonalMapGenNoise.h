// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/NoExportTypes.h"
#include "PolygonalMapGenNoise.generated.h"

UENUM(BlueprintType)		//"BlueprintType" is essential to include
enum class ENoiseType : uint8
{
	Value 			UMETA(DisplayName = "Value"),
	ValueFractal 	UMETA(DisplayName = "ValueFractal"),
	Gradient		UMETA(DisplayName = "Gradient"),
	GradientFractal UMETA(DisplayName = "GradientFractal"),
	Simplex 		UMETA(DisplayName = "Simplex"),
	SimplexFractal	UMETA(DisplayName = "SimplexFractal"),
	Cellular 		UMETA(DisplayName = "Cellular"),
	WhiteNoise		UMETA(DisplayName = "WhiteNoise")
};

UENUM(BlueprintType)		//"BlueprintType" is essential to include
enum class EInterp : uint8
{
	InterpLinear 	UMETA(DisplayName = "InterpLinear"),
	InterpHermite 	UMETA(DisplayName = "InterpHermite"),
	InterpQuintic	UMETA(DisplayName = "InterpQuintic")
};

UENUM(BlueprintType)		//"BlueprintType" is essential to include
enum class EFractalType : uint8
{
	FBM 			UMETA(DisplayName = "FBM"),
	Billow 			UMETA(DisplayName = "Billow"),
	RigidMulti		UMETA(DisplayName = "RigidMulti")
};


UENUM(BlueprintType)		//"BlueprintType" is essential to include
enum class ECellularDistanceFunction : uint8
{
	Euclidean 		UMETA(DisplayName = "Euclidean"),
	Manhattan 		UMETA(DisplayName = "Manhattan"),
	Natural			UMETA(DisplayName = "Natural")
};

UENUM(BlueprintType)		//"BlueprintType" is essential to include
enum class ECellularReturnType : uint8
{
	CellValue 		UMETA(DisplayName = "CellValue"),
	NoiseLookup 	UMETA(DisplayName = "NoiseLookup"),
	Distance		UMETA(DisplayName = "Distance"),
	Distance2		UMETA(DisplayName = "Distance2"),
	Distance2Add 	UMETA(DisplayName = "Distance2Add"),
	Distance2Sub	UMETA(DisplayName = "Distance2Sub"),
	Distance2Mul 	UMETA(DisplayName = "Distance2Mul"),
	Distance2Div	UMETA(DisplayName = "Distance2Div")
};

USTRUCT(BlueprintType)
struct FNoiseSettings
{
	GENERATED_BODY()

	// The seed of our fractal
	UPROPERTY(Category = "Noise Settings", BlueprintReadWrite, EditAnywhere)
	int32 Seed;

	// The number of octaves that the noise generator will use. More octaves creates "messier" noise.
	UPROPERTY(Category = "Noise Settings", BlueprintReadWrite, EditAnywhere)
	int32 NoiseOctaves;

	// The frequency of the noise
	UPROPERTY(Category = "Noise Settings", BlueprintReadWrite, EditAnywhere)
	float NoiseFrequency;

	// The detail found in the noise
	UPROPERTY(Category = "Noise Settings", BlueprintReadWrite, EditAnywhere)
	float Lacunarity;

	// The contrast of the noise
	UPROPERTY(Category = "Noise Settings", BlueprintReadWrite, EditAnywhere)
	float Gain;

	// The type of fractal to use to generate the noise
	UPROPERTY(Category = "Noise Settings", BlueprintReadWrite, EditAnywhere)
	EFractalType FractalType;

	// The type of noise to use
	UPROPERTY(Category = "Noise Settings", BlueprintReadWrite, EditAnywhere)
	ENoiseType NoiseType;

	// How to interpolate the noise
	// Linear produces the smoothest result
	// Hermite produces a slightly more rough result
	// Quintic gives the roughest result
	UPROPERTY(Category = "Noise Settings", BlueprintReadWrite, EditAnywhere)
	EInterp NoiseInterpolation;

	UPROPERTY(Category = "Noise Settings", BlueprintReadWrite, EditAnywhere)
	FVector2D NoiseRange;
	//Constructor
	FNoiseSettings()
	{
		Seed = 123;
		NoiseOctaves = 3;
		NoiseFrequency = 0.01f;
		Lacunarity = 2.0f;
		Gain = 0.5f;
		NoiseType = ENoiseType::SimplexFractal;
		NoiseInterpolation = EInterp::InterpLinear;
		FractalType = EFractalType::FBM;
		NoiseRange = FVector2D(0.0f, 1.0f);
	}
};

USTRUCT(BlueprintType)
struct FNoiseProfile
{
	GENERATED_BODY()

	UPROPERTY(Category = "Voxel Biome", BlueprintReadWrite, EditAnywhere)
	FName ProfileName;

	UPROPERTY(Category = "Voxel Biome", BlueprintReadWrite, EditAnywhere)
	int32 MaterialIndex;

	UPROPERTY(Category = "Voxel Terrain", BlueprintReadWrite, EditAnywhere)
	FNoiseSettings NoiseSettings;

	// How "squished" the noise is
	UPROPERTY(Category = "Voxel Terrain", BlueprintReadWrite, EditAnywhere)
	float TerrainHeightScale;

	UPROPERTY(Category = "Voxel Terrain", BlueprintReadWrite, EditAnywhere)
	float NoiseVerticalOffset;

	//Constructor
	FNoiseProfile()
	{
		MaterialIndex = 0;
		TerrainHeightScale = 0.5f;
		NoiseVerticalOffset = 0.0f;
		ProfileName = NAME_None;
	}
};

/**
 * 
 */
class PolygonalMapGenNoise
{
	public:
		PolygonalMapGenNoise(int seed = 1337) { SetSeed(seed); };
		~PolygonalMapGenNoise() { delete m_cellularNoiseLookup; }

		//enum ENoiseType { Value, ValueFractal, Gradient, GradientFractal, Simplex, SimplexFractal, Cellular, WhiteNoise };
		//enum EInterp { InterpLinear, InterpHermite, InterpQuintic };
		//enum EFractalType { FBM, Billow, RigidMulti };
		//enum ECellularDistanceFunction { Euclidean, Manhattan, Natural };
		//enum ECellularReturnType { CellValue, NoiseLookup, Distance, Distance2, Distance2Add, Distance2Sub, Distance2Mul, Distance2Div };

		void SetSeed(int seed);
		int GetSeed(void) const { return m_seed; }
		void SetNoiseSettings(FNoiseSettings m_noiseSettings);
		void SetFrequency(float frequency) { m_frequency = frequency; }
		void SetInterp(EInterp interp) { m_interp = interp; }
		void SetNoiseType(ENoiseType noiseType) { m_noiseType = noiseType; }

		void SetFractalOctaves(unsigned int octaves) { m_octaves = octaves; }
		void SetFractalLacunarity(float lacunarity) { m_lacunarity = lacunarity; }
		void SetFractalGain(float gain) { m_gain = gain; }
		void SetFractalType(EFractalType fractalType) { m_fractalType = fractalType; }

		void SetCellularDistanceFunction(ECellularDistanceFunction cellularDistanceFunction) { m_cellularDistanceFunction = cellularDistanceFunction; }
		void SetCellularReturnType(ECellularReturnType cellularReturnType) { m_cellularReturnType = cellularReturnType; }
		void SetCellularNoiseLookup(PolygonalMapGenNoise* noise) { m_cellularNoiseLookup = noise; }

		/*
		Timing below are averages of time taken for 1 million iterations on a single thread
		Default noise settings
		CPU: i7 4790k @ 4.0Ghz
		VS 2013 - C++ Console Application
		*/

		//3D												// Win32	x64
		float GetValue(float x, float y, float z);			// 14 ms	14 ms
		float GetValueFractal(float x, float y, float z);	// 48 ms	49 ms

		float GetGradient(float x, float y, float z);		// 23 ms	22 ms
		float GetGradientFractal(float x, float y, float z);// 80 ms	73 ms

		float GetSimplex(float x, float y, float z);		// 30 ms	30 ms
		float GetSimplexFractal(float x, float y, float z);	// 98 ms	101 ms

		float GetCellular(float x, float y, float z);		// 123 ms	113 ms

		float GetWhiteNoise(float x, float y, float z);		// 1.5 ms	1.5 ms
		float GetWhiteNoiseInt(int x, int y, int z);

		float GetNoise(float x, float y, float z);

		//2D												// Win32	x64
		float GetValue(float x, float y);					// 8 ms 	8 ms
		float GetValueFractal(float x, float y);			// 29 ms	29 ms

		float GetGradient(float x, float y);				// 12 ms	11 ms
		float GetGradientFractal(float x, float y);			// 43 ms	40 ms

		float GetSimplex(float x, float y);					// 17 ms	17 ms
		float GetSimplexFractal(float x, float y);			// 55 ms	52 ms

		float GetCellular(float x, float y);				// 35 ms	33 ms

		float GetWhiteNoise(float x, float y);				// 1 ms		1 ms
		float GetWhiteNoiseInt(int x, int y);				// 1 ms		1 ms

		float GetNoise(float x, float y);

		//4D
		float GetSimplex(float x, float y, float z, float w);

		float GetWhiteNoise(float x, float y, float z, float w);
		float GetWhiteNoiseInt(int x, int y, int z, int w);

	protected:
		unsigned char m_perm[512];
		unsigned char m_perm12[512];

		int m_seed = 1337;
		float m_frequency = 0.01f;
		EInterp m_interp = EInterp::InterpQuintic;
		ENoiseType m_noiseType = ENoiseType::Simplex;

		unsigned int m_octaves = 3;
		float m_lacunarity = 2.0f;
		float m_gain = 0.5f;
		EFractalType m_fractalType = EFractalType::FBM;
		FVector2D m_noiseRange = FVector2D(0.0f, 1.0f);

		ECellularDistanceFunction m_cellularDistanceFunction = ECellularDistanceFunction::Euclidean;
		ECellularReturnType m_cellularReturnType = ECellularReturnType::CellValue;
		PolygonalMapGenNoise* m_cellularNoiseLookup = nullptr;

		//2D
		float SingleValueFractalFBM(float x, float y);
		float SingleValueFractalBillow(float x, float y);
		float SingleValueFractalRigidMulti(float x, float y);
		float SingleValue(unsigned char offset, float x, float y);

		float SingleGradientFractalFBM(float x, float y);
		float SingleGradientFractalBillow(float x, float y);
		float SingleGradientFractalRigidMulti(float x, float y);
		float SingleGradient(unsigned char offset, float x, float y);

		float SingleSimplexFractalFBM(float x, float y);
		float SingleSimplexFractalBillow(float x, float y);
		float SingleSimplexFractalRigidMulti(float x, float y);
		float SingleSimplex(unsigned char offset, float x, float y);

		float SingleCellular(float x, float y);
		float SingleCellular2Edge(float x, float y);

		//3D
		float SingleValueFractalFBM(float x, float y, float z);
		float SingleValueFractalBillow(float x, float y, float z);
		float SingleValueFractalRigidMulti(float x, float y, float z);
		float SingleValue(unsigned char offset, float x, float y, float z);

		float SingleGradientFractalFBM(float x, float y, float z);
		float SingleGradientFractalBillow(float x, float y, float z);
		float SingleGradientFractalRigidMulti(float x, float y, float z);
		float SingleGradient(unsigned char offset, float x, float y, float z);

		float SingleSimplexFractalFBM(float x, float y, float z);
		float SingleSimplexFractalBillow(float x, float y, float z);
		float SingleSimplexFractalRigidMulti(float x, float y, float z);
		float SingleSimplex(unsigned char offset, float x, float y, float z);

		float SingleCellular(float x, float y, float z);
		float SingleCellular2Edge(float x, float y, float z);

		//4D
		float SingleSimplex(unsigned char offset, float x, float y, float z, float w);

	private:
		inline unsigned char Index2D_12(unsigned char offset, int x, int y);
		inline unsigned char Index3D_12(unsigned char offset, int x, int y, int z);
		inline unsigned char Index4D_32(unsigned char offset, int x, int y, int z, int w);
		inline unsigned char Index2D_256(unsigned char offset, int x, int y);
		inline unsigned char Index3D_256(unsigned char offset, int x, int y, int z);
		inline unsigned char Index4D_256(unsigned char offset, int x, int y, int z, int w);

		inline float ValCoord2DFast(unsigned char offset, int x, int y);
		inline float ValCoord3DFast(unsigned char offset, int x, int y, int z);
		inline float GradCoord2D(unsigned char offset, int x, int y, float xd, float yd);
		inline float GradCoord3D(unsigned char offset, int x, int y, int z, float xd, float yd, float zd);
		inline float GradCoord4D(unsigned char offset, int x, int y, int z, int w, float xd, float yd, float zd, float wd);
};
