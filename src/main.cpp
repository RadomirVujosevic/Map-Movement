#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <learnopengl/player.hpp>

#include <iostream>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void processInput(GLFWwindow *window, Player &p, std::vector<Marker>);
unsigned int loadTexture(const char *path);

unsigned int loadCubemap(std::vector<std::string> faces);

bool shadows = true;
bool shadowsKeyPressed = false;

// settings
const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 800;

// camera
Camera camera(glm::vec3(0.0f, 15.0f, 15.0f), glm::vec3(0.0f, 1.0f, 0.0f),
              -90.0f, -45.0f);
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
float deltaTime = 0.0f;
float lastFrame = 0.0f;
bool firstMouse = true;

// timing

// rotate,translate and scale a model
glm::mat4 RTS(glm::vec3 translate = glm::vec3(0.0f, 0.0f, 0.0f),
              glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f),
              float angle = 0.0f,
              glm::vec3 rotation = glm::vec3(1.0f, 0.0f, 0.0f));

void drawObject(Model &objectModel, glm::mat4 model, Shader &shader);
void drawPlane(Shader &planeShader, glm::vec3 lightPosition, Model &planeModel);
void setModelShader(Shader &modelShader, glm::vec3 position);
void drawSkybox(Shader &skyboxShader, unsigned int skyboxVAO, unsigned cubemapTexture);
void initSkybox(Shader &skyboxShader, unsigned int *skyboxVAO, unsigned int *cubemapTexture);
void drawArrows(Shader &skyboxShader, int skyboxVAO, unsigned cubemapTexture, glm::mat4 model);
void initArrows(Shader &arrowShader, unsigned int *arrowVAO, unsigned int *arrowTexture);

int main()
{
    srand(glfwGetTime());
    // Load GLFW and Create a Window
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    auto mWindow =
        glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL", nullptr, nullptr);

    // Check for Valid Context

    if (mWindow == nullptr)
    {
        fprintf(stderr, "Failed to Create OpenGL Context");
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(mWindow);
    glfwSetFramebufferSizeCallback(mWindow, framebuffer_size_callback);
    glfwSetCursorPosCallback(mWindow, mouse_callback);
    glfwSetScrollCallback(mWindow, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Create Context and Load OpenGL Functions
    glfwMakeContextCurrent(mWindow);
    gladLoadGL();

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading
    // model).
    // stbi_set_flip_vertically_on_load(true);

    // configure global opengl state
    // -----------------------------

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);

    // build and compile shaders
    // -------------------------
    Shader modelShader("resources/shaders/modelShader.vs",
                       "resources/shaders/modelShader.fs");

    Shader skyboxShader("resources/shaders/skyboxShader.vs",
                        "resources/shaders/skyboxShader.fs");

    Shader planeShader("resources/shaders/planeShader.vs",
                       "resources/shaders/planeShader.fs");

    Shader arrowShader("resources/shaders/arrowShader.vs",
                       "resources/shaders/arrowShader.fs");

    glm::vec3 lightPos(0.0f, 0.0f, 0.0f);

    Model planeModel("resources/objects/plane/plane.obj");
    Model markerModel("resources/objects/marker/marker.obj");
    // location of all the markers
    std::vector<Marker> markers;
    std::ifstream markerFile("resources/markerLocations.txt");
    float a, b;
    int idx = 0;
    while (markerFile >> a >> b)
    {
        Marker m(idx++, a, b);
        markers.push_back(m);
    }
    markerFile.close();
    markerFile.open("resources/markerConnections.txt");
    while (markerFile >> a >> b)
    {
        markers[a].addNeighbour(markers[b]);
        markers[b].addNeighbour(markers[a]);
    }
    markerFile.close();

    arrowShader.use();
    arrowShader.setInt("texture1", 0);

    Player player(markers[0], glm::vec3(0.02f));

    unsigned int skyboxVAO, cubemapTexture;
    initSkybox(skyboxShader, &skyboxVAO, &cubemapTexture);

    // postavljanje buffera za strelice
    unsigned int arrowVAO, arrowTexture;
    initArrows(arrowShader, &arrowVAO, &arrowTexture);

    // Rendering Loop
    while (glfwWindowShouldClose(mWindow) == false)
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        lightPos = player.position + glm::vec3(4.0 * sin(glfwGetTime()), 4.0f, 4.0 * cos(glfwGetTime()));

        processInput(mWindow, player, markers);
        player.processMovement();

        // Background Fill Color
        glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        setModelShader(modelShader, lightPos);
        for (auto &it : markers)
        {
            drawObject(markerModel, RTS(it.position, glm::vec3(0.2f), glm::radians(180.0f)),
                       modelShader);
        }
        player.draw(modelShader);

        glm::mat4 arrowModel = RTS(player.position + glm::vec3(0.0f, 0.0f, 0.7f), glm::vec3(0.3f), glm::radians(90.0f));
        drawArrows(arrowShader, arrowVAO, arrowTexture, arrowModel);
        arrowModel = RTS(player.position + glm::vec3(0.5f, 0.0f, 0.5f), glm::vec3(0.3f), glm::radians(90.0f));
        arrowModel = glm::rotate(arrowModel, glm::radians(45.0f), glm::vec3(0.0f, 0.0f, -1.0f));
        drawArrows(arrowShader, arrowVAO, arrowTexture, arrowModel);
        drawSkybox(skyboxShader, skyboxVAO, cubemapTexture);

        drawPlane(planeShader, player.position, planeModel);

        // Flip Buffers and Draw
        glfwSwapBuffers(mWindow);
        glfwPollEvents();
    }
    glfwTerminate();
    return EXIT_SUCCESS;
}

