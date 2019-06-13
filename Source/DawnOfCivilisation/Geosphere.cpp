// Fill out your copyright notice in the Description page of Project Settings.


#include "Geosphere.h"
#include "SimplexNoiseBPLibrary.h"

#include <map>
#include <vector>
#include <cassert>
#include <algorithm>

#define PI 3.1415926535
#define TWOPI (PI * 2)

AGeosphere::AGeosphere()
	: EditorDivisions(3),
	  PlayDivisions(6),
	  Radius(3000),
	  NoiseScale(3.0f),
	  NoiseHeight(50.0f),
	  Persistence(0.34f),
	  Seed(0),
	  OceanDepth(1.0f),
	  Collidable(true),
	  GenerateHeights(true),
	  ReverseCulling(false)
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;

	Generate(Radius, EditorDivisions);
}

void AGeosphere::GetClosestVertices(TArray<int>& indices, TArray<FVector>& vertices, FVector pos, float distance)
{
	float d2 = distance * distance;

	for(int i = 0; i < Vertices.Num(); ++i)
	{
		if (FVector::DistSquared(Vertices[i], pos) < d2)
			vertices.Add(Vertices[i]), indices.Add(i);
	}
}

void AGeosphere::GenerateMeshSection()
{
	Mesh->CreateMeshSection_LinearColor(0, Vertices, Indices, Normals, UV, VertexColors, Tangents, Collidable);
}

