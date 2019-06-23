#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Tickable.h"
#include "Building.h"
#include "SlateBrush.h"
#include "Json.h"
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

	UPROPERTY(BlueprintReadWrite)
	TMap<EResourceType, int> ResourcesRequired;

	UPROPERTY(BlueprintReadWrite)
	int BuildersRequired;

	FBuildingDesc() : Unlocked(false)
	{
		ResourcesRequired.Add(EResourceType::Wood, 0);
		ResourcesRequired.Add(EResourceType::Metal, 0);
		ResourcesRequired.Add(EResourceType::Coal, 0);
		ResourcesRequired.Add(EResourceType::Oil, 0);
		ResourcesRequired.Add(EResourceType::Silicon, 0);
	}

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
		
		/* ------ Energy methods ------ */

		UFUNCTION(BlueprintCallable)
		void AddEnergyConsumption(float watts);

		UFUNCTION(BlueprintCallable)
		float GetEnergyConsumption() { return EnergyConsumption; }

		UFUNCTION(BlueprintCallable)
		FString GetFormattedEnergyConsumption();

		/* ---------------------------- */

		/* ----- Resource methods ----- */

		UFUNCTION(BlueprintCallable)
		void AddResourceAmount(TMap<EResourceType, int> res);

		UFUNCTION(BlueprintCallable)
		void SubtractResourceAmount(TMap<EResourceType, int> res);

		UFUNCTION(BlueprintCallable)
		int GetResourceAmount(EResourceType res) { return Resources[res](); }

		UFUNCTION(BlueprintCallable)
		void UnlockResource(EResourceType res) { Resources[res].IsUnlocked = true; }

		UFUNCTION(BlueprintCallable)
		bool IsResourceUnlocked(EResourceType res) { return Resources[res].IsUnlocked; }

		/* -----------------------------*/

		/* ----- Building methods ----- */

		UFUNCTION(BlueprintCallable)
		bool IsBuildingUnlocked() { return false; }

		UFUNCTION(BlueprintCallable)
		bool CanPlaceBuilding(FBuildingDesc building);

		/* -----------------------------*/

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

		/* ------- Load settings ------ */

		TSharedPtr<FJsonObject> GetSettingsJson();
		void LoadBuildings();
		void LoadPrefixes();
		void LoadResources();

		/* ---------------------------- */
};
