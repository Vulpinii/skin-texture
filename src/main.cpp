#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <algorithm>
#include <thread>
#include <chrono>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <cstdio>
#include <string>

#ifdef __linux__
    #include <unistd.h>
    #define GetCurrentDir getcwd
#else
    #include <direct.h>
    #define GetCurrentDir _getcwd
#endif

// include project files
#include "Mesh.hpp"
#include "MeshRenderer.hpp"
#include "Camera.hpp"


// settings
unsigned int SCR_WIDTH = 1920;
unsigned int SCR_HEIGHT = 1080;
[[maybe_unused]] float lastX, lastY;
bool firstMouse = true;
double cursorXpos, cursorYpos;
bool wireFrame(false), lighting(true);
int camPlacement(0); float lightPlacement(0.5);
unsigned int meshVertices(0);
std::string loadedObjName, lastLoadedObjName;

MeshRenderer mrenderer, mrenderer2, lrenderer;
Camera mainCamera;
LightSource light;

// math
bool nearlyEqual(double a, double b, double epsilon);

// System
std::string getCurrentWorkingDirectory ();
void little_sleep(std::chrono::milliseconds us);

// Callbacks
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

// GUI Staff
void CherryTheme() ;
void renderGui(LightSource & light, Camera & mainCamera);


unsigned int quadVAO;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
                // positions        // texture Coords
                -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
                1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

        std::cout << "Quad created" << std::endl;
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    //glBindVertexArray(0);
}

