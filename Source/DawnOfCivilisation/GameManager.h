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
	Achievement			 UMETA(DisplayName = "Achievement"),
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

/**
 * 
 */
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
		bool IsBuildingUnlocked() { return false; }

		UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TMap<float, FGameEvent> Milestones;

		UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FBuildingDesc> Buildings;

		UPROPERTY(BlueprintReadWrite)
		int64 EnergyConsumption;
};
