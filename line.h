#pragma once
#include <glm/glm.hpp>
#include <SDL_render.h>
#include "vertex.h"

SDL_Renderer* renderer;

void line(const glm::vec3& start, const glm::vec3& end, SDL_Renderer* renderer) {
    SDL_RenderDrawLine(renderer,
                       static_cast<int>(start.x), static_cast<int>(start.y),
                       static_cast<int>(end.x), static_cast<int>(end.y));
}