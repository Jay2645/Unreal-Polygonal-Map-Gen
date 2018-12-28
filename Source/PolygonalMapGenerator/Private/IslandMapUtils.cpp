/*
* From http://www.redblobgames.com/maps/mapgen2/
* Original work copyright 2017 Red Blob Games <redblobgames@gmail.com>
* Unreal Engine 4 implementation copyright 2018 Jay Stevens <jaystevens42@gmail.com>
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#include "IslandMapUtils.h"
#include "RandomSampling/SimplexNoise.h"

void UIslandMapUtils::RandomShuffle(TArray<FTriangleIndex>& OutShuffledArray, FRandomStream& Rng)
{
	for (int i = OutShuffledArray.Num() - 1; i > 0; i--)
	{
		int32 j = Rng.RandRange(0, i);
		int32 swap = OutShuffledArray[i];
		OutShuffledArray[i] = OutShuffledArray[j];
		OutShuffledArray[j] = swap;
	}
}

float UIslandMapUtils::FBMNoise(const TArray<float>& Amplitudes, const FVector2D& Position)
{
	float sum = 0.0f;
	float sumOfAmplitudes = 0.0f;
	FCustomSimplexNoise noise;
	for (size_t octave = 0; octave < Amplitudes.Num(); octave++)
	{
		size_t frequency = ((size_t)1) << octave;
		sum += Amplitudes[octave] * noise.fractal(octave, Position * frequency);
		sumOfAmplitudes += Amplitudes[octave];
	}

	if (sumOfAmplitudes == 0.0f)
	{
		return 0.0f;
	}
	return sum / sumOfAmplitudes;
}
