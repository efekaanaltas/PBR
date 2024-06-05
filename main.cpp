#include "main.h"
#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include <stdio.h>
#include <map>

#include "model.h"

v3 camPos = v3(0, 0, 3);
v3 camFront = v3(0, 0, -1);
v3 camUp = v3(0, 1, 0);

f32 lastX = 400;
f32 lastY = 300;

f32 yaw = -90;
f32 pitch = 0;

f32 dt = 0.0f;
f32 lastFrameTime = 0.0f;

b32 firstMouse = true;
f32 fov = 90.0f;

void GLFWFramebufferSizeCallback(GLFWwindow* window, s32 width, s32 height)
{
    glViewport(0, 0, width, height);
}

void GLFWScrollCallback(GLFWwindow* window, f64 xOffset, f64 yOffset)
{
    fov -= (f32)yOffset;
    if (fov < 1)    fov = 1;
    if (fov > 120)  fov = 120;
}

void GLFWMouseCallback(GLFWwindow* window, f64 x, f64 y)
{
    if (firstMouse)
    {
        lastX = (f32)x;
        lastY = (f32)y;
        firstMouse = false;
    }

    f32 xOffset = (f32)x - lastX;
    f32 yOffset = lastY - (f32)y;
    lastX = (f32)x;
    lastY = (f32)y;

    f32 sensitivity = 0.1f;
    xOffset *= sensitivity;
    yOffset *= sensitivity;

    yaw += xOffset;
    pitch += yOffset;

#if 1
    if (pitch > 89.0f)
    {
        pitch = 89.0f;
    }
    if (pitch < -89.0f)
    {
        pitch = -89.0f;
    }
#endif

    v3 camDir;
    camDir.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    camDir.y = sin(glm::radians(pitch));
    camDir.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    camFront = glm::normalize(camDir);
}

void ProcessInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    float camSpeed = 2.5f * dt;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        camPos += camSpeed * camFront;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        camPos -= camSpeed * camFront;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        camPos -= glm::normalize(glm::cross(camFront, camUp)) * camSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        camPos += glm::normalize(glm::cross(camFront, camUp)) * camSpeed;
    }
}

const float divisor = 4000.0f;
const float r = 10.0f;

v3 backpackPositions[10] =
{
    {(f32)rand() / divisor - r / 2, (f32)rand() / divisor - r / 2, (f32)rand() / divisor - r / 2},
    {(f32)rand() / divisor - r / 2, (f32)rand() / divisor - r / 2, (f32)rand() / divisor - r / 2},
    {(f32)rand() / divisor - r / 2, (f32)rand() / divisor - r / 2, (f32)rand() / divisor - r / 2},
    {(f32)rand() / divisor - r / 2, (f32)rand() / divisor - r / 2, (f32)rand() / divisor - r / 2},
    {(f32)rand() / divisor - r / 2, (f32)rand() / divisor - r / 2, (f32)rand() / divisor - r / 2},
    {(f32)rand() / divisor - r / 2, (f32)rand() / divisor - r / 2, (f32)rand() / divisor - r / 2},
    {(f32)rand() / divisor - r / 2, (f32)rand() / divisor - r / 2, (f32)rand() / divisor - r / 2},
    {(f32)rand() / divisor - r / 2, (f32)rand() / divisor - r / 2, (f32)rand() / divisor - r / 2},
    {(f32)rand() / divisor - r / 2, (f32)rand() / divisor - r / 2, (f32)rand() / divisor - r / 2},
    {(f32)rand() / divisor - r / 2, (f32)rand() / divisor - r / 2, (f32)rand() / divisor - r / 2},
};

void RenderScene(Shader& shader, Model& model)
{
    std::map<f32, v3> sorted;
    for (u32 i = 0; i < 10; i++)
    {
        float dist = glm::length(camPos - backpackPositions[i]);
        sorted[dist] = backpackPositions[i];
    }

    for (std::map<float, glm::vec3>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it)
    {
        mat4 modelMat = glm::translate(mat4(1.0f), it->second);
        shader.Set("model", modelMat);
        shader.Set("normalMatrix", glm::transpose(glm::inverse(mat3(modelMat))));

        model.Draw(shader);
    }
}

