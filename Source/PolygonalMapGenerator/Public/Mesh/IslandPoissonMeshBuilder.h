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

#pragma once

#include "CoreMinimal.h"
#include "IslandMeshBuilder.h"
#include "IslandPoissonMeshBuilder.generated.h"

/**
 * 
 */
UCLASS()
class POLYGONALMAPGENERATOR_API UIslandPoissonMeshBuilder : public UIslandMeshBuilder
{
	GENERATED_BODY()
public:
	// The size of our map, starting at (0, 0).
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Map")
	FVector2D PoissonSize;
	// How much spacing between Poisson points.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Points", meta = (ClampMin = "0.0"))
	float PoissonSpacing;
	// Maximum samples to generate each step.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Points", meta = (ClampMin = "0"))
	int32 PoissonSamples;

public:
	UIslandPoissonMeshBuilder();

protected:
	virtual void AddPoints_Implementation(UDualMeshBuilder* Builder, FRandomStream& Rng) const override;
};
