#include "GameManager.h"
#include "ConstructorHelpers.h"
#include "Engine/Texture.h"
#include "Engine/Blueprint.h"
#include "FileManager.h"

UGameManager::UGameManager() : EnergyConsumption(0.0)
{
	TArray<FString> folders;
	FString path = FPaths::GameContentDir() + TEXT("Models/Buildings/*");

	IFileManager& fileManager = IFileManager::Get();
	fileManager.FindFiles(folders, *path, false, true);
	
	for(auto building : folders)
	{
		FString base = TEXT("/Game/Models/Buildings/");
		FString texPath = base  + building + "/" + building + "_Ico";
		FString bpPath  = base  + building + "/" + building + "_BP";

		auto tex = ConstructorHelpers::FObjectFinder<UTexture>(*texPath);
		auto bp = ConstructorHelpers::FObjectFinder<UBlueprint>(*bpPath);

		if (!tex.Succeeded())
		{
			UE_LOG(LogTemp, Warning, TEXT("Could not find texture for %s"), *building);
			continue;
		}

		if (!bp.Succeeded())
		{
			UE_LOG(LogTemp, Warning, TEXT("Could not find model blueprint for %s"), *building);
			continue;
		}

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