unsigned int quadVAO = 0;
unsigned int quadVBO;
void RenderQuad()
{
    if (quadVAO == 0)
    {
        // positions
        glm::vec3 pos1(-1.0f, 1.0f, 0.0f);
        glm::vec3 pos2(-1.0f, -1.0f, 0.0f);
        glm::vec3 pos3(1.0f, -1.0f, 0.0f);
        glm::vec3 pos4(1.0f, 1.0f, 0.0f);
        // texture coordinates
        glm::vec2 uv1(0.0f, 1.0f);
        glm::vec2 uv2(0.0f, 0.0f);
        glm::vec2 uv3(1.0f, 0.0f);
        glm::vec2 uv4(1.0f, 1.0f);
        // normal vector
        glm::vec3 nm(0.0f, 0.0f, 1.0f);

        // calculate tangent/bitangent vectors of both triangles
        glm::vec3 tangent1;
        glm::vec3 tangent2;
        // triangle 1
        // ----------
        glm::vec3 edge1 = pos2 - pos1;
        glm::vec3 edge2 = pos3 - pos1;
        glm::vec2 deltaUV1 = uv2 - uv1;
        glm::vec2 deltaUV2 = uv3 - uv1;

        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        tangent1 = glm::normalize(tangent1);

        // triangle 2
        // ----------
        edge1 = pos3 - pos1;
        edge2 = pos4 - pos1;
        deltaUV1 = uv3 - uv1;
        deltaUV2 = uv4 - uv1;

        f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        tangent2 = glm::normalize(tangent2);

        float quadVertices[] = 
        {
            pos1.x, pos1.y, pos1.z, uv1.x, uv1.y, nm.x, nm.y, nm.z, tangent1.x, tangent1.y, tangent1.z,
            pos2.x, pos2.y, pos2.z, uv2.x, uv2.y, nm.x, nm.y, nm.z, tangent1.x, tangent1.y, tangent1.z,
            pos3.x, pos3.y, pos3.z, uv3.x, uv3.y, nm.x, nm.y, nm.z, tangent1.x, tangent1.y, tangent1.z,

            pos1.x, pos1.y, pos1.z, uv1.x, uv1.y, nm.x, nm.y, nm.z, tangent2.x, tangent2.y, tangent2.z,
            pos3.x, pos3.y, pos3.z, uv3.x, uv3.y, nm.x, nm.y, nm.z, tangent2.x, tangent2.y, tangent2.z,
            pos4.x, pos4.y, pos4.z, uv4.x, uv4.y, nm.x, nm.y, nm.z, tangent2.x, tangent2.y, tangent2.z
        };
        // configure plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * 4, (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 11 * 4, (void*)(3 * 4));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * 4, (void*)(5 * 4));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * 4, (void*)(8 * 4));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}


int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    const u32 ScreenWidthPx = 1600;
    const u32 ScreenHeightPx = 1200;

    GLFWwindow* window = glfwCreateWindow(ScreenWidthPx, ScreenHeightPx, "PBR Engine", 0, 0);
    if (!window)
    {
        printf("Failed to create GLFW window.\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        printf("Failed to initialize GLAD.\n");
        return -1;
    }

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glViewport(0, 0, ScreenWidthPx, ScreenHeightPx);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA);
    glEnable(GL_CULL_FACE);
    glEnable(GL_MULTISAMPLE);
    glfwSetFramebufferSizeCallback(window, GLFWFramebufferSizeCallback);
    glfwSetCursorPosCallback(window, GLFWMouseCallback);
    glfwSetScrollCallback(window, GLFWScrollCallback);

    s32 maxAttributeCount;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxAttributeCount);
    printf("Max vertex attributes supported by hardware: %d\n", maxAttributeCount);

    Shader shader("assets/shaders/base.vert", "assets/shaders/base.frag");
    Shader lightCubeShader("assets/shaders/base.vert", "assets/shaders/light.frag");
    Shader screenShader("assets/shaders/screen.vert", "assets/shaders/blit.frag");
    Shader skyboxShader("assets/shaders/skybox.vert", "assets/shaders/skybox.frag");
    Shader shadowShader("assets/shaders/shadow.vert", "assets/shaders/shadow.frag");
    Shader pointShadowShader("assets/shaders/point_shadow.vert", "assets/shaders/point_shadow.frag", "assets/shaders/point_shadow.geom");

    stbi_set_flip_vertically_on_load(true);

    shader.Bind();
    shader.Set("material.shininess", 64.0f);

    Model model = Model("assets/models/backpack.obj");

    std::vector<const char*> faces = 
    {
        "assets/skybox/right.jpg",
        "assets/skybox/left.jpg",
        "assets/skybox/top.jpg",
        "assets/skybox/bottom.jpg",
        "assets/skybox/front.jpg",
        "assets/skybox/back.jpg",
    };
    stbi_set_flip_vertically_on_load(false);
    u32 cubemapTexture = LoadCubemap(faces);
    stbi_set_flip_vertically_on_load(true);

    f32 skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    u32 vaoSkybox;
    glGenVertexArrays(1, &vaoSkybox);
    glBindVertexArray(vaoSkybox);

    u32 vboSkybox;
    glGenBuffers(1, &vboSkybox);
    glBindBuffer(GL_ARRAY_BUFFER, vboSkybox);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32), 0);

    u32 fboShadowMap;
    glGenFramebuffers(1, &fboShadowMap);

    u32 ShadowMapWidth = 1024;
    u32 ShadowMapHeight = 1024;

    u32 shadowMap;
    glGenTextures(1, &shadowMap);
    glBindTexture(GL_TEXTURE_2D, shadowMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, ShadowMapWidth, ShadowMapHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    f32 borderColor[] = { 1.0f,1.0f,1.0f,1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, fboShadowMap);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    u32 fboDepthMap;
    glGenFramebuffers(1, &fboDepthMap);
    glBindFramebuffer(GL_FRAMEBUFFER, fboDepthMap);

    u32 depthCubemap;
    glGenTextures(1, &depthCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
    for (u32 i = 0; i < 6; i++)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, ShadowMapWidth, ShadowMapHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    }

    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    u32 framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    u32 textureColorBuffer;
    glGenTextures(1, &textureColorBuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, ScreenWidthPx, ScreenHeightPx, 0, GL_RGBA, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorBuffer, 0);

    u32 rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, ScreenWidthPx, ScreenHeightPx);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        printf("Framebuffer %d is not complete.", framebuffer);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    u32 texAO = LoadTexture("assets/models/ao.jpg", false);
    u32 texMetallic = LoadTexture("assets/models/roughness.jpg", false);
    u32 brickHeight = LoadTexture("assets/brick_height.jpg", false);

    while (!glfwWindowShouldClose(window))
    {
        ProcessInput(window);

        f32 time = (f32)glfwGetTime();

        dt = time - lastFrameTime;
        lastFrameTime = time;

        v3 lightPos = v3(+2.0f*sin(time), -2.0f, 2.0f);

        glEnable(GL_DEPTH_TEST);
        glViewport(0, 0, ShadowMapWidth, ShadowMapHeight);
        glBindFramebuffer(GL_FRAMEBUFFER, fboDepthMap);
        glClear(GL_DEPTH_BUFFER_BIT);
        
        f32 aspect = (f32)ShadowMapWidth / (f32)ShadowMapHeight;
        f32 pointLightNear = 1.0f;
        f32 pointLightFar = 25.0f;
        mat4 shadowProj = glm::perspective(glm::radians(90.0f), aspect, pointLightNear, pointLightFar);

        std::vector<mat4> shadowMats;
        shadowMats.push_back(shadowProj * glm::lookAt(lightPos, lightPos + v3(+1, 0, 0), v3(0, 1, 0)));
        shadowMats.push_back(shadowProj * glm::lookAt(lightPos, lightPos + v3(-1, 0, 0), v3(0, 1, 0)));
        shadowMats.push_back(shadowProj * glm::lookAt(lightPos, lightPos + v3(0, +1, 0), v3(0, 1, 0)));
        shadowMats.push_back(shadowProj * glm::lookAt(lightPos, lightPos + v3(0, -1, 0), v3(0, 1, 0)));
        shadowMats.push_back(shadowProj * glm::lookAt(lightPos, lightPos + v3(0, 0, +1), v3(0, 1, 0)));
        shadowMats.push_back(shadowProj * glm::lookAt(lightPos, lightPos + v3(0, 0, -1), v3(0, 1, 0)));

        pointShadowShader.Bind();
        for (u32 i = 0; i < 6; i++)
        {
            pointShadowShader.Set("shadowMatrices[" + std::to_string(i) + "]", shadowMats[i]);
        }
        pointShadowShader.Set("farPlane", pointLightFar);
        pointShadowShader.Set("lightPos", lightPos);
        RenderScene(pointShadowShader, model);

        glViewport(0, 0, ScreenWidthPx, ScreenHeightPx);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.7f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        mat4 modelMat = mat4(1.0f);
        mat4 view = glm::lookAt(camPos, camPos + camFront, camUp);
        mat4 projection = glm::perspective(glm::radians(fov), 800.0f / 600.0f, 0.1f, 100.0f);
        
        shader.Bind();
        shader.Set("viewPos", camPos);
        shader.Set("lightPos", lightPos);
        shader.Set("lightColor", v3(20.0f));
        shader.Set("model", modelMat);
        shader.Set("view", view);
        shader.Set("projection", projection);
        shader.Set("nearPlane", 0.1f);
        shader.Set("farPlane", 100.0f);
        shader.Set("heightScale", abs(sin(time))*0.05f);
        
        glActiveTexture(GL_TEXTURE11);
        glBindTexture(GL_TEXTURE_2D, texMetallic);
        shader.Set("material.metallic", 11);

        glActiveTexture(GL_TEXTURE12);
        glBindTexture(GL_TEXTURE_2D, texAO);
        shader.Set("material.ao", 12);

        glActiveTexture(GL_TEXTURE13);
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
        shader.Set("depthCubemap", 13);

        glActiveTexture(GL_TEXTURE15);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        shader.Set("skybox", 15);
        
        v3 lightPositions[] =
        {
            sin(time/1.0f + 464.46f)*1.0f*v3(5.0f, 10.0f, -3.0f),
            sin(time/2.0f + 12.74f)*2.0f*v3(3.0f, 6.0f, -1.0f),
            sin(time/4.0f)*3.0f*v3(-2.0f, 10.0f, -8.0f),
            sin(time/8.0f + 598.44f)*4.0f*v3(-2.0f, 2.0f, +3.0f),
        };
        v3 lightColors[] =
        {
            v3(20.0f, 15.0f, 2.0f),
            v3(100.0f, 30.0f, 15.0f),
            v3(30.0f, 20.0f, 50.0f),
            v3(15.0f, 15.0f, 10.0f),
        };

        for (u32 i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); i++)
        {
            shader.Set("lightPositions[" + std::to_string(i) + "]", lightPositions[i]);
            shader.Set("lightColors[" + std::to_string(i) + "]", lightColors[i]);
        }

        shader.Set("heightScale", abs(sin(time/3.0f)));
        RenderScene(shader, model);

        for (u32 i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); i++)
        {
            modelMat = glm::translate(mat4(1.0f), lightPositions[i]);
            shader.Set("model", modelMat);
            shader.Set("normalMatrix", glm::transpose(glm::inverse(mat3(modelMat))));
            model.Draw(shader);
        }

        glDepthFunc(GL_LEQUAL);
        skyboxShader.Bind();
        mat4 skyboxView = mat4(mat3(view));
        skyboxShader.Set("skybox", 0);
        skyboxShader.Set("view", skyboxView);
        skyboxShader.Set("projection", projection);
        glBindVertexArray(vaoSkybox);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glDepthFunc(GL_LESS);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        screenShader.Bind();
        screenShader.Set("exposure", 4.0f);
        glDisable(GL_DEPTH_TEST);
        glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
        
        glBindVertexArray(model.meshes[0].vao); // We have to have something bound, vertex shader doesn't work when nothing is bound :(
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}