void AGeosphere::Generate(float radius, size_t tessellation)
{
	USimplexNoiseBPLibrary::setNoiseSeed(Seed);

	std::vector<VertexPositionNormalTexture> vertices;
	std::vector<int32> indices;

	typedef std::pair<int32, int32> UndirectedEdge;

	auto makeUndirectedEdge = [](int32 a, int32 b)
	{
		return std::make_pair(std::max(a, b), std::min(a, b));
	};

	typedef std::map<UndirectedEdge, int32> EdgeSubdivisionMap;

	static const FVector Octahedronvertices[] =
	{
		// when looking down the negative z-axis (into the screen)
		FVector(0,  1,  0), // 0 top
		FVector(0,  0, -1), // 1 front
		FVector(1,  0,  0), // 2 right
		FVector(0,  0,  1), // 3 back
		FVector(-1,  0,  0), // 4 left
		FVector(0, -1,  0), // 5 bottom
	};

	static const int32 Octahedronindices[] =
	{
		0, 1, 2, // top front-right face
		0, 2, 3, // top back-right face
		0, 3, 4, // top back-left face
		0, 4, 1, // top front-left face
		5, 1, 4, // bottom front-left face
		5, 4, 3, // bottom back-left face
		5, 3, 2, // bottom back-right face
		5, 2, 1, // bottom front-right face
	};

	// Start with an octahedron; copy the data into the vertex/index collection.

	std::vector<FVector> vertexPositions(std::begin(Octahedronvertices), std::end(Octahedronvertices));

	indices.insert(indices.begin(), std::begin(Octahedronindices), std::end(Octahedronindices));

	const int32 northPoleIndex = 0;
	const int32 southPoleIndex = 5;

	for (size_t iSubdivision = 0; iSubdivision < tessellation; ++iSubdivision)
	{
		assert(indices.size() % 3 == 0); // sanity

		EdgeSubdivisionMap subdividedEdges;

		std::vector<int32> newindices;

		const size_t triangleCount = indices.size() / 3;
		for (size_t iTriangle = 0; iTriangle < triangleCount; ++iTriangle)
		{
			// For each edge on this triangle, create a new vertex in the middle of that edge.
			// The winding order of the triangles we output are the same as the winding order of the inputs.

			// indices of the vertices making up this triangle
			int32 iv0 = indices[iTriangle * 3 + 0];
			int32 iv1 = indices[iTriangle * 3 + 1];
			int32 iv2 = indices[iTriangle * 3 + 2];

			// Get the new vertices
			FVector v01; // vertex on the midpoint of v0 and v1
			FVector v12; // ditto v1 and v2
			FVector v20; // ditto v2 and v0
			int32 iv01; // index of v01
			int32 iv12; // index of v12
			int32 iv20; // index of v20

			// Function that, when given the index of two vertices, creates a new vertex at the midpoint of those vertices.
			auto divideEdge = [&](int32 i0, int32 i1, FVector & outVertex, int32 & outIndex)
			{
				const UndirectedEdge edge = makeUndirectedEdge(i0, i1);

				auto it = subdividedEdges.find(edge);
				if (it != subdividedEdges.end())
				{
					outIndex = it->second; // the index of this vertex
					outVertex = vertexPositions[outIndex]; // and the vertex itself
				}
				else
				{
					outVertex = (vertexPositions[i0] + vertexPositions[i1]) * 0.5f;
					
					outIndex = static_cast<int32>(vertexPositions.size());
					vertexPositions.push_back(outVertex);

					auto entry = std::make_pair(edge, outIndex);
					subdividedEdges.insert(entry);
				}
			};

			divideEdge(iv0, iv1, v01, iv01);
			divideEdge(iv1, iv2, v12, iv12);
			divideEdge(iv0, iv2, v20, iv20);

			const int32 indicesToAdd[] =
			{
				 iv0, iv01, iv20, // a
				iv20, iv12,  iv2, // b
				iv20, iv01, iv12, // c
				iv01,  iv1, iv12, // d
			};
			newindices.insert(newindices.end(), std::begin(indicesToAdd), std::end(indicesToAdd));
		}

		indices = std::move(newindices);
	}

	vertices.reserve(vertexPositions.size());

	for (auto it = vertexPositions.begin(); it != vertexPositions.end(); ++it)
	{
		auto vertexValue = *it;

		auto normal = vertexValue;
		normal.Normalize();

		auto pos = normal * radius;

		FVector normalFloat3 = normal;

		float longitude = atan2(normalFloat3.X, -normalFloat3.Z);
		float latitude = acos(normalFloat3.Y);

		float u = longitude / TWOPI + 0.5f;
		float v = latitude / PI;

		auto texcoord = FVector2D(1.0f - u, v);
		vertices.push_back(VertexPositionNormalTexture(pos, normal, texcoord));
	}

	size_t preFixupVertexCount = vertices.size();

	for (size_t i = 0; i < preFixupVertexCount; ++i)
	{
		bool isOnPrimeMeridian = FVector2D::ZeroVector.Equals(FVector2D(vertices[i].position.X, vertices[i].uv.X));

		if (isOnPrimeMeridian)
		{
			size_t newIndex = vertices.size();

			VertexPositionNormalTexture v = vertices[i];
			v.uv.X = 1.0f;
			vertices.push_back(v);

			for (size_t j = 0; j < indices.size(); j += 3)
			{
				int32* triIndex0 = &indices[j + 0];
				int32* triIndex1 = &indices[j + 1];
				int32* triIndex2 = &indices[j + 2];

				if (*triIndex0 == i)
				{

				}
				else if (*triIndex1 == i)
				{
					std::swap(triIndex0, triIndex1);
				}
				else if (*triIndex2 == i)
				{
					std::swap(triIndex0, triIndex2);
				}
				else
				{
					continue;
				}

				assert(*triIndex0 == i);
				assert(*triIndex1 != i && *triIndex2 != i);

				const VertexPositionNormalTexture & v0 = vertices[*triIndex0];
				const VertexPositionNormalTexture & v1 = vertices[*triIndex1];
				const VertexPositionNormalTexture & v2 = vertices[*triIndex2];

				if (abs(v0.uv.X - v1.uv.X) > 0.5f ||
					abs(v0.uv.X - v2.uv.X) > 0.5f)
				{
					*triIndex0 = static_cast<int32>(newIndex);
				}
			}
		}
	}

	auto fixPole = [&](size_t poleIndex)
	{
		auto poleVertex = vertices[poleIndex];
		bool overwrittenPoleVertex = false;

		for (size_t i = 0; i < indices.size(); i += 3)
		{
			int32* pPoleIndex;
			int32* pOtherIndex0;
			int32* pOtherIndex1;
			if (indices[i + 0] == poleIndex)
			{
				pPoleIndex = &indices[i + 0];
				pOtherIndex0 = &indices[i + 1];
				pOtherIndex1 = &indices[i + 2];
			}
			else if (indices[i + 1] == poleIndex)
			{
				pPoleIndex = &indices[i + 1];
				pOtherIndex0 = &indices[i + 2];
				pOtherIndex1 = &indices[i + 0];
			}
			else if (indices[i + 2] == poleIndex)
			{
				pPoleIndex = &indices[i + 2];
				pOtherIndex0 = &indices[i + 0];
				pOtherIndex1 = &indices[i + 1];
			}
			else
			{
				continue;
			}

			const auto& otherVertex0 = vertices[*pOtherIndex0];
			const auto& otherVertex1 = vertices[*pOtherIndex1];

			VertexPositionNormalTexture newPoleVertex = poleVertex;
			newPoleVertex.uv.X = (otherVertex0.uv.X + otherVertex1.uv.X) / 2;
			newPoleVertex.uv.Y = poleVertex.uv.Y;

			if (!overwrittenPoleVertex)
			{
				vertices[poleIndex] = newPoleVertex;
				overwrittenPoleVertex = true;
			}
			else
			{
				*pPoleIndex = static_cast<int32>(vertices.size());
				vertices.push_back(newPoleVertex);
			}
		}
	};

	fixPole(northPoleIndex);
	fixPole(southPoleIndex);

	ClearMeshData();

	Normals.Init(FVector(0.0f, 0.0f, 0.0f), vertices.size());
	Tangents.Init(FProcMeshTangent(0.0f, 0.0f, 0.0f), vertices.size());
	VertexColors.Init(FLinearColor(0.0f, 0.0f, 0.0f), vertices.size());

	for(auto v : vertices)
	{
		float height = 0.0f;
		int cost = 1;

		if(GenerateHeights)
		{
			height = GetHeight(v.normal);
			v.position += v.normal * height;
		}

		if (height < 0.0f)
			cost = 6;

		Vertices.Add(v.position);
		UV.Add(v.uv);
		Costs.Add(cost);
	}

	for (auto i : indices)
		Indices.Add(i);

	for (int i = 0; i < Indices.Num(); i += 3)
	{
		FVector p1 = Vertices[Indices[i + 0]];
		FVector p2 = Vertices[Indices[i + 1]];
		FVector p3 = Vertices[Indices[i + 2]];

		FVector2D t1 = UV[Indices[i + 0]];
		FVector2D t2 = UV[Indices[i + 1]];
		FVector2D t3 = UV[Indices[i + 2]];

		FVector vector1 = p2 - p1, vector2 = p3 - p1, tangent;
		FVector2D tuVector = t2 - t1, tvVector = t3 - t1;
		FVector n = FVector::CrossProduct(vector2, vector1);
		n.Normalize();

		float den = 1.0f / (tuVector.X * tvVector.Y - tuVector.Y * tvVector.X);

		tangent.X = (tvVector.Y * vector1.X - tvVector.X * vector2.X) * den;
		tangent.Y = (tvVector.Y * vector1.Y - tvVector.X * vector2.Y) * den;
		tangent.Z = (tvVector.Y * vector1.Z - tvVector.X * vector2.Z) * den;

		Normals[Indices[i + 0]] = n;
		Normals[Indices[i + 1]] = n;
		Normals[Indices[i + 2]] = n;

		Tangents[Indices[i + 0]].TangentX = tangent;
		Tangents[Indices[i + 1]].TangentX = tangent;
		Tangents[Indices[i + 2]].TangentX = tangent;
	}

	if (ReverseCulling)
		ReverseWinding();

	NodeGraph = NewObject<UNodeGraph>();
	NodeGraph->Generate(Vertices, Normals, Indices, Costs);

	GenerateMeshSection();
}

