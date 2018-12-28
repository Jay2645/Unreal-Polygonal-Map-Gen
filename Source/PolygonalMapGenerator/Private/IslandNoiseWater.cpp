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

#include "IslandNoiseWater.h"

bool UIslandNoiseWater::IsPointLand_Implementation(FPointIndex Point, UTriangleDualMesh* Mesh, const FVector2D& HalfMeshSize, const FVector2D& Offset, const FIslandShape& Shape) const
{
	FVector2D nVector = Mesh->r_pos(Point);
	nVector.X /= HalfMeshSize.X;
	nVector.Y /= HalfMeshSize.Y;
	nVector = (nVector + Offset) * Shape.IslandFragmentation;
	float n = UIslandMapUtils::FBMNoise(Shape.Amplitudes, nVector);
	float distance = FMath::Max(FMath::Abs(nVector.X), FMath::Abs(nVector.Y));
	return n * distance * distance > WaterCutoff;
}
