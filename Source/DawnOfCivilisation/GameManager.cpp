#include "GameManager.h"
#include "ConstructorHelpers.h"
#include "Engine/Texture.h"
#include "Engine/Blueprint.h"
#include "FileManager.h"
#include "Json.h"

UGameManager::UGameManager() : EnergyConsumption(0.0)
{
	FString jsonFile = FPaths::ProjectConfigDir() + TEXT("Buildings.json"), jsonStr;
	
	if (!FFileHelper::LoadFileToString(jsonStr, *jsonFile))
		throw std::exception("Building JSON file not found");

	TSharedPtr<FJsonObject> json;
	TSharedRef<TJsonReader<TCHAR>> reader = TJsonReaderFactory<TCHAR>::Create(jsonStr);

	if(!FJsonSerializer::Deserialize(reader, json))
		throw std::exception("Error parsing JSON");

	auto buildings = json->GetArrayField("Buildings");

	IFileManager& fileManager = IFileManager::Get();

	for(auto b : buildings)
	{
		auto fields = b->AsObject();

		FString building = fields->GetStringField("Name");
		building.ReplaceInline(L" ", L"");

		FString base = TEXT("/Game/Models/Buildings/");
		FString assetBase = FPaths::ProjectContentDir() + TEXT("Models/Buildings/") + building + "/" + building;
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
		item.Name = fields->GetStringField("Name");
		item.Icon = brush;
		item.Unlocked = fields->GetBoolField("Unlocked");

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