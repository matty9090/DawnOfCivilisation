#include "GameManager.h"
#include "ConstructorHelpers.h"
#include "Engine/Texture.h"
#include "Engine/Blueprint.h"

UGameManager::UGameManager() : EnergyConsumption(0.0)
{
	FString building = "Building0";

	auto tex = ConstructorHelpers::FObjectFinder<UTexture>(TEXT("/Game/Textures/Icons/Ico_Building0"));
	auto bp  = ConstructorHelpers::FObjectFinder<UBlueprint>(TEXT("/Game/Models/Buildings/Building0/Building0_BP"));

	if (!tex.Succeeded())
		UE_LOG(LogTemp, Warning, TEXT("Could not find texture for %s"), *building);

	if (!bp.Succeeded())
		UE_LOG(LogTemp, Warning, TEXT("Could not find model blueprint for %s"), *building);

	FSlateBrush brush;
	brush.ImageSize = FVector2D(128.0f, 128.0f);
	brush.SetResourceObject(tex.Object);

	FBuildingDesc item;
	item.Name = "Building 0";
	item.Icon = brush;
	item.Unlocked = true;

	if (bp.Object != NULL)
		item.Building = bp.Object->GeneratedClass;

	Buildings.Add(item);
}

void UGameManager::Tick(float dt)
{
	
}