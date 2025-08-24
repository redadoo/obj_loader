#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <unordered_map>

#include "../Matrix/src/Maft.hpp"

using namespace Maft;

namespace obj_loader
{
	struct Material
	{
		std::string name;

		Vector3f Ka;
		Vector3f Kd;
		Vector3f Ks;

		float weighted;
		float transparent;
	};

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
		std::vector<Vector3f> vertices;
		std::vector<Vector3f> normals;
		std::vector<Vector2f> texcoords;
	};

	struct Mesh
	{
		Attrib attrib;
		std::vector<Shape> shapes;
		std::unordered_map<std::string, Material> materials;
	};

	Material load_mtl(const std::string& path)
	{
		std::ifstream ifs(path);
		if (!ifs.is_open())
			throw std::runtime_error("Could not open file: " + path);

		std::string line;
		Material current;

		while (std::getline(ifs, line))
		{
			if (line.empty() || line[0] == '#')
				continue;

			std::istringstream iss(line);
			std::string type;
			iss >> type;

			if (type == "newmtl")
			{
				current = Material{};
				iss >> current.name;
			}
			else if (type == "Ka")
				iss >> current.Ka.x >> current.Ka.y >> current.Ka.z;
			else if (type == "Kd")
				iss >> current.Kd.x >> current.Kd.y >> current.Kd.z;
			else if (type == "Ks")
				iss >> current.Ks.x >> current.Ks.y >> current.Ks.z;
			else if (type == "d")
				iss >> current.transparent;
			else if (type == "Ns") 
				iss >> current.weighted;
		}

		return current;
	}


	Mesh parse(const std::string& path)
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
				std::cout << "v " << mesh.attrib.vertices.back() << "\n";
			}
			else if (type == "vt")
			{
				float x, y;
				iss >> x >> y;
				mesh.attrib.texcoords.emplace_back(x, y);
				std::cout << "vt " << mesh.attrib.texcoords.back() << "\n";
			}
			else if (type == "vn")
			{
				float x, y, z;
				iss >> x >> y >> z;
				mesh.attrib.normals.emplace_back(x, y, z);
				std::cout << "vn " << mesh.attrib.normals.back() << "\n";
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

				std::cout << "f ";
				for (auto &fv : face) 
				{
					std::cout << "(" << fv.vertex_index 
							<< "," << fv.textcoord_index 
							<< "," << fv.normal_index << ") ";
				}
				std::cout << "\n";
			}
			else if(type == "mtllib")
			{
				std::string mtlFile;
				iss >> mtlFile;

				std::filesystem::path objPath = path;
				std::filesystem::path mtlPath = objPath.parent_path() / mtlFile;

				auto loaded = load_mtl(mtlPath.string());
				mesh.materials[loaded.name] = loaded;
				std::cout << "material name " << loaded.name << "\n";
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