void drawQuad(unsigned int quadID, unsigned int textureID){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(quadID);
    glUniform1i(glGetUniformLocation(quadID, "image"), 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    renderQuad();
}


// **************
// MAIN
int main()
{

    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    GLFWwindow* window = glfwCreateWindow((int)SCR_WIDTH, (int)SCR_HEIGHT, "Procedural Texture", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    // setup callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetKeyCallback(window, key_callback);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    
    // configure global opengl state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);

    // get current working directory
    std::string currentPath = getCurrentWorkingDirectory();
	std::cout << "Current working directory is " << currentPath << std::endl;

    // create shader
    Shader shader = Shader((currentPath+"/assets/shaders/vertex_shader.glsl").c_str(),
                           (currentPath+"/assets/shaders/fragment_shader.glsl").c_str());

    Shader lighting_shader = Shader((currentPath+"/assets/shaders/light_vertex_shader.glsl").c_str(),
                           (currentPath+"/assets/shaders/light_fragment_shader.glsl").c_str());

    Shader depth_shader = Shader((currentPath+"/assets/shaders/depth_vertex_shader.glsl").c_str(),
                           (currentPath+"/assets/shaders/depth_fragment_shader.glsl").c_str());

    //---
    Shader quad_shader = Shader((currentPath+"/assets/shaders/quad.vs.glsl").c_str(),
                                 (currentPath+"/assets/shaders/quad.fs.glsl").c_str());

    Shader compute_shader = Shader((currentPath+"/assets/shaders/godrays.cs.glsl").c_str());
    unsigned int tex_w = SCR_WIDTH; unsigned int tex_h = SCR_HEIGHT;
    unsigned int tex_norm = compute_shader.generateComputeTexture(tex_w, tex_h, 0);

    // create mesh
    // Thoracique/Thoracique_30 coeur
    Mesh tridimodel = Mesh((currentPath+"/assets/models/hand.off").c_str());
    Mesh lightmodel = Mesh((currentPath+"/assets/models/sphereHQ.off").c_str());

    // create renderer
    glm::vec3 objectcolor = glm::vec3(1.0, 0.75, 0.66);//glm::vec3(0.95, 0.5, 0.35);

    mrenderer = MeshRenderer(shader.ID, depth_shader.ID, tridimodel);
    mrenderer.setModelScale(glm::vec3(0.005));
    mrenderer.setModelTranslation(glm::vec3(125, -200.0, 0.0));
    mrenderer.setModelRotation(glm::vec3(0.0, -glm::radians(/*glfwGetTime()*/90.0f), 0.0));
    mrenderer.setModelColor(objectcolor);

    mrenderer2 = MeshRenderer(shader.ID, depth_shader.ID, tridimodel);
    mrenderer2.setModelScale(glm::vec3(-0.005, 0.005, 0.005));
    mrenderer2.setModelTranslation(glm::vec3(125, -200.0, 0.0));
    mrenderer2.setModelRotation(glm::vec3(0.0, -glm::radians(/*glfwGetTime()*/90.0f), 0.0));
    mrenderer2.setModelColor(objectcolor);

    // create camera
    mainCamera = Camera(glm::vec3(0.0 + cos(0.5 * 3.1415) * 3.0,
                                  -0.5,
                                  0 + sin(0.5 * 3.1415) * 3.0));
    mainCamera.setProjection(glm::perspective(glm::radians(45.0f), (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.01f, 100.0f));
    mainCamera.configure_depthMap();
    mainCamera.Target = glm::vec3(0.0, -1.0, 0.0);
    mainCamera.updateCameraVectorsFromFront();

    // create light position
    light = LightSource(glm::vec3(0.0, 0.25, 0));
    light.color = glm::vec3(0.95, 0.95, 0.9);
    light.configureDepthMapTo(glm::vec3(0.0, 0.0, 0.0));
    lrenderer = MeshRenderer(lighting_shader.ID, depth_shader.ID, lightmodel);
    lrenderer.setModelTranslation(glm::vec3(0.0, 0.25, 0.0));
    lrenderer.setModelRotation(glm::vec3(0.0, -glm::radians(/*glfwGetTime()*/90.0f), 0.0));
    lrenderer.setModelScale(glm::vec3(0.75));
    lrenderer.setModelColor(lighting_shader.ID, light.color);


    // configure framebuffer
    // ------------------------------
    unsigned int framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    // position color buffer
    unsigned int textureColorbuffer;
    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
    unsigned int textureMaskBuffer;
    glGenTextures(1, &textureMaskBuffer);
    glBindTexture(GL_TEXTURE_2D, textureMaskBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, textureMaskBuffer, 0);
    unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachments);

    // create and attach depth buffer (renderbuffer)
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT); // use a single renderbuffer object for both a depth AND stencil buffer.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it
    // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();

    // setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 450");
    // setup Dear ImGui theme
    CherryTheme();
    ImGui::GetStyle().WindowMinSize = ImVec2((float)SCR_WIDTH*0.2f, (float)SCR_HEIGHT);
    ImGui::GetIO().FontGlobalScale = float(SCR_WIDTH)/(1920);
    ImGui::GetIO().Fonts->AddFontFromFileTTF("../assets/fonts/Roboto-Light.ttf", 16.0f);
    // ------------------
    bool animatedLight = false;
    bool animatedCamera = false;
    glm::vec3 skinColor = objectcolor;
    glm::vec3 freckColor = glm::vec3(0.409, 0.101, 0.108);
    float freck_scale = 0.3;
    float freck_frequency = 5.0;

    // RENDER LOOP -----
    while (!glfwWindowShouldClose(window))
    {
        //glClearColor(50.0f/255.0f, 50.0f/255.0f, 50.0f/255.0f, 1.0f);
        glClearColor(0.0f/255.0f, 0.0f/255.0f, 0.0f/255.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if(wireFrame) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            lrenderer.draw(lighting_shader.ID, mainCamera, light);
            mrenderer.draw(shader.ID, mainCamera,light);
            mrenderer2.draw(shader.ID, mainCamera, light);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // compute shader
        {
            compute_shader.use();
            glMemoryBarrier(GL_ALL_BARRIER_BITS);
            glBindImageTexture(0, tex_norm, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, textureMaskBuffer);
            glm::vec4 clipSpacePos = mainCamera.projection * (mainCamera.GetViewMatrix() * glm::vec4(light.position, 1.0));
            glm::vec3 ndcSpacePos = glm::vec3(clipSpacePos.x, clipSpacePos.y, clipSpacePos.z) / clipSpacePos.w;
            glm::vec2 windowSpacePos = ((glm::vec2(ndcSpacePos.x, ndcSpacePos.y) + glm::vec2(1.0)) / 2.0f);
            compute_shader.setVec2("sunPos", windowSpacePos);
            glDispatchCompute((GLuint)tex_w/16, (GLuint)tex_h/16, 1);
            glMemoryBarrier(GL_ALL_BARRIER_BITS);
        }
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        drawQuad(quad_shader.ID, tex_norm);

        // feed inputs to dear imgui, start new frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        //renderGui(light, mainCamera);
        if(ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoMove)){
            ImGui::SetWindowPos(ImVec2(0, 0), ImGuiCond_Once);
            ImGui::SetWindowSize(ImVec2(400, (float)SCR_HEIGHT));
            ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth());

            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
            if (ImGui::CollapsingHeader("Skin", ImGuiTreeNodeFlags_None)) {
                ImGui::Dummy(ImVec2(0.0f, 10.0f));
                if (ImGui::Button("Reset Skin Presets", ImVec2(ImGui::GetContentRegionAvailWidth(),0))) {
                    freck_scale = 0.3;
                    freck_frequency = 5.0;
                    freckColor = glm::vec3(0.409, 0.101, 0.108);
                    skinColor = objectcolor;
                }
                ImGui::Dummy(ImVec2(0.0f, 10.0f));
                ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth()*0.80);
                ImGui::ColorEdit3("##ColorSkin", &skinColor.x);
                ImGui::SameLine(); ImGui::Text("Skin");
                ImGui::Dummy(ImVec2(0.0,10.0));
                ImGui::ColorEdit3("##ColorFreck", &freckColor.x);
                ImGui::SameLine(); ImGui::Text("Freck");
                ImGui::PopItemWidth();
                ImGui::Dummy(ImVec2(0.0,10.0));
                ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth()*0.72);
                ImGui::DragFloat("Freck frequency", &freck_frequency, 0.01, 0.0, 100.0);
                ImGui::Dummy(ImVec2(0.0,10.0));
                ImGui::DragFloat("Freck scale", &freck_scale, 0.001, 0.0f);
                ImGui::PopItemWidth();
                ImGui::Dummy(ImVec2(0.0f, 20.0f));
                ImGui::Separator();
            }

            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
            if (ImGui::CollapsingHeader("Lighting", ImGuiTreeNodeFlags_None)) {
                ImGui::Dummy(ImVec2(0.0f, 10.0f));
                ImGui::Checkbox("Animated light", &animatedLight);
                ImGui::Dummy(ImVec2(0.0f, 10.0f));
                if (ImGui::Button("Reset Light Position", ImVec2(ImGui::GetContentRegionAvailWidth(),0))) {
                    light.position = glm::vec3(0, 0.25, 0);
                }
                ImGui::Dummy(ImVec2(0.0f, 10.0f));
                ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth()*0.72);
                ImGui::DragFloat3("##LightPosition", &light.position.x, 0.01);
                ImGui::SameLine(); ImGui::Text("Position");
                ImGui::PopItemWidth();
                ImGui::Dummy(ImVec2(0.0f, 10.0f));
                ImGui::ColorEdit3("##Color", &light.color.x);


                ImGui::Dummy(ImVec2(0.0f, 20.0f));
                ImGui::Separator();
            }

            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
            if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_None)) {
                ImGui::Dummy(ImVec2(0.0f, 10.0f));
                ImGui::Checkbox("Animated camera", &animatedCamera);
                ImGui::Dummy(ImVec2(0.0f, 10.0f));
                if (ImGui::Button("Reset Camera Position", ImVec2(ImGui::GetContentRegionAvailWidth(),0))) {
                    mainCamera.Position = glm::vec3(glm::vec3(0.0 + cos(0.5 * 3.1415) * 3.0,
                                                              -0.5,
                                                              0 + sin(0.5 * 3.1415) * 3.0));
                }
                ImGui::Dummy(ImVec2(0.0f, 10.0f));
                ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth()*0.72);
                ImGui::DragFloat3("##CameraPosition", &mainCamera.Position.x, 0.01);
                ImGui::SameLine(); ImGui::Text("Position");
                ImGui::PopItemWidth();
                ImGui::Dummy(ImVec2(0.0f, 20.0f));
                ImGui::Separator();
            }

            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
            if (ImGui::CollapsingHeader("Textures", ImGuiTreeNodeFlags_None)) {
                ImGui::Dummy(ImVec2(0.0f, 10.0f));
                ImVec2 region = ImVec2(ImGui::GetContentRegionAvailWidth(), ImGui::GetContentRegionAvailWidth()*(9.0/16.0));
                ImGui::Text("Color texture : ");
                ImGui::Dummy(ImVec2(0.0f, 5.0f));
                ImGui::Image((void*)(intptr_t)textureColorbuffer, region, ImVec2(0,1), ImVec2(1,0));
                ImGui::Dummy(ImVec2(0.0f, 20.0f));
                ImGui::Text("Mask texture : ");
                ImGui::Dummy(ImVec2(0.0f, 5.0f));
                ImGui::Image((void*)(intptr_t)textureMaskBuffer, region, ImVec2(0,1), ImVec2(1,0));

                ImGui::Dummy(ImVec2(0.0f, 20.0f));
                ImGui::Separator();
            }

            ImGui::PopItemWidth();
        }
        ImGui::End();

        // update uniforms -------------------------------------------------------------------
        {
            if (animatedLight) {
                light.position = glm::vec3(sin(glfwGetTime() * 2.0f) * 0.075f,
                                           light.position.y,
                                           cos(glfwGetTime() * 2.0f) * 0.075f);
            }
            lrenderer.setModelNewTranslation(light.position);
            lrenderer.setModelColor(lighting_shader.ID, light.color);
            shader.use();
            shader.setVec3("lightColor", light.color);
            shader.setVec3("freck_col", freckColor);
            shader.setFloat("freck_scale", freck_scale);
            shader.setFloat("freck_frequency", freck_frequency);
            mrenderer.setModelColor(skinColor);
            mrenderer2.setModelColor(skinColor);
            if (animatedCamera) {
                mainCamera.Position = glm::vec3(cos(glfwGetTime() / 10.0f) * 3.0f,
                                                mainCamera.Position.y,
                                                sin(glfwGetTime() / 10.0f) * 3.0f);
            }
        }
        //-------------------------------------------------------------------------------------



        // Render dear imgui into screen
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();

        //little_sleep(std::chrono::milliseconds(25));
    }
    // clean up
    mrenderer.cleanUp();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    return 0;
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
    glViewport(0, 0, width, height);
    ImGui::GetIO().FontGlobalScale = 1 + float(SCR_WIDTH)/(1920);
    ImGui::GetStyle().WindowMinSize = ImVec2((float)SCR_WIDTH*0.2f, (float)SCR_HEIGHT);
}


