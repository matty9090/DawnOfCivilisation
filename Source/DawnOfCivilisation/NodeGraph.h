#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GraphNode.h"
#include "NodeGraph.generated.h"

USTRUCT(BlueprintType)
struct FNodeGraphSettings
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	int Cost;

	UPROPERTY(BlueprintReadWrite)
	bool Less;
};

UCLASS()
class DAWNOFCIVILISATION_API UNodeGraph : public UObject
{
	GENERATED_BODY()
	
	public:
		UNodeGraph() {}

		void SetAttributes(UWorld* world, float radius, float height, TMap<FString, float> attrs);
		void Generate(TArray<FVector> vertices, TArray<FVector> normals, TArray<int32> indices, TMap<float, FNodeGraphSettings> costSettings);
		int  Heuristic(int start, int end);

		UFUNCTION(BlueprintCallable)
		void GetClosestVertices(TArray<int>& indices, TArray<FVector>& vertices, FVector pos, float distance);

		UFUNCTION(BlueprintCallable)
		bool Pathfind(int start, int end, TArray<UGraphNode*>& path, TArray<UGraphNode*>& closedList);

		UFUNCTION(BlueprintCallable)
		UGraphNode* GetNodeByIndex(int index) { return index < Nodes.Num() ? Nodes[index] : NULL; }

	private:
		UPROPERTY()
		TArray<UGraphNode*> Nodes;

		UPROPERTY()
		TArray<FVector> Vertices;

		UPROPERTY()
		UWorld* World;

		UPROPERTY()
		float Radius;

		UPROPERTY()
		float Height;

		UPROPERTY()
		TMap<FString, float> Attributes;

		UPROPERTY()
		TArray<AActor*> Obstacles;

		bool IsObstacleInRadius(FVector pos, float threshold = 0.0f);
};
