#include <SDL2/SDL.h>
#include <iostream>
#include <glm/glm.hpp>
#include <vector>
#include <array>
#include <fstream>
#include <sstream>
#include <string>
#include <ext/matrix_transform.hpp>

#include "face.h"
#include "color.h"
#include "fragment.h"
#include "line.h"
#include "loadOBJ.h"
#include "triangle.h"
#include "uniform.h"
#include "vertex.h"
#include "shaders.h"

const int WINDOW_WIDTH = 1150;
const int WINDOW_HEIGHT = 600;
float x = 3.14f / 3.0f;

Color clearColor = {0, 0, 0, 255};
Color currentColor = {255, 255, 255, 255};
Color color_a(255, 0, 0, 255); // Red color
Color color_b(0, 255, 255, 255); // Green color
Color color_c(0, 24, 255, 255); // Blue color

SDL_Window* window;
Uniform uniform;

std::array<double, WINDOW_WIDTH * WINDOW_HEIGHT> zBuffer;

glm::vec3 L = glm::vec3(0, 0, 200.0f); // Coloque la luz puntual en el punto deseado


struct Camera {
    glm::vec3 cameraPosition;
    glm::vec3 targetPosition;
    glm::vec3 upVector;
};

void clear(SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderClear(renderer);
}

Color interpolateColor(const glm::vec3& barycentricCoord, const Color& color_a, const Color& color_b, const Color& color_c) {
    float u = barycentricCoord.x;
    float v = barycentricCoord.y;
    float w = barycentricCoord.z;

    // Realiza una interpolación lineal para cada componente del color
    uint8_t r = static_cast<uint8_t>(u * color_a.r + v * color_b.r + w * color_c.r);
    uint8_t g = static_cast<uint8_t>(u * color_a.g + v * color_b.g + w * color_c.g);
    uint8_t b = static_cast<uint8_t>(u * color_a.b + v * color_b.b + w * color_c.b);
    uint8_t a = static_cast<uint8_t>(u * color_a.a + v * color_b.a + w * color_c.a);

    return Color(r, g, b, a);
}

bool isBarycentricCoord(const glm::vec3& barycentricCoord) {
    return barycentricCoord.x >= 0 && barycentricCoord.y >= 0 && barycentricCoord.z >= 0 &&
           barycentricCoord.x <= 1 && barycentricCoord.y <= 1 && barycentricCoord.z <= 1 &&
           glm::abs(1 - (barycentricCoord.x + barycentricCoord.y + barycentricCoord.z)) < 0.00001f;
}

glm::vec3 calculateBarycentricCoord(const glm::vec2& A, const glm::vec2& B, const glm::vec2& C, const glm::vec2& P) {
    float denominator = (B.y - C.y) * (A.x - C.x) + (C.x - B.x) * (A.y - C.y);
    float u = ((B.y - C.y) * (P.x - C.x) + (C.x - B.x) * (P.y - C.y)) / denominator;
    float v = ((C.y - A.y) * (P.x - C.x) + (A.x - C.x) * (P.y - C.y)) / denominator;
    float w = 1 - u - v;
    return glm::vec3(u, v, w);
}

std::vector<Fragment> triangle(const Vertex& a, const Vertex& b, const Vertex& c) {
    std::vector<Fragment> fragments;

    // Calculate the bounding box of the triangle
    int minX = static_cast<int>(std::min({a.position.x, b.position.x, c.position.x}));
    int minY = static_cast<int>(std::min({a.position.y, b.position.y, c.position.y}));
    int maxX = static_cast<int>(std::max({a.position.x, b.position.x, c.position.x}));
    int maxY = static_cast<int>(std::max({a.position.y, b.position.y, c.position.y}));

    // Iterate over each point in the bounding box
    for (int y = minY; y <= maxY; ++y) {
        for (int x = minX; x <= maxX; ++x) {
            glm::vec2 pixelPosition(static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f); // Central point of the pixel
            glm::vec3 barycentricCoord = calculateBarycentricCoord(a.position, b.position, c.position, pixelPosition);

            if (isBarycentricCoord(barycentricCoord)) {
                Color p {0, 0, 0};
                // Interpolate attributes (color, depth, etc.) using barycentric coordinates
                Color interpolatedColor = interpolateColor(barycentricCoord, p, p, p);

                // Calculate the interpolated Z value using barycentric coordinates
                float interpolatedZ = barycentricCoord.x * a.position.z + barycentricCoord.y * b.position.z + barycentricCoord.z * c.position.z;

                // Create a fragment with the position, interpolated attributes, and Z coordinate
                Fragment fragment;
                fragment.position = glm::ivec2(x, y);
                fragment.color = interpolatedColor;
                fragment.z = interpolatedZ;

                fragments.push_back(fragment);
            }
        }
    }

    return fragments;
}

