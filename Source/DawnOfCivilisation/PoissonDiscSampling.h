// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <vector>

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PoissonDiscSampling.generated.h"

UCLASS()
class DAWNOFCIVILISATION_API UPoissonDiscSampling : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	public:
		UFUNCTION(BlueprintCallable)
		static TArray<FVector2D> GeneratePoints(float radius, float regionSize, float threshold, float frequency, float fade, int numSamplesBeforeRejection = 30);

	private:
		typedef std::vector<std::vector<int>> Grid;

		static bool IsValid(FVector2D candidate, float regionSize, float cellSize, float radius, TArray<FVector2D> points, Grid grid);
};
