#include "glm/glm.hpp"
#include "uniform.h"
#include "fragment.h"
#include "color.h"
#include "vertex.h"
#pragma once

void printMatrix(const glm::mat4& myMatrix) {
    // Imprimir los valores de la matriz
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            std::cout << myMatrix[i][j] << " ";
        }
        std::cout << std::endl;
    }
}

// Función para imprimir un vec4
void printVec4(const glm::vec4& vector) {
    std::cout << "(" << vector.x << ", " << vector.y << ", " << vector.z << ", " << vector.w << ")";
}

// Función para imprimir un vec3
void printVec3(const glm::vec3& vector) {
    std::cout << "(" << vector.x << ", " << vector.y << ", " << vector.z << ")";
}


Vertex vertexShader(const Vertex& vertex, const Uniform& uniform) {
    glm::vec4 transformedVertex = uniform.viewport * uniform.projection * uniform.view * uniform.model * glm::vec4(vertex.position, 1.0f);
    glm::vec3 vertexRedux;
    vertexRedux.x = transformedVertex.x / transformedVertex.w;
    vertexRedux.y = transformedVertex.y / transformedVertex.w;
    vertexRedux.z = transformedVertex.z / transformedVertex.w;
    Color fragmentColor(255, 0, 0, 255);
    glm::vec3 normal = glm::normalize(glm::mat3(uniform.model) * vertex.normal);
    Fragment fragment;
    fragment.position = glm::ivec2(transformedVertex.x, transformedVertex.y);
    fragment.color = fragmentColor;
    return Vertex {vertexRedux, normal};
}

Fragment fragmentShader(Fragment& fragment) {
    fragment.color = fragment.color * fragment.intensity; // Red color with full opacity
    return fragment;
}