std::vector<std::vector<glm::vec3>> primitiveAssembly(const std::vector<glm::vec3>& transformedVertices) {
    std::vector<std::vector<glm::vec3>> groupedVertices;

    for (int i = 0; i < transformedVertices.size(); i += 3) {
        std::vector<glm::vec3> triangle;
        triangle.push_back(transformedVertices[i]);
        triangle.push_back(transformedVertices[i+1]);
        triangle.push_back(transformedVertices[i+2]);

        groupedVertices.push_back(triangle);
    }

    return groupedVertices;
}

glm::mat4 createModelMatrix() {
    static float rotationSpeed = 2.5f;  // Ajusta la velocidad de rotación aquí

    glm::mat4 translation = glm::translate(glm::mat4(1), glm::vec3(0.0f, 0.0f, 0.0f));
    glm::mat4 scale = glm::scale(glm::mat4(1), glm::vec3(0.6f, 0.6f, 0.6f));
    glm::mat4 rotation = glm::rotate(glm::mat4(1), glm::radians((x += rotationSpeed)), glm::vec3(0.0f, 1.0f, 0.0f));
    return translation * scale * rotation;
}

glm::mat4 createViewMatrix() {
    return glm::lookAt(
            // En donde se encuentra
            glm::vec3(-15, 3, 10),
            // En donde está viendo
            glm::vec3(0, -1, -5),
            // Hacia arriba para la cámara
            glm::vec3(0, 1, 0)
    );
}

glm::mat4 createProjectionMatrix() {
    float fovInDegrees = 45.0f;
    float aspectRatio = WINDOW_WIDTH / WINDOW_HEIGHT;
    float nearClip = 0.1f;
    float farClip = 100.0f;

    return glm::perspective(glm::radians(fovInDegrees), aspectRatio, nearClip, farClip);
}

glm::mat4 createViewportMatrix() {
    glm::mat4 viewport = glm::mat4(1.0f);
    // Scale
    viewport = glm::scale(viewport, glm::vec3(WINDOW_WIDTH / 3.0f, WINDOW_HEIGHT / 2.0f, 5.0f));
    // Translate
    viewport = glm::translate(viewport, glm::vec3(1.0f, 1.0f, 0.5f));

    return viewport;
}

