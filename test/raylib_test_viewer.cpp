#include "raylib.h"
#include "obj_loader.hpp"

#include <vector>
#include <string>

Mesh convert_mesh_to_raylib(const obj_loader::Mesh& obj)
{
	Mesh mesh = {};

	int triangleCount = 0;
	for (const auto& shape : obj.shapes)
	{
		for (const auto& face : shape.faces)
			triangleCount += (face.size() - 2);
	}

	mesh.triangleCount = triangleCount;
	mesh.vertexCount   = triangleCount * 3;

	mesh.vertices = (float*)MemAlloc(mesh.vertexCount * 3 * sizeof(float));
	mesh.texcoords = (float*)MemAlloc(mesh.vertexCount * 2 * sizeof(float));
	mesh.normals   = (float*)MemAlloc(mesh.vertexCount * 3 * sizeof(float));

	int vIndex = 0, tIndex = 0, nIndex = 0;

	for (const auto& shape : obj.shapes)
	{
		for (const auto& face : shape.faces)
		{
			for (size_t i = 1; i + 1 < face.size(); i++)
			{
				const obj_loader::FaceVertex fv[3] = {
					face[0], face[i], face[i+1]
				};

				for (int j = 0; j < 3; j++)
				{
					auto v = obj.attrib.vertices[fv[j].vertex_index];
					mesh.vertices[vIndex++] = v.x;
					mesh.vertices[vIndex++] = v.y;
					mesh.vertices[vIndex++] = v.z;

					if (fv[j].textcoord_index >= 0)
					{
						auto uv = obj.attrib.texcoords[fv[j].textcoord_index];
						mesh.texcoords[tIndex++] = uv.x;
						mesh.texcoords[tIndex++] = 1.0f - uv.y;
					}
					else
					{
						mesh.texcoords[tIndex++] = 0.0f;
						mesh.texcoords[tIndex++] = 0.0f;
					}

					if (fv[j].normal_index >= 0)
					{
						auto n = obj.attrib.normals[fv[j].normal_index];
						mesh.normals[nIndex++] = n.x;
						mesh.normals[nIndex++] = n.y;
						mesh.normals[nIndex++] = n.z;
					}
					else
					{
						mesh.normals[nIndex++] = 0.0f;
						mesh.normals[nIndex++] = 1.0f;
						mesh.normals[nIndex++] = 0.0f;
					}
				}
			}
		}
	}

	UploadMesh(&mesh, false);
	return mesh;
}

void test_viewer(bool useMaterial = false)
{
	InitWindow(800, 600, "OBJ Loader Test");

	Camera3D camera = {
		{ 3.0f, 3.0f, 3.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f },
		45.0f,
		CAMERA_PERSPECTIVE
	};

	SetTargetFPS(60);

	obj_loader::Mesh obj = obj_loader::parse_obj_file("obj/42.obj");
	Mesh rmesh = convert_mesh_to_raylib(obj);

	Model model = LoadModelFromMesh(rmesh);

	if (useMaterial)
	{
	const auto& firstMatPair = *obj.materials.begin();
	const obj_loader::Material& mat = firstMatPair.second;

#ifdef OBJLOADER_USE_MAFT
	model.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = Color{
		(unsigned char)(mat.Kd.x * 255.0f),
		(unsigned char)(mat.Kd.y * 255.0f),
		(unsigned char)(mat.Kd.z * 255.0f),
		255
	};
#else
	model.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = Color{
		(unsigned char)(mat.Kd[0] * 255.0f),
		(unsigned char)(mat.Kd[1] * 255.0f),
		(unsigned char)(mat.Kd[2] * 255.0f),
		255
	};
#endif
	}

	while (!WindowShouldClose()) 
	{
		UpdateCamera(&camera, CAMERA_ORBITAL);

		BeginDrawing();
		ClearBackground(RAYWHITE);

		BeginMode3D(camera);
			DrawGrid(10, 1.0f);
			DrawModel(model, {0,0,0}, 1.0f, WHITE);
		EndMode3D();

		DrawFPS(10, 10);
		EndDrawing();
	}

	UnloadModel(model);
	CloseWindow();
}