// glfw: whenever the mouse moves, this callback is called
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = (float) xpos;
        lastY = (float) ypos;
        firstMouse = false;
    }

    lastX = (float) xpos;
    lastY = (float) ypos;
}


// glfw: mouse button callback
// ---------------------------
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) 
    {
       //getting cursor position
       glfwGetCursorPos(window, &cursorXpos, &cursorYpos);
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
        glfwSetWindowShouldClose(window, true);
    }
}

// Those light colors are better suited with a thicker font than the default one + FrameBorder
// From https://github.com/procedural/gpulib/blob/master/gpulib_imgui.h
void CherryTheme() {
    // cherry colors, 3 intensities
#define HI(v)   ImVec4(0.35f, 0.35f, 0.35f, v)
#define MED(v)  ImVec4(0.20, 0.20, 0.20, v)
#define LOW(v)  ImVec4(0.25, 0.25, 0.25, v)
    // backgrounds (@todo: complete with BG_MED, BG_LOW)
#define BG(v)   ImVec4(0.15, 0.15f, 0.15f, v)
    // text
#define TEXT(v) ImVec4(0.860f, 0.930f, 0.890f, v)

    auto &style = ImGui::GetStyle();
    style.Colors[ImGuiCol_Text]                  = TEXT(0.78f);
    style.Colors[ImGuiCol_TextDisabled]          = TEXT(0.28f);
    style.Colors[ImGuiCol_WindowBg]              = BG( 0.9f);//ImVec4(0.13f, 0.14f, 0.17f, 1.00f);
    style.Colors[ImGuiCol_ChildBg]               = BG( 0.58f);
    style.Colors[ImGuiCol_PopupBg]               = BG( 0.9f);
    style.Colors[ImGuiCol_Border]                = ImVec4(0.31f, 0.31f, 1.00f, 0.00f);
    style.Colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_FrameBg]               = HI( 0.25f); // slider background
    style.Colors[ImGuiCol_FrameBgHovered]        = MED( 0.78f);
    style.Colors[ImGuiCol_FrameBgActive]         = MED( 1.00f);
    style.Colors[ImGuiCol_TitleBg]               = LOW( 1.00f);
    style.Colors[ImGuiCol_TitleBgActive]         = HI( 1.00f);
    style.Colors[ImGuiCol_TitleBgCollapsed]      = BG( 0.75f);
    style.Colors[ImGuiCol_MenuBarBg]             = MED( 0.57f);
    style.Colors[ImGuiCol_ScrollbarBg]           = BG( 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.09f, 0.15f, 0.16f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered]  = MED( 0.78f);
    style.Colors[ImGuiCol_ScrollbarGrabActive]   = MED( 1.00f);
    style.Colors[ImGuiCol_CheckMark]             = ImVec4(0.71f, 0.22f, 0.27f, 1.00f);
    style.Colors[ImGuiCol_SliderGrab]            = ImVec4(0.47f, 0.77f, 0.83f, 0.14f);
    style.Colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.71f, 0.22f, 0.27f, 1.00f);
    style.Colors[ImGuiCol_Button]                = ImVec4(0.47f, 0.77f, 0.83f, 0.14f);
    style.Colors[ImGuiCol_ButtonHovered]         = MED( 0.86f);
    style.Colors[ImGuiCol_ButtonActive]          = MED( 1.00f);
    style.Colors[ImGuiCol_Header]                = MED( 0.76f);
    style.Colors[ImGuiCol_HeaderHovered]         = MED( 0.86f);
    style.Colors[ImGuiCol_HeaderActive]          = HI( 1.00f);
    style.Colors[ImGuiCol_ResizeGrip]            = ImVec4(0.47f, 0.77f, 0.83f, 0.04f);
    style.Colors[ImGuiCol_ResizeGripHovered]     = MED( 0.78f);
    style.Colors[ImGuiCol_ResizeGripActive]      = MED( 1.00f);
    style.Colors[ImGuiCol_PlotLines]             = TEXT(0.63f);
    style.Colors[ImGuiCol_PlotLinesHovered]      = MED( 1.00f);
    style.Colors[ImGuiCol_PlotHistogram]         = TEXT(0.63f);
    style.Colors[ImGuiCol_PlotHistogramHovered]  = MED( 1.00f);
    style.Colors[ImGuiCol_TextSelectedBg]        = MED( 0.43f);
    style.Colors[ImGuiCol_ModalWindowDarkening]  = BG( 0.73f);

    style.WindowPadding            = ImVec2(6, 4);
    style.WindowRounding           = 0.0f;
    style.FramePadding             = ImVec2(5, 2);
    style.FrameRounding            = 3.0f;
    style.ItemSpacing              = ImVec2(7, 1);
    style.ItemInnerSpacing         = ImVec2(1, 1);
    style.TouchExtraPadding        = ImVec2(0, 0);
    style.IndentSpacing            = 6.0f;
    style.ScrollbarSize            = 12.0f;
    style.ScrollbarRounding        = 16.0f;
    style.GrabMinSize              = 20.0f;
    style.GrabRounding             = 2.0f;

    style.WindowTitleAlign.x = 0.50f;

    style.Colors[ImGuiCol_Border] = ImVec4(0.539f, 0.479f, 0.255f, 0.162f);
    style.FrameBorderSize = 0.0f;
    style.WindowBorderSize = 1.0f;
}


void little_sleep(std::chrono::milliseconds us)
{
    auto start = std::chrono::high_resolution_clock::now();
    auto end = start + us;
    do {
        std::this_thread::yield();
        //if (!awake) std::cout << "sleep" << std::endl;
        //else {std::cout << "-------- AWAKE" << std::endl; break;}
    } while (std::chrono::high_resolution_clock::now() < end);
}

bool nearlyEqual(double a, double b, double epsilon)
{
    // if the distance between a and b is less than epsilon, then a and b are "close enough"
    return std::abs(a - b) <= epsilon;
}

std::string getCurrentWorkingDirectory (){
    char cCurrentPath[FILENAME_MAX];

    if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath))){return std::string();}

    cCurrentPath[sizeof(cCurrentPath) - 1] = '\0';
    std::string currentPath = std::string(cCurrentPath);
    int found = currentPath.find("build");
    currentPath = currentPath.substr(0, found);
    unsigned int i = 0;
    while (i < currentPath.size()){
        if (currentPath[i] == '\\') {currentPath[i] = '/';}
        ++i;
    }
    return currentPath;
}
