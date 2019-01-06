/*
* Based on https://github.com/redblobgames/dual-mesh
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
*
* Unreal Engine 4 Dual Mesh implementation test cases.
*/

#pragma once

#include <vector>

#include "CoreMinimal.h"

#include "Delaunator/Public/DelaunayHelper.h"

#include "RandomSampling/PoissonDiscUtilities.h"
#include "TriangleDualMesh.h"

#define BAD_ANGLE_LIMIT 20.0f

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPoissonSamplingTest, "Procedural Generation.Poisson Disk Sampling.Check Sampling", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter | EAutomationTestFlags::LowPriority)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPointInequalityTest, "Procedural Generation.DualMesh.Check Point Inequality", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter | EAutomationTestFlags::LowPriority)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTriangleInequalityTest, "Procedural Generation.DualMesh.Check Triangle Inequality", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter | EAutomationTestFlags::MediumPriority)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMeshConnectivityTest, "Procedural Generation.DualMesh.Check Region Circulation", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter | EAutomationTestFlags::MediumPriority)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FConstructDualMeshTest, "Procedural Generation.DualMesh.Construct Dual Mesh", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter | EAutomationTestFlags::HighPriority)

TArray<FVector2D> GeneratePoints()
{
	TArray<FVector2D> points;

	FVector2D size = FVector2D(1000.0f, 1000.0f);
	float minimumDistance = 10.0f;
	int32 maxStepSamples = 30;
	int32 rngSeed = 0;
	UPoissonDiscUtilities::Distribute2D(points, rngSeed, size, FVector2D::ZeroVector, minimumDistance, maxStepSamples);

	return points;
}

UTriangleDualMesh* GenerateMeshBuilder()
{
	UDualMeshBuilder* builder = NewObject<UDualMeshBuilder>();
	if (builder == NULL)
	{
		UE_LOG(LogDualMesh, Error, TEXT("Could not create Dual Mesh Builder!"));
		return NULL;
	}
	builder->Initialize(FVector2D(1000.0f, 1000.0f), 10);
	FRandomStream rng(0);
	builder->AddPoisson(rng);
	return builder->Create();
}

bool FPoissonSamplingTest::RunTest(const FString& Parameters)
{
	TArray<FVector2D> points = GeneratePoints();
	FVector2D size = FVector2D(1000.0f, 1000.0f);

	UE_LOG(LogDualMesh, Log, TEXT("Generated %d poisson-distributed points (Round 1)"), points.Num());

	if (points.Num() == 0)
	{
		UE_LOG(LogDualMesh, Error, TEXT("PoissonDisc didn't distribute any points!"));
		return false;
	}
	for (int i = 0; i < points.Num(); i++)
	{
		FVector2D point = points[i];
		if (point.X < 0.0f || point.Y < 0.0f || point.X > size.X || point.Y > size.Y)
		{
			UE_LOG(LogDualMesh, Error, TEXT("PoissonDisc generated out of bounds point!"));
			return false;
		}
	}

	size = FVector2D(2000.0f, 2000.0f);
	float minimumDistance = 50.0f;
	int32 maxStepSamples = 30;
	UPoissonDiscUtilities::Distribute2D(points, 1, size, FVector2D::ZeroVector, minimumDistance, maxStepSamples);

	UE_LOG(LogDualMesh, Log, TEXT("Generated %d poisson-distributed points (Round 2)"), points.Num());

	for (int i = 0; i < points.Num(); i++)
	{
		FVector2D point = points[i];
		if (point.X < 0.0f || point.Y < 0.0f || point.X > size.X || point.Y > size.Y)
		{
			UE_LOG(LogDualMesh, Error, TEXT("PoissonDisc generated out of bounds point!"));
			return false;
		}
	}
	return true;
}

bool FPointInequalityTest::RunTest(const FString& Parameters)
{
	FDelaunayMesh graph = FDelaunayMesh(GeneratePoints());

	// Check for triangles with no area
	for (int t = 0; t < graph.DelaunayTriangles.Num(); t += 3)
	{
		FDelaunayTriangle triangle = UDelaunayHelper::ConvertTriangleIDToTriangle(graph, t);
		float area = triangle.GetArea();
		if (FMath::IsNearlyEqual(area, 0.0f))
		{
			UE_LOG(LogDualMesh, Error, TEXT("Triangle %d had no area!"), t);
			//return false;
		}
	}

	// Check for parallel lines
	for (int i = -1; i < graph.HalfEdges.Num(); i++)
	{
		FDelaunayTriangle triangle;
		if (i == -1)
		{
			// The first case just verifies that the test works as it should,
			// given a known circumcenter
			TArray<FVector2D> points = { FVector2D(6.0f, 0.0f), FVector2D(0.0f, 0.0f), FVector2D(0.0f, 8.0f) };
			// This will give the circumcenter (3, 4)
			triangle = FDelaunayTriangle(points[0], points[1], points[2], 0, 1, 2);
		}
		else
		{
			triangle = UDelaunayHelper::GetTriangleFromHalfEdge(graph, i);
		}
		if (!triangle.IsValid())
		{
			continue;
		}

		FVector2D p = triangle.A;
		FVector2D q = triangle.B;
		FVector2D r = triangle.C;
		// Line PQ is represented as ax + by = c
		float a = q.Y - p.Y;
		float b = p.X - q.X;
		//float c = a * (p.X) + b * (p.Y);

		// Line QR is represented as ex + fy = g 
		float e = r.Y - q.Y;
		float f = q.X - r.X;
		//float g = e * (q.X) + f * (q.Y);

		// Converting lines PQ and QR to perpendicular 
		// bisectors. After this, L = ax + by = c 
		// M = ex + fy = g 
		FVector2D midpointPQ = FVector2D((p.X + q.X) / 2.0f, (p.Y + q.Y) / 2.0f);
		// c = -bx + ay 
		float c = -b * (midpointPQ.X) + a * (midpointPQ.Y);
		float temp = a;
		a = -b;
		b = temp;
		FVector2D midpointQR = FVector2D((q.X + r.X) / 2.0f, (q.Y + r.Y) / 2.0f);
		// g = -fx + ey 
		float g = -f * (midpointQR.X) + e * (midpointQR.Y);
		temp = e;
		e = -f;
		f = temp;

		float determinant = a * f - e * b;
		if (determinant == 0.0f)
		{
			// The lines are parallel!
			UE_LOG(LogDualMesh, Error, TEXT("Triangle %d had parallel lines!"), i);
			return false;
		}
		else
		{
			// We've come this far; may as well verify circumcenters "for free"
			float x = (f * c - b * g) / determinant;
			float y = (a * g - e * c) / determinant;
			FVector2D calculatedCircumcenter = FVector2D(x, y);
			FVector2D reportedCircumcenter = triangle.GetCircumcenter();
			if (!calculatedCircumcenter.Equals(reportedCircumcenter, 1.0f))
			{
				UE_LOG(LogDualMesh, Error, TEXT("Triangle %d had a bad circumcenter! Calculated: (%f, %f); reported: (%f, %f). Points: (%f, %f), (%f, %f), (%f, %f)"), i, calculatedCircumcenter.X, calculatedCircumcenter.Y, reportedCircumcenter.X, reportedCircumcenter.Y, p.X, p.Y, q.X, q.Y, r.X, r.Y);
				return false;
			}
		}
	}
	return true;
}

