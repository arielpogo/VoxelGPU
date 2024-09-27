#pragma once
#include "src/Globals.h"
#include "Model.h"

#include <iostream>

#define TINYOBJLOADER_IMPLEMENTATION
#include "vendor/tiny_obj_loader.h"

//#define OPTIMIZE_VERTICES

class LoadedModel : public Model 
{
public:

    LoadedModel(const char* path){
        loadModel(path);
    }

private:
    void loadModel(const char* path) {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string err, warn;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path))
            throw std::runtime_error(err+warn);

        if(DEBUG) std::cout << "---Model loading messages for " << path << "---\n" << err+warn << "---End Model loading messages for " << path << "---\n";

#ifdef OPTIMIZE_VERTICES
        std::unordered_map<Vertex, uint32_t> uniqueVertices{};

        for (const auto& shape : shapes) {
            for (const auto& index : shape.mesh.indices) {
                Vertex vertex{};

                vertex.pos = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };

                vertex.texCoord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                };

                vertex.color = {1.0f, 1.0f, 1.0f};

                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }

                indices.push_back(uniqueVertices[vertex]);
            }
        }
#else
        for(const auto& shape : shapes){
            for(const auto& index : shape.mesh.indices){
                Vertex vertex{};

                vertex.pos = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };

                vertex.texCoord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                };

                vertex.color = {1.0f, 1.0f, 1.0f};

                vertices.push_back(vertex);
                indices.push_back(indices.size());
            }
        }
#endif
        if(DEBUG) std::cout << "Final vertex count for model " << path << ": " << vertices.size() << '\n';
    }
};