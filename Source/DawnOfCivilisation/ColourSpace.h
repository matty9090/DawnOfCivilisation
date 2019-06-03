// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ColourSpace.generated.h"

/**
 * 
 */
UCLASS()
class DAWNOFCIVILISATION_API UColourSpace : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	public:
		static FVector RGBToHSL(FVector rgb);
		static FVector HSLToRGB(FVector hsl);

private:
		static float HueCalculations(float v1, float v2, float hue);
};