void AGeosphere::ReverseWinding()
{
	for (int i = 0; i < Indices.Num(); i += 3)
		Swap(Indices[i], Indices[i + 2]);

	for (int i = 0; i < UV.Num(); ++i)
		UV[i].X = 1.0f - UV[i].X;
}

void AGeosphere::OnConstruction(const FTransform& Transform)
{
	Generate(Radius, EditorDivisions);
}

void AGeosphere::BeginPlay()
{
	Generate(Radius, PlayDivisions);
	Super::BeginPlay();
}

void AGeosphere::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

float AGeosphere::GetHeight(const FVector& pos)
{
	float height = 0.0f;
	float freq = NoiseScale;
	float amplitude = NoiseHeight;

	for (int i = 0; i < 8; ++i)
	{
		height += USimplexNoiseBPLibrary::SimplexNoise3D(pos.X * freq, pos.Y * freq, pos.Z * freq) * amplitude;
		amplitude *= Persistence;
		freq *= 2;
	}

	return (height < 0.0f) ? height * OceanDepth : height;
}

void AGeosphere::ClearMeshData()
{
	Vertices.Empty();
	Indices.Empty();
	Costs.Empty();
	Normals.Empty();
	VertexColors.Empty();
	Tangents.Empty();
	UV.Empty();
}
