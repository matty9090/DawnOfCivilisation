// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Tickable.h"
#include "Building.h"
#include "SlateBrush.h"
#include "GameManager.generated.h"

UENUM(BlueprintType)
enum class EEvent : uint8
{
	UnlockBuilding 		 UMETA(DisplayName = "Unlock Building"),
	UnlockTechnology 	 UMETA(DisplayName = "Unlock Technology"),
	Achievement			 UMETA(DisplayName = "Achievement")
};

UENUM(BlueprintType)
enum class EResourceType : uint8
{
	Wood 		 UMETA(DisplayName = "Wood"),
	Metal 		 UMETA(DisplayName = "Metal"),
	Coal		 UMETA(DisplayName = "Coal"),
	Oil			 UMETA(DisplayName = "Oil"),
	Silicon		 UMETA(DisplayName = "Silicone")
};

USTRUCT(BlueprintType)
struct FGameEvent
{
	GENERATED_BODY()

	UPROPERTY()
	EEvent Type;
};

USTRUCT(BlueprintType)
struct FBuildingDesc
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString Name;

	UPROPERTY(BlueprintReadWrite)
	FSlateBrush Icon;

	UPROPERTY(BlueprintReadWrite)
	bool Unlocked;

	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<AActor> Building;

	FBuildingDesc() : Unlocked(false) {}

	bool operator==(const FBuildingDesc& other)
	{
		return Name == other.Name;
	}
};

USTRUCT(BlueprintType)
struct FResource
{
	GENERATED_BODY()

	UPROPERTY()
	int Amount;

	UPROPERTY()
	bool IsUnlocked;

	FResource() : Amount(0), IsUnlocked(false) {}
	FResource(bool unlocked) : Amount(0), IsUnlocked(unlocked) {}
	FResource(int val, bool unlocked) : Amount(val), IsUnlocked(unlocked) {}

	void operator+=(int v) { Amount += v; }
	void operator-=(int v) { Amount -= v; }

	int operator+(int v) { return Amount + v; }
	int operator-(int v) { return Amount - v; }

	int operator()() { return Amount; }
};

UCLASS(Blueprintable)
class DAWNOFCIVILISATION_API UGameManager : public UObject, public FTickableGameObject
{
	GENERATED_BODY()
	
	public:
		UGameManager();

		void Tick(float dt) override;
		bool IsTickable() const override { return true; }
		bool IsTickableInEditor() const override { return true; }
		bool IsTickableWhenPaused() const override { return true; }
		TStatId GetStatId() const override { return TStatId(); }

		UFUNCTION(BlueprintCallable)
		FString GetFormattedEnergyConsumption();
		
		UFUNCTION(BlueprintCallable)
		void AddEnergyConsumption(float watts);

		UFUNCTION(BlueprintCallable)
		float GetEnergyConsumption() { return EnergyConsumption; }

		UFUNCTION(BlueprintCallable)
		bool IsBuildingUnlocked() { return false; }

		UFUNCTION(BlueprintCallable)
		void AddResourceAmount(EResourceType res, int val) { Resources[res] += val; }

		UFUNCTION(BlueprintCallable)
		void SubtractResourceAmount(EResourceType res, int val) { Resources[res] -= val; }

		UFUNCTION(BlueprintCallable)
		int GetResourceAmount(EResourceType res) { return Resources[res](); }

		UFUNCTION(BlueprintCallable)
		void UnlockResource(EResourceType res) { Resources[res].IsUnlocked = true; }

		UFUNCTION(BlueprintCallable)
		bool IsResourceUnlocked(EResourceType res) { return Resources[res].IsUnlocked; }

		UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TMap<float, FGameEvent> Milestones;

		UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FBuildingDesc> Buildings;

	private:
		UPROPERTY()
		TMap<int, FString> Prefixes;

		UPROPERTY()
		TMap<EResourceType, FResource> Resources;

		UPROPERTY()
		int CurrentPrefix;

		UPROPERTY()
		float EnergyConsumption;
};
