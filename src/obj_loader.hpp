#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>

#include <vector>
#include <unordered_map>
#include <array>

namespace obj_loader
{
#ifdef OBJLOADER_USE_MAFT
	#include "../Matrix/src/Maft.hpp"
#else

	struct vec2
	{
		float x;
		float y;
		vec2(float _x, float _y) : x(_x), y(_y) {}
	};

	struct vec3
	{
		float x;
		float y;
		float z;
		vec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
	};

#endif

	struct FaceVertex 
	{
		int vertex_index{-1};
		int normal_index{-1};
		int textcoord_index{-1};
	};

	struct Shape
	{
		std::vector< std::vector<FaceVertex> > faces;
		std::string material_name;
	};

	struct Attrib
	{
#ifdef OBJLOADER_USE_MAFT
		std::vector<Maft::Vector3f> vertices;
		std::vector<Maft::Vector3f> normals;
		
		std::vector<Maft::Vector2f> texcoords;
#else
		std::vector<vec3> vertices;
		std::vector<vec3> normals;

		std::vector<vec2> texcoords;
#endif
	};

	struct Material
	{
		std::string name;

#ifdef OBJLOADER_USE_MAFT
		Maft::Vector3f Ka;
		Maft::Vector3f Kd;
		Maft::Vector3f Ks;

#else

		float Ka[3];
		float Kd[3];
		float Ks[3];
#endif
		float opacity; 
		float transparent;
		float weighted;
	};

	struct Mesh
	{
		Attrib attrib;
		std::vector<Shape> shapes;
		std::unordered_map<std::string, Material> materials;
	};

	std::unordered_map<std::string, Material> parse_mtl_file(const std::string& path)
	{
		std::ifstream ifs(path);
		if (!ifs.is_open())
			throw std::runtime_error("Could not open file at path: " + path);

		std::unordered_map<std::string, Material> materials;
		Material current;
		std::string line;

		while (std::getline(ifs, line))
		{
			if (line.empty() || line[0] == '#')
				continue;

			std::istringstream iss(line);
			std::string type;
			iss >> type;

			if (type == "newmtl")
			{
				if (!current.name.empty())
					materials[current.name] = current;

				current = Material{};
				iss >> current.name;
			}
#ifdef OBJLOADER_USE_MAFT
			else if (type == "Ka")
				iss >> current.Ka.x >> current.Ka.y >> current.Ka.z;
			else if (type == "Kd")
				iss >> current.Kd.x >> current.Kd.y >> current.Kd.z;
			else if (type == "Ks")
				iss >> current.Ks.x >> current.Ks.y >> current.Ks.z;
#else
			else if (type == "Ka")
				iss >> current.Ka[0] >> current.Ka[1] >> current.Ka[2];
			else if (type == "Kd")
				iss >> current.Kd[0] >> current.Kd[1] >> current.Kd[2];
			else if (type == "Ks")
				iss >> current.Ks[0] >> current.Ks[1] >> current.Ks[2];
#endif
			else if (type == "d")
				iss >> current.opacity;
			else if (type == "Ns")
				iss >> current.weighted;
		}

		if (!current.name.empty())
			materials[current.name] = current;

		return materials;
	}

	Mesh parse_obj_file(const std::string& path)
	{
		std::ifstream ifstream(path);
		if (!ifstream.is_open())
			throw std::runtime_error("Could not open file: " + path);

		Mesh mesh;
		std::string line;

		Shape currentShape;

		while (std::getline(ifstream, line))
		{
			if (line.empty() || line[0] == '#' || line[0] == 's') 
				continue;

			std::istringstream iss(line);
			std::string type;
			iss >> type;

			if (type == "v") 
			{
				float x, y, z;
				iss >> x >> y >> z;
				mesh.attrib.vertices.emplace_back(x, y, z);
			}
			else if (type == "vt")
			{
				float x, y;
				iss >> x >> y;
				mesh.attrib.texcoords.emplace_back(x, y);
			}
			else if (type == "vn")
			{
				float x, y, z;
				iss >> x >> y >> z;
				mesh.attrib.normals.emplace_back(x, y, z);
			}
			else if (type  == "f") 
			{
				std::vector<FaceVertex> face;
				std::string token;

				while (iss >> token) 
				{
					FaceVertex fv;

					int v = 0, vt = 0, vn = 0;
					char slash;

					std::istringstream tokenStream(token);

					if (!(tokenStream >> v)) continue;
					fv.vertex_index = (v > 0) ? v - 1 : mesh.attrib.vertices.size() + v;

					if (tokenStream.peek() == '/') 
					{
						tokenStream.get(slash);

						if (tokenStream.peek() != '/') 
						{ 
							if (tokenStream >> vt)
								fv.textcoord_index = (vt > 0) ? vt - 1 : mesh.attrib.texcoords.size() + vt;
						}

						if (tokenStream.peek() == '/') 
						{
							tokenStream.get(slash);
							if (tokenStream >> vn)
								fv.normal_index = (vn > 0) ? vn - 1 : mesh.attrib.normals.size() + vn;
						}
					}

					face.push_back(fv);
				}
				currentShape.faces.push_back(face);
			}
			else if(type == "mtllib")
			{
				std::string mtlFile;
				iss >> mtlFile;

				std::filesystem::path objPath = path;
				std::filesystem::path mtlPath = objPath.parent_path() / mtlFile;

				auto loaded = parse_mtl_file(mtlPath.string());
				mesh.materials.insert(loaded.begin(), loaded.end());
			}
			else if (type == "usemtl")
			{
				std::string matName;
				iss >> matName;
				currentShape.material_name = matName;
			}
		}

		if (!currentShape.faces.empty())
			mesh.shapes.push_back(currentShape);

		return mesh;
	}

}