void render(const std::vector<Vertex>& vertexArray,  const Uniform& uniform) {
    std::vector<Vertex> transformedVertexArray;
    for (const auto& vertex : vertexArray) {
        auto transformedVertex = vertexShader(vertex, uniform);
        transformedVertexArray.push_back(transformedVertex);
    }

    // Clear z-buffer at the beginning of each frame
    std::fill(zBuffer.begin(), zBuffer.end(), std::numeric_limits<double>::max());

    for (size_t i = 0; i < transformedVertexArray.size(); i += 3) {
        const Vertex& a = transformedVertexArray[i];
        const Vertex& b = transformedVertexArray[i + 1];
        const Vertex& c = transformedVertexArray[i + 2];

        glm::vec3 A = a.position;
        glm::vec3 B = b.position;
        glm::vec3 C = c.position;

        glm::vec3 edge1 = B - A;
        glm::vec3 edge2 = C - A;
        glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

        // Bounding box para el triangulo
        int minX = static_cast<int>(std::min({A.x, B.x, C.x}));
        int minY = static_cast<int>(std::min({A.y, B.y, C.y}));
        int maxX = static_cast<int>(std::max({A.x, B.x, C.x}));
        int maxY = static_cast<int>(std::max({A.y, B.y, C.y}));

        // Iterating
        for (int y = minY; y <= maxY; ++y) {
            for (int x = minX; x <= maxX; ++x) {
                if (y>0 && y<WINDOW_HEIGHT && x>0 && x<WINDOW_WIDTH) {
                    glm::vec2 pixelPosition(static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f); // Central point of the pixel
                    glm::vec3 barycentricCoord = calculateBarycentricCoord(A, B, C, pixelPosition);

                    if (isBarycentricCoord(barycentricCoord)) {
                        Color barycentricColor {255, 105, 180};
                        Color interpolatedColor = interpolateColor(barycentricCoord, barycentricColor, barycentricColor, barycentricColor);

                        float depth = barycentricCoord.x * A.z + barycentricCoord.y * B.z + barycentricCoord.z * C.z;

                        // Calculate the interpolated Z value using barycentric coordinates
                        float interpolatedZ = barycentricCoord.x * A.z + barycentricCoord.y * B.z + barycentricCoord.z * C.z;

                        // Calculate the position 'P' of the fragment
                        glm::vec3 P = glm::vec3(x, y, depth);
                        glm::vec3 lightDirection = glm::normalize(L - P);

                        glm::vec3 normal = a.normal * barycentricCoord.x + b.normal * barycentricCoord.y+ c.normal * barycentricCoord.z;

                        // Create a fragment with the position, interpolated attributes, and depth
                        Fragment fragment;
                        fragment.position = glm::ivec2(x, y);

                        fragment.z = depth;

                        // Obtén la dirección de la luz en el espacio de vista
                        glm::vec3 lightDirectionViewSpace = glm::normalize(glm::vec3(uniform.view * glm::vec4(L, 1.0f)));
                        float intensity = glm::dot(normal, lightDirectionViewSpace);


                        if (intensity <= 0) {
                            // Aplicar sombreado plano aquí (e.g., establecer fragment.color a un color oscuro)
                            fragment.color = Color{0, 0, 0}; // Color negro para la sombra
                        } else {
                            // Resto del sombreado normal aquí (usar interpolatedColor y multiplicar por la intensidad)
                            Color finalColor = interpolatedColor * intensity;
                            fragment.color = finalColor;
                        }

                        // Apply fragment shader to calculate final color with shading
                        Color finalColor = interpolatedColor * intensity;
                        fragment.color = finalColor;

                        // Apply the intensity to the fragment color
                        fragment.intensity = intensity;

                        int index = y * WINDOW_WIDTH + x;
                        if (depth < zBuffer[index]) {
                            // Apply fragment shader to calculate final color
                            Fragment fragmentShaderf = fragmentShader(fragment);
                            Color finalColor = fragmentShaderf.color;


                            // Draw the fragment using SDL_SetRenderDrawColor and SDL_RenderDrawPoint
                            SDL_SetRenderDrawColor(renderer, finalColor.r, finalColor.g, finalColor.b, finalColor.a);
                            SDL_RenderDrawPoint(renderer, x, WINDOW_HEIGHT-y);

                            // Update the z-buffer value for this pixel
                            zBuffer[index] = depth;
                        }
                    }
                }
            }
        }
    }
}


int main(int argc, char* args[]) {
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("Blender's Model", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, 0);

    SDL_Event event;
    bool quit = false;

    uniform.view = createViewMatrix();
    Fragment fragment;

    srand(time(nullptr));

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normal;
    std::vector<Face> faces;

    if (!loadOBJ("../nave_final.obj", vertices, normal, faces)) {
        std::cerr << "Error loading OBJ file." << std::endl;
        return 1;
    }

    std::vector<Vertex> vertexArray = setupVertexArray(vertices, normal, faces);

    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
        }

        uniform.model = createModelMatrix();
        uniform.projection = createProjectionMatrix();
        uniform.viewport = createViewportMatrix();

        SDL_SetRenderDrawColor(renderer, clearColor.r, clearColor.g, clearColor.b, clearColor.a);
        SDL_RenderClear(renderer);

        // Llamada a la función render con la matriz de vértices transformados
        render(vertexArray, uniform);



        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}