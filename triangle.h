#pragma once
#include "fragment.h"
#include "color.h"
#include "line.h"
#include <glm/glm.hpp>

void triangle(const glm::vec3& A, const glm::vec3& B, const glm::vec3& C, SDL_Renderer* renderer) {
    line(A, B, renderer);
    line(B, C, renderer);
    line(C, A, renderer);
}