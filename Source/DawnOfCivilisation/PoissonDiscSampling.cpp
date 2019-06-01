// Fill out your copyright notice in the Description page of Project Settings.


#include "PoissonDiscSampling.h"
#include "SimplexNoiseBPLibrary.h"

#include <algorithm>

TArray<FVector2D> UPoissonDiscSampling::GeneratePoints(float radius, float regionSize, float threshold, float frequency, int numSamplesBeforeRejection)
{
	float cellSize = radius / sqrtf(2.0f);

	Grid grid;
	grid.resize(ceil(regionSize / cellSize));

	for (int y = 0; y < grid.size(); ++y)
		grid[y].resize(ceil(regionSize / cellSize));

	TArray<FVector2D> points, spawnPoints;

	spawnPoints.Add(FVector2D(regionSize / 2, regionSize / 2));

	while (spawnPoints.Num() > 0)
	{
		int spawnIndex = FMath::RandRange(0, spawnPoints.Num() - 1);
		FVector2D spawnCentre = spawnPoints[spawnIndex];
		bool candidateAccepted = false;

		for (int i = 0; i < numSamplesBeforeRejection; i++)
		{
			float angle = FMath::RandRange(0.0f, 1.0f) * PI * 2;
			FVector2D dir(sinf(angle), cosf(angle));
			FVector2D candidate = spawnCentre + dir * FMath::RandRange(radius, 2 * radius);

			if (IsValid(candidate, regionSize, cellSize, radius, points, grid))
			{
				points.Add(candidate);
				spawnPoints.Add(candidate);
				grid[(int)(candidate.X / cellSize)][(int)(candidate.Y / cellSize)] = points.Num();
				candidateAccepted = true;
				break;
			}
		}

		if (!candidateAccepted)
			spawnPoints.RemoveAt(spawnIndex);
	}

	for(int i = 0; i < points.Num();)
	{
		float noise = USimplexNoiseBPLibrary::SimplexNoise2D(points[i].X * frequency, points[i].Y * frequency);
		noise += USimplexNoiseBPLibrary::SimplexNoise2D(points[i].X * frequency * 2, points[i].Y * frequency * 2) * 0.5f;
		noise /= 2;

		float distToCentre = FVector2D::DistSquared(FVector2D(regionSize / 2, regionSize / 2), points[i]);

		if (distToCentre > (regionSize * regionSize) / 4 || noise > threshold )
			points.RemoveAt(i);
		else
			++i;
	}

	return points;
}

bool UPoissonDiscSampling::IsValid(FVector2D candidate, float regionSize, float cellSize, float radius, TArray<FVector2D> points, Grid grid)
{
	if (candidate.X >= 0 && candidate.X < regionSize && candidate.Y >= 0 && candidate.Y < regionSize)
	{
		int cellX = (int)(candidate.X / cellSize);
		int cellY = (int)(candidate.Y / cellSize);

		int searchStartX = std::max(0, cellX - 2);
		int searchEndX = std::min(cellX + 2, (int)grid.size() - 1);
		int searchStartY = std::max(0, cellY - 2);
		int searchEndY = std::min(cellY + 2, (int)grid[0].size() - 1);

		for (int x = searchStartX; x <= searchEndX; x++)
		{
			for (int y = searchStartY; y <= searchEndY; y++)
			{
				int pointIndex = grid[x][y] - 1;

				if (pointIndex != -1)
					if (FVector2D::DistSquared(candidate, points[pointIndex]) < radius * radius)
						return false;
			}
		}

		return true;
	}

	return false;
}
