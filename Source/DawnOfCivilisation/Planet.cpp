#include "Planet.h"
#include "Geosphere.h"
#include "ColourSpace.h"
#include "Materials/MaterialInstanceDynamic.h"

#include <time.h>

const FBoolRange FBoolRange::MakeRandom = FBoolRange(true, true);
const FBoolRange FBoolRange::MakeEnabled = FBoolRange(true, false);
const FBoolRange FBoolRange::MakeDisabled = FBoolRange(false, false);

APlanet::APlanet()
	: EditorDivisions(3),
	  PlayDivisions(6),
	  Radius(3000.0f),
	  Seed(0)
{
	PrimaryActorTick.bCanEverTick = false;

	FPlanetPreset Rocky;
	Rocky.Name = "Rocky";
	Rocky.HasTerrain = FBoolRange::MakeEnabled;
	Rocky.HasAtmosphere = FBoolRange::MakeEnabled;
	Rocky.HasWater = FBoolRange::MakeEnabled;
	Rocky.AtmosphereColour = FVectorRange(FVector(0.0f, 0.0f, 0.0f), FVector(1.0f, 1.0f, 1.0f));
	
	Rocky.TerrainNoise = {
		FScalarRange(0.5f, 4.0f),
		FScalarRange(10.0f, 70.0f),
		FScalarRange(0.2f, 0.4f),
		FScalarRange(2.0f, 7.0f),
		true
	};

	Rocky.LandFeatures = {
		FVectorRange(FVector(0.88f, 0.31f, 0.10f)),
		FVectorRange(FVector(0.01f, 0.16f, 0.00f)),
		FVectorRange(FVector(0.01f, 0.10f, 0.10f)),
		//FVectorRange(FVector(0.04f, 0.23f, 0.37f)),
		FVectorRange(FVector(0.0f, 100.0f, 60.0f), FVector(360.0f, 100.0f, 60.0f)),
		FScalarRange(0.0f, 20.0f),
		FScalarRange(0.0f, 12.0f)
	};

	Presets.Add(Rocky);
	Preset = 0;
}

void APlanet::OnConstruction(const FTransform& Transform)
{
	Generate();
}

void APlanet::BeginPlay()
{
	Super::BeginPlay();
}

void APlanet::Generate()
{
	Seed = time(0);

	RandomStream = FRandomStream(Seed);
	FPlanetPreset preset = Presets[Preset];

	auto components = GetComponents();

	for (auto c : components)
		c->DestroyComponent();

	if (preset.HasTerrain.GetValue(RandomStream))
	{
		auto component = CreateChildComponent(AGeosphere::StaticClass());
		PlanetComponents.Add(EPlanetComponent::Terrain, component);

		// TODO: Change to interface
		AGeosphere* terrain = static_cast<AGeosphere*>(component->GetChildActor());
		terrain->EditorDivisions = EditorDivisions;
		terrain->PlayDivisions = PlayDivisions;
		terrain->Radius = Radius;
		terrain->NoiseScale = preset.TerrainNoise.NoiseScale.GetValue(RandomStream);
		terrain->NoiseHeight = preset.TerrainNoise.NoiseHeight.GetValue(RandomStream);
		terrain->Persistence = preset.TerrainNoise.Persistence.GetValue(RandomStream);
		terrain->OceanDepth = preset.TerrainNoise.OceanDepth.GetValue(RandomStream);
		terrain->GenerateHeights = preset.TerrainNoise.GenerateHeights;
		terrain->Seed = Seed;
		terrain->Collidable = true;

		if (PlanetMaterials.Contains(EPlanetComponent::Terrain) && PlanetMaterials[EPlanetComponent::Terrain])
		{
			UMaterialInstanceDynamic* mat = terrain->Mesh->CreateDynamicMaterialInstance(0, PlanetMaterials[EPlanetComponent::Terrain]);
			mat->SetScalarParameterValue(FName("Radius"), Radius);
			mat->SetVectorParameterValue(FName("Sand Colour"), preset.LandFeatures.BeachColour.GetValue(RandomStream));
			mat->SetVectorParameterValue(FName("Grass Colour"), preset.LandFeatures.LandColour.GetValue(RandomStream));
			mat->SetVectorParameterValue(FName("Rock Colour"), preset.LandFeatures.MountainColour.GetValue(RandomStream));
			terrain->Mesh->SetMaterial(0, mat);
		}
	}

	if (preset.HasWater.GetValue(RandomStream))
	{
		auto component = CreateChildComponent(AGeosphere::StaticClass());
		PlanetComponents.Add(EPlanetComponent::Water, component);

		AGeosphere* water = static_cast<AGeosphere*>(component->GetChildActor());
		water->EditorDivisions = EditorDivisions;
		water->PlayDivisions = PlayDivisions;
		water->Radius = Radius;
		water->GenerateHeights = false;
		water->Collidable = true;
		
		if (PlanetMaterials.Contains(EPlanetComponent::Water) && PlanetMaterials[EPlanetComponent::Water])
		{			
			auto shallow = preset.LandFeatures.WaterColour.GetValue(RandomStream);

			auto deep = shallow - FVector(0.0f, 0.0f, 10.0f);
			deep = UColourSpace::HSLToRGB(deep);

			auto shore = shallow + FVector(0.0f, 0.0f, 30.0f);
			shore = UColourSpace::HSLToRGB(shore);

			UMaterialInstanceDynamic* mat = water->Mesh->CreateDynamicMaterialInstance(0, PlanetMaterials[EPlanetComponent::Water]);
			mat->SetVectorParameterValue(FName("Shallow Water"), UColourSpace::HSLToRGB(shallow));
			mat->SetVectorParameterValue(FName("Deep Water"), deep);
			mat->SetVectorParameterValue(FName("Shore"), shore);
			water->Mesh->SetMaterial(0, mat);
		}
	}

	if (preset.HasAtmosphere.GetValue(RandomStream))
	{
		auto component = CreateChildComponent(AGeosphere::StaticClass());
		PlanetComponents.Add(EPlanetComponent::Atmosphere, component);

		AGeosphere* atmosphere = static_cast<AGeosphere*>(component->GetChildActor());
		atmosphere->EditorDivisions = EditorDivisions;
		atmosphere->PlayDivisions = PlayDivisions;
		atmosphere->Radius = Radius * 1.08f;
		atmosphere->GenerateHeights = false;
		atmosphere->Collidable = false;
		atmosphere->ReverseCulling = true;

		if (PlanetMaterials.Contains(EPlanetComponent::Atmosphere) && PlanetMaterials[EPlanetComponent::Atmosphere])
		{
			UMaterialInstanceDynamic* mat = atmosphere->Mesh->CreateDynamicMaterialInstance(0, PlanetMaterials[EPlanetComponent::Atmosphere]);
			mat->SetVectorParameterValue(FName("Colour"), preset.AtmosphereColour.GetValue(RandomStream));
			atmosphere->Mesh->SetMaterial(0, mat);
		}
	}
}

void APlanet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

UChildActorComponent* APlanet::CreateChildComponent(UClass* c)
{
	UChildActorComponent* comp = NewObject<UChildActorComponent>(this);
	comp->SetChildActorClass(c);
	comp->CreateChildActor();
	comp->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	return comp;
}

