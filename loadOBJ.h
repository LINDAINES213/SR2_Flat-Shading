#pragma once
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <fstream>
#include <sstream>
#include <array>
#include <vector>
#include <algorithm>
#include "face.h"


bool loadOBJ(const std::string& path, std::vector<glm::vec3>& out_vertices, std::vector<glm::vec3>& out_normals, std::vector<Face>& out_faces) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << path << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string type;
        iss >> type;

        if (type == "v") {
            float x, y, z;
            iss >> x >> y >> z;
            out_vertices.emplace_back(x, y, z);
        } else if (type == "f") {
            Face face;
            for (int i = 0; i < 3; ++i)
            {
                std::string faceData;
                iss >> faceData;

                std::replace(faceData.begin(), faceData.end(), '/', ' ');

                std::istringstream faceDataIss(faceData);
                int temp; // for discarding texture indices
                faceDataIss >> face.vertexIndices[i] >> temp >> face.normalIndices[i];

                // obj indices are 1-based, so convert to 0-based
                face.vertexIndices[i]--;
                face.normalIndices[i]--;
            }
            out_faces.push_back(face);

        } else if (type == "vn") {
            glm::vec3 normal;
            iss >> normal.x >> normal.y >> normal.z;
            out_normals.push_back(normal);
        }
    }

    file.close();
    return true;
}
