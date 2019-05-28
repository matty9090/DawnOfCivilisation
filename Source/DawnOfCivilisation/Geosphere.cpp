// Fill out your copyright notice in the Description page of Project Settings.


#include "Geosphere.h"

#include <map>
#include <vector>
#include <cassert>
#include <algorithm>

#define PI 3.1415926535
#define TWOPI (PI * 2)

AGeosphere::AGeosphere()
{
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;

	Generate(100.0f, 2);
}

void AGeosphere::Generate(float diameter, size_t tessellation)
{
	std::vector<VertexPositionNormalTexture> vertices;
	std::vector<uint16_t> indices;

	typedef std::pair<uint16_t, uint16_t> UndirectedEdge;

	auto makeUndirectedEdge = [](uint16_t a, uint16_t b)
	{
		return std::make_pair(std::max(a, b), std::min(a, b));
	};

	typedef std::map<UndirectedEdge, uint16_t> EdgeSubdivisionMap;

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

	static const uint16_t Octahedronindices[] =
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

	const float radius = diameter / 2.0f;

	// Start with an octahedron; copy the data into the vertex/index collection.

	std::vector<FVector> vertexPositions(std::begin(Octahedronvertices), std::end(Octahedronvertices));

	indices.insert(indices.begin(), std::begin(Octahedronindices), std::end(Octahedronindices));

	const uint16_t northPoleIndex = 0;
	const uint16_t southPoleIndex = 5;

	for (size_t iSubdivision = 0; iSubdivision < tessellation; ++iSubdivision)
	{
		assert(indices.size() % 3 == 0); // sanity

		EdgeSubdivisionMap subdividedEdges;

		std::vector<uint16_t> newindices;

		const size_t triangleCount = indices.size() / 3;
		for (size_t iTriangle = 0; iTriangle < triangleCount; ++iTriangle)
		{
			// For each edge on this triangle, create a new vertex in the middle of that edge.
			// The winding order of the triangles we output are the same as the winding order of the inputs.

			// indices of the vertices making up this triangle
			uint16_t iv0 = indices[iTriangle * 3 + 0];
			uint16_t iv1 = indices[iTriangle * 3 + 1];
			uint16_t iv2 = indices[iTriangle * 3 + 2];

			// Get the new vertices
			FVector v01; // vertex on the midpoint of v0 and v1
			FVector v12; // ditto v1 and v2
			FVector v20; // ditto v2 and v0
			uint16_t iv01; // index of v01
			uint16_t iv12; // index of v12
			uint16_t iv20; // index of v20

			// Function that, when given the index of two vertices, creates a new vertex at the midpoint of those vertices.
			auto divideEdge = [&](uint16_t i0, uint16_t i1, FVector & outVertex, uint16_t & outIndex)
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
					/*XMStoreFloat3(
						&outVertex,
						XMVectorScale(
							XMVectorAdd(XMLoadFloat3(&vertexPositions[i0]), XMLoadFloat3(&vertexPositions[i1])),
							0.5f
						)
					);*/

					outVertex = (vertexPositions[i0] + vertexPositions[i1]) * 0.5f;
					
					outIndex = static_cast<uint16_t>(vertexPositions.size());
					vertexPositions.push_back(outVertex);

					auto entry = std::make_pair(edge, outIndex);
					subdividedEdges.insert(entry);
				}
			};

			divideEdge(iv0, iv1, v01, iv01);
			divideEdge(iv1, iv2, v12, iv12);
			divideEdge(iv0, iv2, v20, iv20);

			const uint16_t indicesToAdd[] =
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

		/*auto normal = XMVector3Normalize(XMLoadFloat3(&vertexValue));
		auto pos = XMVectorScale(normal, radius);*/

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
		/*bool isOnPrimeMeridian = XMVector2NearEqual(
			XMVectorSet(vertices[i].position.x, vertices[i].textureCoordinate.x, 0.0f, 0.0f),
			XMVectorZero(),
			XMVectorSplatEpsilon());*/

		bool isOnPrimeMeridian = FVector2D::ZeroVector.Equals(FVector2D(vertices[i].position.X, vertices[i].uv.X), 1.192092896e-7);

		if (isOnPrimeMeridian)
		{
			size_t newIndex = vertices.size();

			VertexPositionNormalTexture v = vertices[i];
			v.uv.X = 1.0f;
			vertices.push_back(v);

			for (size_t j = 0; j < indices.size(); j += 3)
			{
				uint16_t* triIndex0 = &indices[j + 0];
				uint16_t* triIndex1 = &indices[j + 1];
				uint16_t* triIndex2 = &indices[j + 2];

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
					*triIndex0 = static_cast<uint16_t>(newIndex);
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
			uint16_t* pPoleIndex;
			uint16_t* pOtherIndex0;
			uint16_t* pOtherIndex1;
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
				*pPoleIndex = static_cast<uint16_t>(vertices.size());
				vertices.push_back(newPoleVertex);
			}
		}
	};

	fixPole(northPoleIndex);
	fixPole(southPoleIndex);

	ClearMeshData();

	for(auto v : vertices)
	{
		Vertices.Add(v.position);
		Normals.Add(v.normal);
		UV.Add(v.uv);
	}

	for (auto i : indices)
		Indices.Add(i);

	Tangents.Init(FProcMeshTangent(0.0f, 0.0f, 0.0f), vertices.size());
	VertexColors.Init(FLinearColor(0.0f, 0.0f, 0.0f), vertices.size());

	Mesh->CreateMeshSection_LinearColor(0, Vertices, Indices, Normals, UV, VertexColors, Tangents, true);
}

void AGeosphere::OnConstruction(const FTransform& Transform)
{
	if(Material)
		Mesh->SetMaterial(0, Material);

	Generate(Diameter, Divisions);
}

void AGeosphere::BeginPlay()
{
	Super::BeginPlay();
}

void AGeosphere::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AGeosphere::ClearMeshData()
{
	Vertices.Empty();
	Indices.Empty();
	Normals.Empty();
	VertexColors.Empty();
	Tangents.Empty();
	UV.Empty();
}
