#include "NodeGraph.h"
#include "Geosphere.h"
#include "Kismet/GameplayStatics.h"

#include <set>
#include <map>

void UNodeGraph::SetAttributes(UWorld* world, float radius, float height, TMap<FString, float> attrs)
{
	World = world;
	Radius = radius;
	Height = height;
	Attributes = attrs;
}

void UNodeGraph::Generate(TArray<FVector> vertices, TArray<FVector> normals, TArray<int32> indices, TMap<float, FNodeGraphSettings> costSettings)
{
	Vertices = vertices;
	
	UGameplayStatics::GetAllActorsWithTag(World, "PlanetObstacle", Obstacles);

	for (auto o : Obstacles)
		UE_LOG(LogTemp, Display, TEXT("%s"), *o->GetClass()->GetName());

	for (int i = 0; i < vertices.Num(); ++i)
	{
		UGraphNode* node = NewObject<UGraphNode>();
		node->Id = i;
		node->Position = vertices[i];
		node->Normal = normals[i];
		node->Cost = 1;

		for(auto setting : costSettings)
		{
			float pos = (node->Position.Size() - Radius) / Height;

			if (!IsObstacleInRadius(node->Position, 0.0f))
			{
				if (setting.Value.Less)
					node->Cost = (pos < setting.Key) ? setting.Value.Cost : node->Cost;
				else
					node->Cost = (pos > setting.Key) ? setting.Value.Cost : node->Cost;
			}
			else
				node->Cost = 0;
		}

		Nodes.Add(node);
	}

	for (int i = 0; i < Nodes.Num(); ++i)
	{
		TArray<int32> ind;
		FOccluderVertexArray vert;
		GetClosestVertices(ind, vert, Nodes[i]->Position, 140.0f);

		for (auto index : ind)
			Nodes[i]->Children.Add(Nodes[index]);
	}

	for (int i = 0; i < indices.Num(); i += 3)
	{
		for (int j = 0; j < 3; ++j)
		{
			int i1 = indices[i + ((j + 1) % 3)];
			int i2 = indices[i + ((j + 2) % 3)];

			UGraphNode* node = Nodes[indices[i + j]];
			node->Children.Add(Nodes[i1]);
			node->Children.Add(Nodes[i2]);
		}
	}
}

bool UNodeGraph::Pathfind(int start, int end, TArray<UGraphNode*>& path, TArray<UGraphNode*>& closedList)
{
	std::set<int> closed, open = { start };

	std::map<int, int> data;
	std::map<int, int> gScore;
	std::map<int, int> fScore;

	for(int i = 0; i < Nodes.Num(); ++i)
	{
		gScore[i] = 99999999;
		fScore[i] = 99999999;
	}

	gScore[start] = 0;
	fScore[start] = Heuristic(start, end);

	while(!open.empty())
	{
		int current, minScore = 99999999;

		for (auto n : open)
			if (fScore[n] < minScore)
				minScore = fScore[n], current = n;

		if(current == end)
		{
			int n = end;
			path.Add(Nodes[current]);

			while (data.find(n) != data.end())
			{
				n = data[n];
				path.Add(Nodes[n]);
			}

			return true;
		}

		open.erase(current);
		closed.insert(current);

		for(auto node : Nodes[current]->Children)
		{
			if (node->Cost <= 0 || closed.find(node->Id) != closed.end())
				continue;

			int nScore = gScore[current] + Nodes[current]->Cost;

			if (open.find(node->Id) == open.end())
				open.insert(node->Id);
			else if (nScore >= gScore[node->Id])
				continue;

			data[node->Id] = current;
			gScore[node->Id] = nScore;
			fScore[node->Id] = nScore + Heuristic(node->Id, end);
		}
	}

	for(auto n : closed)
		closedList.Add(Nodes[n]);

	return false;
}

int UNodeGraph::Heuristic(int start, int end)
{
	auto p1 = Nodes[start]->Position;
	auto p2 = Nodes[end]->Position;

	p1.Normalize();
	p2.Normalize();

	return acosf(FVector::DotProduct(p1, p2));
}

void UNodeGraph::GetClosestVertices(TArray<int>& indices, TArray<FVector>& vertices, FVector pos, float distance)
{
	float d2 = distance * distance;

	for (int i = 0; i < Vertices.Num(); ++i)
	{
		if (FVector::DistSquared(Vertices[i], pos) < d2)
			vertices.Add(Vertices[i]), indices.Add(i);
	}
}

bool UNodeGraph::IsObstacleInRadius(FVector pos, float threshold)
{
	for(auto o : Obstacles)
	{
		if(Attributes.Find(o->GetClass()->GetName()) != NULL)
		{
			float d = Attributes[o->GetClass()->GetName()];

			if (FVector::Dist(o->GetActorLocation(), pos) < d + threshold)
				return true;
		}
	}

	return false;
}