bool FTriangleInequalityTest::RunTest(const FString& Parameters)
{
	// Check for skinny triangles
	TArray<int32> summary;
	summary.SetNumZeroed(BAD_ANGLE_LIMIT);
	int32 count = 0;

	FDelaunayMesh graph = FDelaunayMesh(GeneratePoints());

	for (int i = 0; i < graph.DelaunayTriangles.Num(); i += 3)
	{
		FDelaunayTriangle triangle = UDelaunayHelper::ConvertTriangleIDToTriangle(graph, i);
		FVector2D a = FVector2D(triangle.A.X - triangle.B.X, triangle.A.Y - triangle.B.Y);
		FVector2D b = FVector2D(triangle.A.X - triangle.C.X, triangle.A.Y - triangle.C.Y);
		float angleRadians = FMath::Acos((a.X * b.X + a.Y * b.Y) / (a.Size() * b.Size()));
		int32 angleDegrees = FMath::RoundToInt(FMath::RadiansToDegrees(angleRadians));
		if (angleDegrees < BAD_ANGLE_LIMIT)
		{
			summary[angleDegrees]++;
			count++;
		}
	}

	if (count > 0)
	{
		UE_LOG(LogDualMesh, Warning, TEXT("%d bad angles"), count);
		for (int i = 0; i < summary.Num(); i++)
		{
			UE_LOG(LogDualMesh, Warning, TEXT("Number of %d degree bad angles: %d"), i, summary[i]);
		}
		//return false;
	}
	return true;
}

bool FMeshConnectivityTest::RunTest(const FString& Parameters)
{
	FVector2D size = FVector2D(1000.0f, 1000.0f);
	FDualMesh mesh = FDualMesh(GeneratePoints(), size);

	int32 ghostR = mesh.Coordinates.Num() - 1;
	TArray<int32> outS;
	for (int32 s0 = 0; s0 < mesh.DelaunayTriangles.Num(); s0++)
	{
		if (mesh.HalfEdges[mesh.HalfEdges[s0]] != s0)
		{
			UE_LOG(LogDualMesh, Error, TEXT("Half-edges at vertex %d don't point back to itself (%d, %d, %d)!"), s0, s0, mesh.HalfEdges[s0], mesh.HalfEdges[mesh.HalfEdges[s0]]);
			return false;
		}
		int32 s = s0;
		int32 startRegion = mesh.DelaunayTriangles[s0];

		outS.Empty();
		do
		{
			if (outS.Num() >= 100 && startRegion != ghostR)
			{
				FString outSOutput = "";
				for (int i = 0; i < outS.Num(); i++)
				{
					outSOutput.AppendInt(outS[i]);
					outSOutput.Append(", ");
				}
				int32 endRegion = mesh.DelaunayTriangles[UTriangleDualMesh::s_next_s(s0)];
				UE_LOG(LogDualMesh, Error, TEXT("Failed to circulate around region with start side %d from region %d to %d. OutS: %s"), s0, startRegion, endRegion, *outSOutput);
				return false;
			}
			if (!mesh.HalfEdges.IsValidIndex(s))
			{
				FString outSOutput = "";
				for (int i = 0; i < outS.Num(); i++)
				{
					outSOutput.AppendInt(outS[i]);
					outSOutput.Append(", ");
				}
				int32 endRegion = mesh.DelaunayTriangles[UTriangleDualMesh::s_next_s(s0)];
				UE_LOG(LogDualMesh, Error, TEXT("Invalid index! %d out of size %d."), s, mesh.HalfEdges.Num());
				UE_LOG(LogDualMesh, Error, TEXT("Failed to circulate around region with start side %d from region %d to %d. OutS: %s"), s0, startRegion, endRegion, *outSOutput);
				return false;
			}

			outS.Add(s);
			s = UTriangleDualMesh::s_next_s(mesh.HalfEdges[s]);
		} while (s != s0);
	}
	return true;
}

bool FConstructDualMeshTest::RunTest(const FString& Parameters)
{
	UTriangleDualMesh* mesh = GenerateMeshBuilder();
	return mesh != NULL;
}