void renderScene()
{
}

void drawObject(Model &objectModel, glm::mat4 model, Shader &shader)
{
    shader.use();
    shader.setMat4("model", model);
    objectModel.Draw(shader);
}

glm::mat4 RTS(glm::vec3 translate, glm::vec3 scale, float angle, glm::vec3 rotation)
{
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, translate);
    model = glm::scale(model, scale);
    model = glm::rotate(model, angle, rotation);
    return model;
}

void processInput(GLFWwindow *window, Player &player,
                  std::vector<Marker> markers)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        player.setRandomMovementTarget();
    //  if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    //     camera.ProcessKeyboard(RIGHT, deltaTime);
    // if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    //     camera.ProcessKeyboard(RIGHT, deltaTime);
    // if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    //     camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this
// callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that
    // width and height will be significantly larger than specified on
    // retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset =
        lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

unsigned int loadCubemap(std::vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data =
            stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width,
                         height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i]
                      << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

void drawPlane(Shader &planeShader, glm::vec3 lightPosition,
               Model &planeModel)
{

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    planeShader.use();

    planeShader.setVec3("viewPos", camera.Position);
    planeShader.setInt("shadows", 0); // enable/disable shadows by pressing 'SPACE'

    planeShader.setVec3("viewPos", camera.Position);
    planeShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
    planeShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
    planeShader.setVec3("dirLight.diffuse", 0.1f, 0.1f, 0.1f);
    planeShader.setVec3("spotLight.position",
                        lightPosition + glm::vec3(0.0f, 5.0f, 0.0f));
    planeShader.setVec3("spotLight.direction", glm::vec3(0.0f, -1.0f, 0.0f));
    planeShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
    planeShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
    planeShader.setFloat("spotLight.constant", 1.0f);
    planeShader.setFloat("spotLight.linear", 0.09f);
    planeShader.setFloat("spotLight.quadratic", 0.032f);
    planeShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(45.0f)));
    planeShader.setFloat("spotLight.outerCutOff",
                         glm::cos(glm::radians(50.0f)));

    glm::mat4 projection =
        glm::perspective(glm::radians(camera.Zoom),
                         (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    planeShader.setMat4("projection", projection);
    planeShader.setMat4("view", view);
    drawObject(planeModel, RTS(glm::vec3(0.0f, -0.15f, 0.0f), glm::vec3(4.0f)),
               planeShader);

    glDisable(GL_CULL_FACE);
}

void setModelShader(Shader &modelShader, glm::vec3 position)
{
    modelShader.use();
    modelShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
    modelShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
    modelShader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
    modelShader.setVec3("dirLight.specular", 1.0f, 1.0f, 1.0f);
    modelShader.setFloat("shininess", 64.0f);

    modelShader.setVec3("pointLight.position", position);
    modelShader.setVec3("pointLight.ambient", 0.05f, 0.05f, 0.05f);
    modelShader.setVec3("pointLight.diffuse", 0.8f, 0.8f, 0.8f);
    modelShader.setVec3("pointLight.specular", 1.0f, 1.0f, 1.0f);
    modelShader.setFloat("pointLight.constant", 1.0f);
    modelShader.setFloat("pointLight.linear", 0.09);
    modelShader.setFloat("pointLight.quadratic", 0.032);

    glm::mat4 projection =
        glm::perspective(glm::radians(camera.Zoom),
                         (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    modelShader.setMat4("projection", projection);
    modelShader.setMat4("view", view);
}

void drawSkybox(Shader &skyboxShader, unsigned int skyboxVAO, unsigned cubemapTexture)
{
    glDepthFunc(GL_LEQUAL); // change depth function so depth test passes when
                            // values are equal to depth buffer's content
    skyboxShader.use();
    glm::mat4 view = glm::mat4(glm::mat3(
        camera.GetViewMatrix())); // remove translation from the view matrix
    glm::mat4 projection =
        glm::perspective(glm::radians(camera.Zoom),
                         (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    skyboxShader.setMat4("view", view);
    skyboxShader.setMat4("projection", projection);
    // skybox cube
    glBindVertexArray(skyboxVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);
}

void initSkybox(Shader &skyboxShader, unsigned int *skyboxVAO, unsigned int *cubemapTexture)
{
    float skyboxVertices[] = {
        // positions
        -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f,

        -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f,

        1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f,

        -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f};

    // skybox VAO
    unsigned int skyboxVBO;
    glGenVertexArrays(1, skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(*skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices,
                 GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                          (void *)0);

    std::vector<std::string> faces = {"resources/Skybox/right.jpg",
                                      "resources/Skybox/left.jpg",
                                      "resources/Skybox/top.jpg",
                                      "resources/Skybox/bottom.jpg",
                                      "resources/Skybox/front.jpg",
                                      "resources/Skybox/back.jpg"};

    *cubemapTexture = loadCubemap(faces);
    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);
}

void initArrows(Shader &arrowShader,
                unsigned int *arrowVAO,
                unsigned int *arrowTexture)
{

    // Crtanje strelica
    float arrowVertices[] = {
        // positions          // texture coords
        0.5f, 0.5f, 0.0f, 1.0f, 1.0f,   // top right
        0.5f, -0.5f, 0.0f, 1.0f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, // bottom left
        -0.5f, 0.5f, 0.0f, 0.0f, 1.0f   // top left
    };
    unsigned int indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    unsigned int arrowVBO, arrowEBO;
    glGenVertexArrays(1, arrowVAO);
    glGenBuffers(1, &arrowVBO);
    glGenBuffers(1, &arrowEBO);

    glBindVertexArray(*arrowVAO);

    glBindBuffer(GL_ARRAY_BUFFER, arrowVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(arrowVertices), arrowVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, arrowEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    *arrowTexture = loadTexture("resources/textures/arrow.png");

    arrowShader.use();
    arrowShader.setInt("texture1", 0);
}

void drawArrows(Shader &arrowShader, int arrowVAO, unsigned arrowTexture, glm::mat4 model)
{
    arrowShader.use();
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    arrowShader.setMat4("projection", projection);
    arrowShader.setMat4("view", view);

    arrowShader.setMat4("model", model);

    glBindVertexArray(arrowVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, arrowTexture);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const *path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}