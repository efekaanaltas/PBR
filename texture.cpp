#include "texture.h"

u32 LoadTexture(const char* path, bool gammaCorrection)
{
    u32 id;
    glGenTextures(1, &id);

    int width, height, channelCount;
    unsigned char* data = stbi_load(path, &width, &height, &channelCount, 0);

    if (data)
    {
        GLenum internalFormat = GL_RGBA;
        GLenum dataFormat = GL_RGBA;
        switch (channelCount)
        {
        case 1: internalFormat = GL_RED;  
                dataFormat = GL_RED; 
                break;
        case 3: internalFormat = gammaCorrection ? GL_SRGB : GL_RGB;
                dataFormat = GL_RGB;
                break;
        case 4: internalFormat = gammaCorrection ? GL_SRGB_ALPHA : GL_RGBA;
                dataFormat = GL_RGBA;
                break;
        }

        glBindTexture(GL_TEXTURE_2D, id);

        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        stbi_image_free(data);
    }
    else
    {
        printf("Failed to load image %s.", path);
    }

    return id;
}

u32 LoadCubemap(std::vector<const char*> faces)
{
    u32 id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);

    int width, height, channelCount;
    u8* data;
    for (u32 i = 0; i < faces.size(); i++)
    {
        data = stbi_load(faces[i], &width, &height, &channelCount, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        }
        else
        {
            printf("Cubemap texture failed to load at path: %s", faces[i]);
            stbi_image_free(data);
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return id;
}

