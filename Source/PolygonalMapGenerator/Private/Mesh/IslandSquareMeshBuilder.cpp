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

#include "IslandSquareMeshBuilder.h"
#include "DualMeshBuilder.h"

UIslandSquareMeshBuilder::UIslandSquareMeshBuilder()
{
	NumberOfPoints = 1000;
}

void UIslandSquareMeshBuilder::AddPoints_Implementation(UDualMeshBuilder* Builder, FRandomStream& Rng) const
{
	int32 gridSize = FMath::CeilToInt(FMath::Sqrt(NumberOfPoints));
	for (int32 x = 0; x < gridSize; x++)
	{
		for (int32 y = 0; y < gridSize; y++)
		{
			Builder->AddPoint(FVector2D((0.5f + x) / gridSize * MapSize.X, (0.5f + y) / gridSize * MapSize.Y));
		}
	}
}
