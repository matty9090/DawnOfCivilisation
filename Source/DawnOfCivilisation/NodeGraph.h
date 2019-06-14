#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GraphNode.h"
#include "NodeGraph.generated.h"

UCLASS()
class DAWNOFCIVILISATION_API UNodeGraph : public UObject
{
	GENERATED_BODY()
	
	public:
		UNodeGraph() {}

		void Generate(TArray<FVector> vertices, TArray<FVector> normals, TArray<int32> indices, TArray<int32> costs);
		int  Heuristic(int start, int end);
		void GetClosestVertices(TArray<int>& indices, TArray<FVector>& vertices, FVector pos, float distance);

		UFUNCTION(BlueprintCallable)
		bool Pathfind(int start, int end, TArray<UGraphNode*>& path, TArray<UGraphNode*>& closedList);

		UFUNCTION(BlueprintCallable)
		UGraphNode* GetNodeByIndex(int index) { return Nodes[index]; }


	private:
		UPROPERTY()
		TArray<UGraphNode*> Nodes;

		UPROPERTY()
		TArray<FVector> Vertices;
};
