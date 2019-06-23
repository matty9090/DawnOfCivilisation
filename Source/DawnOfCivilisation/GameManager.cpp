#include "GameManager.h"
#include "ConstructorHelpers.h"
#include "Engine/Texture.h"
#include "Engine/Blueprint.h"
#include "FileManager.h"

UGameManager::UGameManager()
	: CurrentPrefix(0),
	  EnergyConsumption(0.0)
{
	LoadBuildings();
	LoadPrefixes();
	LoadResources();
}

TSharedPtr<FJsonObject> UGameManager::GetSettingsJson()
{
	FString jsonFile = FPaths::ProjectConfigDir() + TEXT("GameConfig.json"), jsonStr;

	if (!FFileHelper::LoadFileToString(jsonStr, *jsonFile))
		throw std::exception("Building JSON file not found");

	TSharedPtr<FJsonObject> json;
	TSharedRef<TJsonReader<TCHAR>> reader = TJsonReaderFactory<TCHAR>::Create(jsonStr);

	if (!FJsonSerializer::Deserialize(reader, json))
		throw std::exception("Error parsing JSON");

	return json;
}

void UGameManager::LoadBuildings()
{
	auto json = GetSettingsJson();
	auto buildings = json->GetArrayField("Buildings");

	IFileManager& fileManager = IFileManager::Get();

	for (auto b : buildings)
	{
		auto fields = b->AsObject();

		FString building = fields->GetStringField("Name");
		building.ReplaceInline(L" ", L"");

		FString base = TEXT("/Game/Models/Buildings/");
		FString assetBase = FPaths::ProjectContentDir() + TEXT("Models/Buildings/") + building + "/" + building;
		FString texPath = building + "/" + building + "_Ico";
		FString bpPath = building + "/" + building + "_BP";

		if (!fileManager.FileExists(*(assetBase + "_Ico.uasset")))
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
		item.Unlocked = fields->HasField("Unlocked") ? fields->GetBoolField("Unlocked") : false;
		item.BuildersRequired = fields->GetIntegerField("NumBuilders");
		item.HP = fields->GetIntegerField("HP");

		item.ResourcesRequired[EResourceType::Wood]	   = fields->HasField("ResWood")    ? fields->GetIntegerField("ResWood") : 0;
		item.ResourcesRequired[EResourceType::Metal]   = fields->HasField("ResMetal")   ? fields->GetIntegerField("ResMetal") : 0;
		item.ResourcesRequired[EResourceType::Coal]	   = fields->HasField("ResCoal")    ? fields->GetIntegerField("ResCoal") : 0;
		item.ResourcesRequired[EResourceType::Oil]     = fields->HasField("ResOil")     ? fields->GetIntegerField("ResOil") : 0;
		item.ResourcesRequired[EResourceType::Silicon] = fields->HasField("ResSilicon") ? fields->GetIntegerField("ResSilicon") : 0;

		if (bp.Object != NULL)
			item.Building = bp.Object->GeneratedClass;
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Error loading class for"), *building);
			continue;
		}

		Buildings.Add(item.Name, item);
	}
}

void UGameManager::LoadPrefixes()
{
	auto json = GetSettingsJson();
	auto prefixes = json->GetArrayField("Prefixes");

	for(auto prefix : prefixes)
	{
		auto obj = prefix->AsObject();
		Prefixes.Add(obj->GetNumberField("Power"), obj->GetStringField("Short"));
	}
}

void UGameManager::LoadResources()
{
	Resources.Add(EResourceType::Wood, FResource(1000, true));
	Resources.Add(EResourceType::Metal, FResource());
	Resources.Add(EResourceType::Coal, FResource());
	Resources.Add(EResourceType::Oil, FResource());
	Resources.Add(EResourceType::Silicon, FResource());
}

void UGameManager::LoadMilestones()
{
	auto json = GetSettingsJson();
	auto milestones = json->GetArrayField("Milestones");

	for (auto milestone : milestones)
	{
		auto obj = milestone->AsObject();
		auto type = obj->GetStringField("Event");

		FGameEvent evt;
		
		if (type == "UnlockBuilding")	evt.Type = EEvent::UnlockBuilding;
		if (type == "UnlockTechnology") evt.Type = EEvent::UnlockTechnology;
		if (type == "Achievement")		evt.Type = EEvent::Achievement;

		evt.Name = obj->GetStringField("Name");
		evt.Data = obj->GetStringField("Data");
		evt.EnergyConsumption = obj->GetIntegerField("EnergyRequired");

		Milestones.Add(evt);
	}
}

void UGameManager::Tick(float dt)
{
	for(auto milestone : Milestones)
	{
		if(milestone.EnergyConsumption > 0 && EnergyConsumption >= milestone.EnergyConsumption)
			CompleteMilestone(milestone.Name);
	}
}

void UGameManager::CompleteMilestone(FString name)
{
	for (auto milestone : Milestones)
	{
		if(milestone.Name == name)
		{
			CompletedMilestones.Add(milestone);
			Milestones.Remove(milestone);
			OnMilestoneCompleted(milestone);

			break;
		}
	}
}

void UGameManager::AddEnergyConsumption(float watts)
{
	EnergyConsumption += watts;

	for(auto power : Prefixes)
	{
		if (EnergyConsumption >= pow(10.0, power.Key))
			CurrentPrefix = power.Key;
	}
}

FString UGameManager::GetFormattedEnergyConsumption()
{
	double adj = static_cast<int>(EnergyConsumption / pow(10.0, CurrentPrefix));
	return FString::FromInt(adj) + Prefixes[CurrentPrefix] + 'W';
}

void UGameManager::AddResourceAmount(TMap<EResourceType, int> res)
{
	for (auto r : res)
		Resources[r.Key] += r.Value;
}

void UGameManager::SubtractResourceAmount(TMap<EResourceType, int> res)
{
	for (auto r : res)
		Resources[r.Key] -= r.Value;
}

bool UGameManager::CanPlaceBuilding(FBuildingDesc building)
{
	for(auto res : building.ResourcesRequired)
	{
		if (Resources[res.Key]() < res.Value)
			return false;
	}

	return true;
}

TArray<FBuildingDesc> UGameManager::GetBuildingList()
{
	TArray<FBuildingDesc> list;
	Buildings.GenerateValueArray(list);
	return list;
}