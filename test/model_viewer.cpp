#include "raylib.h"
#include "obj_loader.hpp"
#include <algorithm>

Color toColor(const Vector3f& v) {
    return {
        (unsigned char)(v.x * 255),
        (unsigned char)(v.y * 255),
        (unsigned char)(v.z * 255),
        255
    };
}

void test() 
{
    InitWindow(800, 600, "OBJ Loader Test");

    Camera3D camera = {
        { 3.0f, 3.0f, 3.0f },
        { 0.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f },
        45.0f,
        CAMERA_PERSPECTIVE
    };

    obj_loader::Mesh parsed = obj_loader::parse("obj/teapot2.obj");

    SetTargetFPS(60);

    while (!WindowShouldClose()) 
    {
        UpdateCamera(&camera, CAMERA_ORBITAL);

        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode3D(camera);
            DrawGrid(10, 1.0f);

            for (auto& shape : parsed.shapes) 
            {
                Color col = RED;
                if (!shape.material_name.empty()) 
                {
                    auto it = parsed.materials.find(shape.material_name);
                    if (it != parsed.materials.end()) 
                        col = toColor(it->second.Kd);
                }

                for (auto& face : shape.faces) 
                {
                    if (face.size() < 3) continue;
                
                    for (size_t i = 1; i + 1 < face.size(); i++) 
                    {
                        Vector3f v0 = parsed.attrib.vertices[face[0].vertex_index];
                        Vector3f v1 = parsed.attrib.vertices[face[i].vertex_index];
                        Vector3f v2 = parsed.attrib.vertices[face[i+1].vertex_index];

                        DrawTriangle3D(
                            { v0.x, v0.y, v0.z },
                            { v1.x, v1.y, v1.z },
                            { v2.x, v2.y, v2.z },
                            col
                        );
                    }
                }
            }
        EndMode3D();

        DrawFPS(10, 10);
        EndDrawing();
    }

    CloseWindow();
}
