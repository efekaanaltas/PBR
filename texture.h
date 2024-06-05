#pragma once

#include "types.h"
#include "stb_image.h"
#include "glad/glad.h"

#include <string>
#include <vector>

struct Texture
{
    u32 id;
    std::string type;
    std::string path;
};

u32 LoadTexture(const char* path, bool gammaCorrection);
u32 LoadCubemap(std::vector<const char*> faces);