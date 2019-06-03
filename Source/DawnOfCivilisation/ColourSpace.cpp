// Fill out your copyright notice in the Description page of Project Settings.


#include "ColourSpace.h"

FVector UColourSpace::RGBToHSL(FVector rgb)
{
	float H = 0.0f, S = 0.0f, L = 0.0f;

	float fr = rgb.X;
	float fg = rgb.Y;
	float fb = rgb.Z;

	float min = FMath::Min3(fr, fg, fb);
	float max = FMath::Max3(fr, fg, fb);

	L = 50.0f * (max + min);

	if (min == max) {
		S = H = 0;
		return FVector(H, S, L);
	}

	if (L < 50.0f)
		S = 100.0f * (max - min) / (max + min);
	else
		S = 100.0f * (max - min) / (2.0f - max - min);

	if (max == fr) H = 60.0f * (fg - fb) / (max - min);
	if (max == fg) H = 60.0f * (fb - fr) / (max - min) + 120.0f;
	if (max == fb) H = 60.0f * (fr - fg) / (max - min) + 240.0f;

	if (H < 0.0f) H += 360.0f;

	return FVector(H, S, L);
}

FVector UColourSpace::HSLToRGB(FVector hsl)
{
	float H = hsl.X, S = hsl.Y, L = FMath::Clamp(hsl.Z, 0.0f, 100.0f);
	float R, G, B;

	float tempL = L / 100.0f;
	float tempS = S / 100.0f;

	if (tempS == 0)
		R = G = B = tempL * 255;
	else
	{
		float v1, v2;
		float hue = H / 360.0f;

		if (tempL < 0.5f)
		{
			v2 = tempL * (1 + tempS);
		}
		else
		{
			v2 = (tempL + tempS) - (tempL * tempS);
		}
		v1 = 2 * tempL - v2;

		R = HueCalculations(v1, v2, hue + (1.0f / 3));
		G = HueCalculations(v1, v2, hue);
		B = HueCalculations(v1, v2, hue - (1.0f / 3));
	}

	return FVector(R, G, B);
}

float UColourSpace::HueCalculations(float v1, float v2, float hue) {
	if (hue < 0)
		hue += 1;

	if (hue > 1)
		hue -= 1;

	if ((6 * hue) < 1)
		return (v1 + (v2 - v1) * 6 * hue);

	if ((2 * hue) < 1)
		return v2;

	if ((3 * hue) < 2)
		return (v1 + (v2 - v1) * ((2.0f / 3) - hue) * 6);


	return v1;
}