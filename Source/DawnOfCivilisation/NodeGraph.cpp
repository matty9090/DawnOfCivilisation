#include "NodeGraph.h"

void UNodeGraph::Generate(TArray<FVector> vertices, TArray<FVector> normals, TArray<int32> indices, TArray<int32> costs)
{
	for (int i = 0; i < vertices.Num(); ++i)
	{
		UGraphNode* node = NewObject<UGraphNode>();
		node->Id = i;
		node->Position = vertices[i];
		node->Normal = normals[i];
		node->Cost = costs[i];
		Nodes.Add(node);
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

bool UNodeGraph::Pathfind(int start, int end, TArray<UGraphNode*>& path)
{
	TSet<int> closed, open = { start };

	TMap<int, int> data;
	TMap<int, int> gScore;
	TMap<int, int> fScore;

	for(int i = 0; i < Nodes.Num(); ++i)
	{
		gScore.Add(i, 99999);
		fScore.Add(i, 99999);
	}

	gScore[start] = 0;
	fScore[start] = Heuristic(start, end);

	while(open.Num() > 0)
	{
		int current, minScore = 99999;

		for (auto n : open)
			if (fScore[n] < minScore)
				minScore = fScore[n], current = n;

		if(current == end)
		{
			int n = end;
			path.Add(Nodes[current]);

			while (data.Find(n) != NULL)
			{
				n = data[n];
				path.Add(Nodes[n]);
			}

			return true;
		}

		open.Remove(current);
		closed.Add(current);

		for(auto node : Nodes[current]->Children)
		{
			if (node->Cost <= 0 || closed.Find(node->Id) != NULL)
				continue;

			int nScore = gScore[current] + Nodes[current]->Cost;

			if (open.Find(node->Id) == NULL)
				open.Add(node->Id);
			else if (nScore >= gScore[node->Id])
				continue;

			data.Add(node->Id, current);
			gScore[node->Id] = nScore;
			fScore[node->Id] = nScore + Heuristic(node->Id, end);
		}
	}

	return false;
}

int UNodeGraph::Heuristic(int start, int end)
{
	auto p1 = Nodes[start]->Position;
	auto p2 = Nodes[end]->Position;

	return abs(p1.X - p2.X) + abs(p1.Y - p2.Y) + abs(p1.Z - p2.Z);
}