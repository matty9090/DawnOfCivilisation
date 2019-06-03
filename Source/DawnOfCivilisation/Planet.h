// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Planet.generated.h"

UENUM(BlueprintType)
enum class EPlanetComponent : uint8
{
	Terrain		 UMETA(DisplayName = "Terrain"),
	Water		 UMETA(DisplayName = "Water"),
	Atmosphere	 UMETA(DisplayName = "Atmosphere")
};

USTRUCT(BlueprintType)
struct FScalarRange
{
	GENERATED_BODY()

	public:
		FScalarRange() : Start(0.0f), End(0.0f) {}
		FScalarRange(float v) : Start(v), End(v) {}
		FScalarRange(float s, float e) : Start(s), End(e) {}

		float GetValue(FRandomStream& rand)
		{
			return rand.FRandRange(Start, End);
		}

	private:
		float Start;
		float End;
};

USTRUCT(BlueprintType)
struct FVectorRange
{
	GENERATED_BODY()

	public:
		FVectorRange() : Start(FVector::ZeroVector), End(FVector::ZeroVector) {}
		FVectorRange(FVector v) : Start(v), End(v) {}
		FVectorRange(FVector s, FVector e) : Start(s), End(e) {}

		FVector GetValue(FRandomStream& rand)
		{
			return FVector (rand.FRandRange(Start.X, End.X),
							rand.FRandRange(Start.Y, End.Y),
							rand.FRandRange(Start.Z, End.Z));
		}

	private:
		FVector Start;
		FVector End;
};

USTRUCT(BlueprintType)
struct FBoolRange
{
	GENERATED_BODY()

	public:
		FBoolRange() : Enabled(false), Random(false) {}
		FBoolRange(bool enabled, bool isRandom) : Enabled(enabled), Random(isRandom) {}
	
		static const FBoolRange MakeRandom;
		static const FBoolRange MakeEnabled;
		static const FBoolRange MakeDisabled;

		bool GetValue(FRandomStream& rand)
		{
			return Random ? (bool)rand.RandRange(0, 1) : Enabled;
		}

	private:
		bool Enabled;
		bool Random;
};

USTRUCT(BlueprintType)
struct FNoiseParameters
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FScalarRange NoiseScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FScalarRange NoiseHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FScalarRange Persistence;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FScalarRange OceanDepth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool GenerateHeights;
};

USTRUCT(BlueprintType)
struct FLandFeatures
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVectorRange BeachColour;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVectorRange LandColour;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVectorRange MountainColour;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVectorRange WaterColour;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FScalarRange NumForests;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FScalarRange NumMountains;
};

USTRUCT(BlueprintType)
struct FPlanetPreset
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FBoolRange HasTerrain;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FBoolRange HasWater;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FBoolRange HasAtmosphere;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FNoiseParameters TerrainNoise;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLandFeatures LandFeatures;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVectorRange AtmosphereColour;
};

UCLASS()
class DAWNOFCIVILISATION_API APlanet : public AActor
{
	GENERATED_BODY()
	
	public:	
		APlanet();

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sphere")
		int EditorDivisions;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sphere")
		int PlayDivisions;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
		float Radius;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
		int32 Seed;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
		TArray<FPlanetPreset> Presets;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
		int Preset;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
		TMap<EPlanetComponent, UMaterialInterface*> PlanetMaterials;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geometry")
		TMap<EPlanetComponent, UChildActorComponent*> PlanetComponents;

		UFUNCTION(BlueprintCallable)
		float GetScalarRangeValue(FScalarRange range) { return range.GetValue(RandomStream); }

	protected:
		virtual void BeginPlay() override;
		virtual void OnConstruction(const FTransform& Transform) override;

	public:	
		virtual void Tick(float DeltaTime) override;

	private:
		void Generate();
		UChildActorComponent* CreateChildComponent(UClass* c);

		UPROPERTY()
		FRandomStream RandomStream;
};
