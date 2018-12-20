/*
* Based on https://github.com/Xaymar/RandomDistributionPlugin
* Original work copyright 2014 Michael Fabian Dirks
* Modified version copyright 2018 Jay Stevens <jaystevens42@gmail.com>
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

#include "PoissonDiscUtilities.h"

void UPoissonDiscUtilities::Distribute2D(TArray<FVector2D>& Samples, int32 Seed, FVector2D Size, FVector2D StartLocation, float MinimumDistance, int32 MaxStepSamples, bool WrapX, bool WrapY)
{
	uint64 iCells, iCellsX, iCellsY, iCell, iCellX, iCellY;
	double dCellSize;
	FVector2D* v2DSample; FVector2D** v2DSamples;
	TDoubleLinkedList<FVector2D*>* v2DList;
	FRandomStream rsRandomStream = FRandomStream(Seed);

	// Calculate cell size, count and total.
	dCellSize = MinimumDistance / sqrt(2.0);
	iCellsX = ceil(Size.X / dCellSize);
	iCellsY = ceil(Size.Y / dCellSize);
	iCells = iCellsX * iCellsY;

	// Initialize temporary arrays and lists for the generated elements.
	v2DSamples = new FVector2D*[iCells];
	v2DList = new TDoubleLinkedList<FVector2D*>();
	for (uint64 n = 0; n < iCells; ++n)
		v2DSamples[n] = NULL;

	// Generate starting sample.
	v2DSample = new FVector2D(rsRandomStream.FRandRange(0, Size.X), rsRandomStream.FRandRange(0, Size.Y));
	iCellX = (int)(v2DSample->X / dCellSize);
	iCellY = (int)(v2DSample->Y / dCellSize);
	iCell = iCellX + (iCellY * iCellsX);
	v2DSamples[iCell] = v2DSample;
	v2DList->AddTail(v2DSample);

	// Generate samples until the processing list is empty (no more samples can be generated).
	while (v2DList->Num() > 0) {
		FVector2D* v2DSampleOrigin = v2DList->GetHead()->GetValue();

		// Check if the retrieved sample is NULL instead of a valid vector,
		if (v2DSampleOrigin == NULL) { // and break out instead.
			break;
		}

		// Now try and generate samples around the sample origin.
		bool bIsSuccessful = false;
		for (uint32 i = 0; i < (uint32)MaxStepSamples; ++i) {
			double dAngle, dRadius;
			dAngle = rsRandomStream.FRandRange(0, PI * 2.0f) + (double)i;
			dRadius = rsRandomStream.FRandRange(1.0f, 2.0f) * (double)MinimumDistance;
			v2DSample = new FVector2D(v2DSampleOrigin->X + (dRadius * cos(dAngle)), v2DSampleOrigin->Y + (dRadius * sin(dAngle)));

			// Destroy sample if outside of boundaries.
			if ((v2DSample->X < 0 || v2DSample->X > Size.X) || (v2DSample->Y < 0 || v2DSample->Y > Size.Y)) {
				delete v2DSample; continue;
			}
			else {
				// Check if it too close to any other sample generated.
				iCellX = (uint64)(v2DSample->X / dCellSize);
				iCellY = (uint64)(v2DSample->Y / dCellSize);
				iCell = iCellX + (iCellY * iCellsX);
				if (iCell < 0 || iCell > iCells) {
					delete v2DSample; continue;
				}
				else {
					for (int x = -2; x <= 2; x++) { // Loop through nearest grid points that are inside the radius.
						for (int y = -2; y <= 2; y++) {
							float fRealX = v2DSample->X, fRealY = v2DSample->Y;
							int64 iTempCellX = iCellX + x;
							int64 iTempCellY = iCellY + y;

							if (!WrapX) {
								if (iTempCellX < 0 || iTempCellX >= (int64)iCellsX)
									continue;
							}
							else {
								if (iTempCellX < 0) {
									iTempCellX = iTempCellX + iCellsX;
									v2DSample->X = fRealX + Size.X;
								}
								else if (iTempCellX >= (int64)iCellsX) {
									iTempCellX = iTempCellX - iCellsX;
									v2DSample->X = fRealX - Size.X;
								}
							}
							if (!WrapY) {
								if (iTempCellY < 0 || iTempCellY >= (int64)iCellsY)
									continue;
							}
							else {
								if (iTempCellY < 0) {
									iTempCellY = iTempCellY + iCellsY;
									v2DSample->Y = fRealY + Size.Y;
								}
								else if (iTempCellY >= (int64)iCellsY) {
									iTempCellY = iTempCellY - iCellsY;
									v2DSample->Y = fRealY - Size.Y;
								}
							}
							int64 iTempCell = iTempCellX + (iTempCellY * iCellsX);

							if (v2DSamples[iTempCell] != NULL) {
								double dDist = FVector2D::Distance(*v2DSamples[iTempCell], *v2DSample);
								if (dDist < MinimumDistance) {
									delete v2DSample;
									goto PoissonDisc2DCheckBreakout;
								}
							}

							if (WrapX || WrapY) {
								v2DSample->X = fRealX;
								v2DSample->Y = fRealY;
							}
						}
					}
				}
			}

			v2DSamples[iCell] = v2DSample;
			v2DList->AddTail(v2DSample);
			bIsSuccessful = true;

		PoissonDisc2DCheckBreakout:;
			//continue;
		}

		// If we weren't successful in generating any new points, remove this point from the working list.
		if (!bIsSuccessful)
			v2DList->RemoveNode(v2DSampleOrigin);
	}

	// Fill up the output array with generated samples.
	for (uint64 n = 0; n < iCells; ++n) {
		if (v2DSamples[n] != NULL)
			Samples.Add(*v2DSamples[n] + StartLocation);
	}

	// Destroy temporary arrays and lists.
	delete v2DSamples; delete v2DList;
}

void UPoissonDiscUtilities::Distribute3D(TArray<FVector>& Samples, int32 Seed /* = 0 */, FVector Size /* = FVector2D(1.0f , 1.0f) */, float MinimumDistance /* = 1.0f */, int32 MaxStepSamples /* = 30 */, bool WrapX /* = false */, bool WrapY /* = false */, bool WrapZ /* = false */)
{
	uint64 iCells, iCellsX, iCellsY, iCellsZ, iCell, iCellX, iCellY, iCellZ;
	double dCellSize;
	FVector* v3DSample; FVector** v3DSamples;
	TDoubleLinkedList<FVector*>* v3DList;
	FRandomStream rsRandomStream = FRandomStream(Seed);

	// Calculate cell size, count and total.
	dCellSize = MinimumDistance / sqrt(2.0);
	iCellsX = ceil(Size.X / dCellSize);
	iCellsY = ceil(Size.Y / dCellSize);
	iCellsZ = ceil(Size.Z / dCellSize);
	iCells = iCellsX * iCellsY * iCellsZ;

	// Initialize temporary arrays and lists for the generated elements.
	v3DSamples = new FVector*[iCells];
	v3DList = new TDoubleLinkedList<FVector*>();
	for (uint64 n = 0; n < iCells; ++n)
		v3DSamples[n] = NULL;

	// Generate starting sample.
	v3DSample = new FVector(rsRandomStream.FRandRange(0, Size.X), rsRandomStream.FRandRange(0, Size.Y), rsRandomStream.FRandRange(0, Size.Z));
	iCellX = (int)(v3DSample->X / dCellSize);
	iCellY = (int)(v3DSample->Y / dCellSize);
	iCellZ = (int)(v3DSample->Z / dCellSize);
	iCell = iCellX + (iCellY * iCellsX) + (iCellZ * iCellsX * iCellY);
	v3DSamples[iCell] = v3DSample;
	v3DList->AddTail(v3DSample);

	// Generate samples until the processing list is empty (no more samples can be generated).
	while (v3DList->Num() > 0) {
		FVector* v3DSampleOrigin = v3DList->GetHead()->GetValue();

		// Check if the retrieved sample is NULL instead of a valid vector,
		if (v3DSampleOrigin == NULL) { // and break out instead.
			break;
		}

		// Now try and generate samples around the sample origin.
		bool bIsSuccessful = false;
		for (uint32 i = 0; i < (uint32)MaxStepSamples; ++i) {
			double dYawAngle, dPitchAngle, dRadius;
			dYawAngle = rsRandomStream.FRandRange(0, PI * 2.0f) + (double)i;
			dPitchAngle = rsRandomStream.FRandRange(0, PI) + (double)i;
			dRadius = rsRandomStream.FRandRange(1.0f, 2.0f) * (double)MinimumDistance;
			v3DSample = new FVector(v3DSampleOrigin->X + (dRadius * cos(dYawAngle) * sin(dPitchAngle)),
				v3DSampleOrigin->Y + (dRadius * sin(dYawAngle) * sin(dPitchAngle)),
				v3DSampleOrigin->Z + (dRadius * cos(dPitchAngle)));

			// Destroy sample if outside of boundaries.
			if ((v3DSample->X < 0 || v3DSample->X > Size.X) || (v3DSample->Y < 0 || v3DSample->Y > Size.Y) || (v3DSample->Z < 0 || v3DSample->Z > Size.Z)) {
				delete v3DSample; continue;
			}
			else {
				// Check if it too close to any other sample generated.
				iCellX = (uint64)(v3DSample->X / dCellSize);
				iCellY = (uint64)(v3DSample->Y / dCellSize);
				iCellZ = (uint64)(v3DSample->Z / dCellSize);
				iCell = iCellX + (iCellY * iCellsX) + (iCellZ * iCellsX * iCellsY);
				if (iCell < 0 || iCell > iCells) {
					delete v3DSample; continue;
				}
				else {
					for (int x = -2; x <= 2; x++) { // Loop through nearest grid points that are inside the radius.
						for (int y = -2; y <= 2; y++) {
							for (int z = -2; z <= 2; z++) {
								float fRealX = v3DSample->X, fRealY = v3DSample->Y, fRealZ = v3DSample->Z;
								int64 iTempCellX = iCellX + x;
								int64 iTempCellY = iCellY + y;
								int64 iTempCellZ = iCellZ + z;

								if (!WrapX) {
									if (iTempCellX < 0 || iTempCellX >= (int64)iCellsX)
										continue;
								}
								else {
									if (iTempCellX < 0) {
										iTempCellX = iTempCellX + iCellsX;
										v3DSample->X = fRealX + Size.X;
									}
									else if (iTempCellX >= (int64)iCellsX) {
										iTempCellX = iTempCellX - iCellsX;
										v3DSample->X = fRealX - Size.X;
									}
								}
								if (!WrapY) {
									if (iTempCellY < 0 || iTempCellY >= (int64)iCellsY)
										continue;
								}
								else {
									if (iTempCellY < 0) {
										iTempCellY = iTempCellY + iCellsY;
										v3DSample->Y = fRealY + Size.Y;
									}
									else if (iTempCellY >= (int64)iCellsY) {
										iTempCellY = iTempCellY - iCellsY;
										v3DSample->Y = fRealY - Size.Y;
									}
								}
								if (!WrapZ) {
									if (iTempCellZ < 0 || iTempCellZ >= (int64)iCellsZ)
										continue;
								}
								else {
									if (iTempCellZ < 0) {
										iTempCellZ = iTempCellZ + iCellsZ;
										v3DSample->Z = fRealZ + Size.Z;
									}
									else if (iTempCellZ >= (int64)iCellsZ) {
										iTempCellZ = iTempCellZ - iCellsZ;
										v3DSample->Z = fRealZ - Size.Z;
									}
								}
								int64 iTempCell = iTempCellX + (iTempCellY * iCellsX) + (iTempCellZ * iCellsX * iCellsY);

								if (v3DSamples[iTempCell] != NULL) {
									double dDist = FVector::Dist(*v3DSamples[iTempCell], *v3DSample);
									if (dDist < MinimumDistance) {
										delete v3DSample;
										goto PoissonDisc2DCheckBreakout;
									}
								}

								if (WrapX || WrapY || WrapZ) {
									v3DSample->X = fRealX;
									v3DSample->Y = fRealY;
									v3DSample->Z = fRealZ;
								}
							}
						}
					}
				}
			}

			v3DSamples[iCell] = v3DSample;
			v3DList->AddTail(v3DSample);
			bIsSuccessful = true;

		PoissonDisc2DCheckBreakout:;
			//continue;
		}

		// If we weren't successful in generating any new points, remove this point from the working list.
		if (!bIsSuccessful)
		{
			v3DList->RemoveNode(v3DSampleOrigin);
		}
	}

	// Fill up the output array with generated samples.
	for (uint64 n = 0; n < iCells; ++n)
	{
		if (v3DSamples[n] != NULL)
			Samples.Add(*v3DSamples[n]);
	}

	// Destroy temporary arrays and lists.
	delete v3DSamples; delete v3DList;
}