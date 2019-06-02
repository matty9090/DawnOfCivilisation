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
		virtual void Tick(float DeltaTime) override;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
		UProceduralMeshComponent* Mesh;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sphere")
		int EditorDivisions;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sphere")
		int PlayDivisions;
		
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sphere")
		float Radius;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
		float NoiseScale;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
		float NoiseHeight;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
		float Persistence;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
		float OceanDepth;

		UFUNCTION(BlueprintCallable)
		void GetVertices(TArray<FVector>& vertices) { vertices = Vertices; }

		UFUNCTION(BlueprintCallable)
		void GetClosestVertices(TArray<int>& indices, TArray<FVector>& vertices, FVector pos, float distance);

		UFUNCTION(BlueprintCallable)
		void SetVertex(int vertex, FVector v) { Vertices[vertex] = v; }

		UFUNCTION(BlueprintCallable)
		FVector GetNormal(int index) { return Normals[index]; }

		UFUNCTION(BlueprintCallable)
		void GenerateMeshSection();

	private:
		struct VertexPositionNormalTexture
		{
			FVector position, normal;
			FVector2D uv;

			VertexPositionNormalTexture(FVector pos, FVector norm, FVector2D tex) : position(pos), normal(norm), uv(tex) {}
		};

		void Generate(float diameter, size_t tessellation);
		void ClearMeshData();
		float GetHeight(const FVector& pos);

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
