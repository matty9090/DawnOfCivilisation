#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GraphNode.generated.h"

/**
 * 
 */
UCLASS()
class DAWNOFCIVILISATION_API UGraphNode : public UObject
{
	GENERATED_BODY()
	
	public:
		UPROPERTY(BlueprintReadOnly)
		int Id;

		UPROPERTY(BlueprintReadOnly)
		FVector Position;

		UPROPERTY(BlueprintReadOnly)
		FVector Normal;

		UPROPERTY(BlueprintReadOnly)
		int Cost;

		UPROPERTY(BlueprintReadOnly)
		TSet<UGraphNode*> Children;
};
