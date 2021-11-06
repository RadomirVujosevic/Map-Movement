// Preprocessor Directives
#ifndef GLITTER
#define GLITTER
#pragma once

// System Headers
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glad/glad.h>

#include <assimp/Importer.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// Reference: https://github.com/nothings/stb/blob/master/stb_image.h#L4
// To use stb_image, add this in *one* C++ source file.
//     #define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// Define Some Constants
float deltaTime = 0.0f;
float lastFrame = 0.0f;

#endif  //~ Glitter Header
