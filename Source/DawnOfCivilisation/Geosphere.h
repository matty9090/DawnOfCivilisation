// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Geosphere.generated.h"

UCLASS()
class DAWNOFCIVILISATION_API AGeosphere : public AActor
{
	GENERATED_BODY()
	
	public:	
		AGeosphere();

	protected:
		virtual void BeginPlay() override;
		virtual void OnConstruction(const FTransform& Transform) override;

	public:	
		// Called every frame
		virtual void Tick(float DeltaTime) override;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
		UProceduralMeshComponent* Mesh;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sphere")
		int Divisions;
		
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sphere")
		float Diameter;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sphere")
		UMaterialInterface* Material;

	private:
		struct VertexPositionNormalTexture
		{
			FVector position, normal;
			FVector2D uv;

			VertexPositionNormalTexture(FVector pos, FVector norm, FVector2D tex) : position(pos), normal(norm), uv(tex) {}
		};

		void Generate(float diameter, size_t tessellation);
		void ClearMeshData();

		UPROPERTY()
		TArray<FVector> Vertices;

		UPROPERTY()
		TArray<FVector> Normals;

		UPROPERTY()
		TArray<int32> Indices;

		UPROPERTY()
		TArray<FVector2D> UV;

		UPROPERTY()
		TArray<FLinearColor> VertexColors;

		UPROPERTY()
		TArray<FProcMeshTangent> Tangents;
};
