#include "GameManager.h"
#include "ConstructorHelpers.h"
#include "Engine/Texture.h"
#include "Engine/Blueprint.h"
#include "FileManager.h"
#include "Json.h"

UGameManager::UGameManager() : EnergyConsumption(0.0)
{
	//FJsonSerializer::Deserialize()

	TArray<FString> folders;
	FString path = FPaths::GameContentDir() + TEXT("Models/Buildings/*");

	IFileManager& fileManager = IFileManager::Get();
	fileManager.FindFiles(folders, *path, false, true);
	
	for(auto building : folders)
	{
		FString base = TEXT("/Game/Models/Buildings/");
		FString assetBase = FPaths::GameContentDir() + TEXT("Models/Buildings/") + building + "/" + building;
		FString texPath = building + "/" + building + "_Ico";
		FString bpPath  = building + "/" + building + "_BP";

		if(!fileManager.FileExists(*(assetBase + "_Ico.uasset")))
		{
			UE_LOG(LogTemp, Warning, TEXT("Could not find texture for %s"), *building);
			continue;
		}

		if (!fileManager.FileExists(*(assetBase + "_BP.uasset")))
		{
			UE_LOG(LogTemp, Warning, TEXT("Could not find model blueprint for %s"), *building);
			continue;
		}

		auto tex = ConstructorHelpers::FObjectFinder<UTexture>(*(base + texPath));
		auto bp = ConstructorHelpers::FObjectFinder<UBlueprint>(*(base + bpPath));

		if (!tex.Succeeded()) continue;
		if (!bp.Succeeded())  continue;

		FSlateBrush brush;
		brush.ImageSize = FVector2D(128.0f, 128.0f);
		brush.SetResourceObject(tex.Object);

		FBuildingDesc item;
		item.Name = "Building 0";
		item.Icon = brush;
		item.Unlocked = true;

		if (bp.Object != NULL)
			item.Building = bp.Object->GeneratedClass;
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Error loading class for"), *building);
			continue;
		}

		Buildings.Add(item);
	}
}

void UGameManager::Tick(float dt)
{
	
}