#include "main.h"
#include "structure.h"
#include "bobby.h"
#include "objects.h"
#include "shader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
void RenderText(Shader &shader, std::string text, float x, float y, float scale, glm::vec3 color);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

/// Holds all state information relevant to a character as loaded using FreeType
struct Character
{
    unsigned int TextureID; // ID handle of the glyph texture
    glm::ivec2 Size;        // Size of glyph
    glm::ivec2 Bearing;     // Offset from baseline to left/top of glyph
    unsigned int Advance;   // Horizontal offset to advance to next glyph
};

std::map<GLchar, Character> Characters;

const char *vertexShaderSource = "#version 330 core\n"
                                 "layout (location = 0) in vec3 aPos;\n"
                                 "uniform mat4 transform;\n"
                                 "void main()\n"
                                 "{\n"
                                 "   gl_Position = transform * vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
                                 "}\0";
const char *fragmentShaderSource = "#version 330 core\n"
                                   "out vec4 FragColor;\n"
                                   "uniform vec4 col;\n"
                                   "void main()\n"
                                   "{\n"
                                   "   FragColor = col;\n"
                                   "}\n\0";

unsigned int VAO, VBO;

Floor game_floor_1;
Floor game_floor_2;
Floor game_floor_3;
Ceiling game_ceiling_1;
Ceiling game_ceiling_2;
Ceiling game_ceiling_3;
Bobby bobby_1;
Bobby bobby_2;
Bobby bobby_3;
double velocity;
Coin coin1a;
Coin coin1b;
Coin coin2b;
Coin coin1c;
Coin coin2c;
Coin coin3c;
Game game;
Zapper zapper1a;
Zapper zapper1b;
Zapper zapper2b;
Zapper zapper1c;
Zapper zapper2c;
Zapper zapper3c;
int currLevel = 1;
int coins_collected;
double past;
double present;
double gravity = 9.8;
double delta;
double start;
int die;
float move_x = 0;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // build and compile our shader program
    // ------------------------------------
    // vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }
    // fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }
    // link shaders
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                  << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Shader shader("../src/text.vs", "../src/text.fs");
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), 0.0f, static_cast<float>(SCR_HEIGHT));
    shader.use();
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // FreeType
    // --------
    FT_Library ft;
    // All functions return a value different than 0 whenever an error occurred
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return -1;
    }

    // find path to font
    std::string font_name = "../fonts/Inter-SemiBold.ttf";
    if (font_name.empty())
    {
        std::cout << "ERROR::FREETYPE: Failed to load font_name" << std::endl;
        return -1;
    }

    // load font as face
    FT_Face face;
    if (FT_New_Face(ft, font_name.c_str(), 0, &face))
    {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
        return -1;
    }
    else
    {
        // set size to load glyphs as
        FT_Set_Pixel_Sizes(face, 0, 48);

        // disable byte-alignment restriction
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        // load first 128 characters of ASCII set
        for (unsigned char c = 0; c < 128; c++)
        {
            // Load character glyph
            if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            {
                std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
                continue;
            }
            // generate texture
            unsigned int texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer);
            // set texture options
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            // now store character for later use
            Character character = {
                texture,
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                static_cast<unsigned int>(face->glyph->advance.x)};
            Characters.insert(std::pair<char, Character>(c, character));
        }
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    // destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    Shader ourShader("../src/shader.vs", "../src/shader.fs");

    float vertices[] = {
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 0.0f, 1.0f};
    unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3};

    unsigned int VAO_texture, VBO_texture, EBO_texture;
    glGenVertexArrays(1, &VAO_texture);
    glGenBuffers(1, &VBO_texture);
    glGenBuffers(1, &EBO_texture);

    glBindVertexArray(VAO_texture);

    glBindBuffer(GL_ARRAY_BUFFER, VBO_texture);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_texture);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    unsigned int texture1, texture2, texture3;
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    int width, height, channels;
    // The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load("../textures/background.jpg", &width, &height, &channels, STBI_rgb);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width_2, height_2, channels_2;

    stbi_set_flip_vertically_on_load(true);
    unsigned char *data2 = stbi_load("../textures/player.png", &width_2, &height_2, &channels_2, STBI_rgb_alpha);
    if (data2)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, width_2, height_2, 0, GL_RGBA, GL_UNSIGNED_BYTE, data2);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data2);

    glGenTextures(1, &texture3);
    glBindTexture(GL_TEXTURE_2D, texture3); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width_3, height_3, channels_3;

    stbi_set_flip_vertically_on_load(true);
    unsigned char *data3 = stbi_load("../textures/zapper.png", &width_3, &height_3, &channels_3, STBI_rgb_alpha);
    if (data3)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, width_3, height_3, 0, GL_RGBA, GL_UNSIGNED_BYTE, data3);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data3);

    Shader blurShader("../src/blur_vertex.vs", "../src/blur_shader.fs");
    Shader HDRshader("../src/hdr.vs", "../src/hdr.fs");

    unsigned int hdrFBO;
    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

    unsigned int colorBuffers[2];
    glGenTextures(2, colorBuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // attach texture to framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
    }
    // create and attach depth buffer (renderbuffer)
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
    unsigned int attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, attachments);
    // finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    unsigned int pingpongFBO[2];
    unsigned int pingpongColorbuffers[2];
    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongColorbuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);
        // also check if framebuffers are complete (no need for depth buffer)
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;
    }

    float quadVertices[] = {
        -1.0f, 1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 0.0f,

        1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f};

    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    velocity = 0;
    coins_collected = 0;

    game_floor_1.createVAO();
    game_ceiling_1.createVAO();
    bobby_1.createVAO();
    coin1a.createVAO(1);
    zapper1a.createVAO(1);

    past = glfwGetTime();
    start = glfwGetTime();
    die = 0;

    ourShader.use();
    unsigned int transformbackground = glGetUniformLocation(ourShader.ID, "transback");
    glUniform2f(transformbackground, 0.0f, 0.0f);
    unsigned int transformLoc = glGetUniformLocation(ourShader.ID, "transform");
    glm::vec3 blur = glm::vec3(1.0f);
    unsigned int BlurLoc = glGetUniformLocation(ourShader.ID, "blur");
    unsigned int OpaqueLoc = glGetUniformLocation(ourShader.ID, "opaque");

    glUniform3fv(BlurLoc, 1, glm::value_ptr(blur));
    glUniform1f(OpaqueLoc, 1.0f);

    blur = glm::vec3(1.0f, 1.0f, 0.0f);

    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        ourShader.use();

        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
        move_x -= 0.01;

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glm::mat4 trans = glm::mat4(1.0f);

        ourShader.use();
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));

        ourShader.setInt("Texture", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glUniform2f(transformbackground, move_x, 0.0f);
        glBindVertexArray(VAO_texture);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glUniform1f(OpaqueLoc, 1.0f);
        glUniform3fv(BlurLoc, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f, 0.0f)));

        glUniform2f(transformbackground, 0.0f, 0.0f);

        char score[100];
        char level[100];
        sprintf(score, "Total Coins: %d", coins_collected);
        sprintf(level, "Current Level: %d", currLevel);
        string lev = level;
        string message = score;
        string target = "Target: 10";

        RenderText(shader, message, 20.0f, 570.0f, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f));
        RenderText(shader, lev, 320.0f, 15.0f, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f));
        RenderText(shader, target, 660.0f, 570.0f, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f));

        present = glfwGetTime();
        delta = present - past;
        velocity += gravity * delta;
        if (bobby_1.y > 0)
        {
            bobby_1.y -= velocity * delta * 0.4;
            bobby_1.abs_y -= velocity * delta * 0.4;
        }
        past = present;

        int distance = present - start;
        char dist[100];
        sprintf(dist, "Distance travelled: %d", distance);
        string curr_dist = dist;
        RenderText(shader, curr_dist, 400.0f, 570.0f, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f));

        ourShader.use();
        ourShader.setInt("Texture", 1);
        blur = glm::vec3(1, 1, 1);
        glUniform3fv(BlurLoc, 1, glm::value_ptr(blur));
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);
        bobby_1.draw(ourShader.ID);

        glUseProgram(shaderProgram);

        coin1a.draw(shaderProgram, coin1a.visible);
        coin1a.visible = 1;
        bool collected = game.coin_collision(bobby_1, coin1a);
        if (collected)
        {
            coin1a.visible = 0;
            coins_collected++;
        }

        blur = glm::vec3(0);
        glUniform3fv(BlurLoc, 1, glm::value_ptr(blur));

        ourShader.use();
        ourShader.setInt("Texture", 2);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, texture3);
        zapper1a.draw(ourShader.ID);
        glUniform3fv(BlurLoc, 1, glm::value_ptr(glm::vec3(0.0f)));
        bool dead = game.zapper_collision(bobby_1, zapper1a);

        bool horizontal = true, first_iteration = true;
        unsigned int amount = 10;
        blurShader.use();
        for (unsigned int i = 0; i < amount; i++)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            blurShader.setInt("image", 0);
            blurShader.setInt("horizontal", horizontal);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongColorbuffers[!horizontal]);
            glBindVertexArray(quadVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);
            horizontal = !horizontal;
            if (first_iteration)
                first_iteration = false;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT);
        HDRshader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);
        HDRshader.setInt("scene", 0);
        HDRshader.setInt("bloomBlur", 1);
        HDRshader.setInt("bloom", 1);
        HDRshader.setFloat("exposure", 3.0f);
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        if (dead)
        {
            currLevel = 5;
            break;
        }

        if (distance > 9)
        {
            currLevel++;
            break;
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    velocity = 0;
    move_x = 0;

    if (currLevel == 2)
    {
        game_floor_2.createVAO();
        game_ceiling_2.createVAO();
        bobby_2.createVAO();
        coin1b.createVAO(1);
        coin2b.createVAO(2);
        zapper1b.createVAO(1);
        zapper2b.createVAO(2);

        past = glfwGetTime();
        start = glfwGetTime();
        die = 0;

        while (!glfwWindowShouldClose(window))
        {
            processInput(window);
            ourShader.use();

            glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
            move_x -= 0.01;

            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            glm::mat4 trans = glm::mat4(1.0f);

            ourShader.use();
            glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));

            ourShader.setInt("Texture", 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture1);
            glUniform2f(transformbackground, move_x, 0.0f);
            glBindVertexArray(VAO_texture);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);

            glUniform1f(OpaqueLoc, 1.0f);
            glUniform3fv(BlurLoc, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f, 0.0f)));

            glUniform2f(transformbackground, 0.0f, 0.0f);

            char score[100];
            char level[100];
            sprintf(score, "Total Coins: %d", coins_collected);
            sprintf(level, "Current Level: %d", currLevel);
            string lev = level;
            string message = score;
            string target = "Target: 15";

            RenderText(shader, message, 20.0f, 570.0f, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f));
            RenderText(shader, lev, 320.0f, 15.0f, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f));
            RenderText(shader, target, 660.0f, 570.0f, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f));

            present = glfwGetTime();
            delta = present - past;
            velocity += gravity * delta;
            if (bobby_2.y > 0)
            {
                bobby_2.y -= velocity * delta * 0.4;
                bobby_2.abs_y -= velocity * delta * 0.4;
            }
            past = present;

            int distance = present - start;
            char dist[100];
            sprintf(dist, "Distance travelled: %d", distance);
            string curr_dist = dist;
            RenderText(shader, curr_dist, 400.0f, 570.0f, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f));

            ourShader.use();
            ourShader.setInt("Texture", 1);
            blur = glm::vec3(1, 1, 1);
            glUniform3fv(BlurLoc, 1, glm::value_ptr(blur));
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, texture2);
            bobby_2.draw(ourShader.ID);

            glUseProgram(shaderProgram);

            coin1b.draw(shaderProgram, coin1b.visible);
            coin1b.visible = 1;
            bool collected1 = game.coin_collision(bobby_2, coin1b);
            if (collected1)
            {
                coin1b.visible = 0;
                coins_collected++;
            }

            coin2b.draw(shaderProgram, coin2b.visible);
            coin2b.visible = 1;
            bool collected2 = game.coin_collision(bobby_2, coin2b);
            if (collected2)
            {
                coin2b.visible = 0;
                coins_collected++;
            }

            blur = glm::vec3(0);
            glUniform3fv(BlurLoc, 1, glm::value_ptr(blur));

            ourShader.use();
            ourShader.setInt("Texture", 2);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, texture3);
            zapper1b.draw(ourShader.ID);
            bool dead1 = game.zapper_collision(bobby_2, zapper1b);

            ourShader.use();
            ourShader.setInt("Texture", 2);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, texture3);
            zapper2b.draw(ourShader.ID);
            glUniform3fv(BlurLoc, 1, glm::value_ptr(glm::vec3(0.0f)));
            bool dead2 = game.zapper_collision(bobby_2, zapper2b);

            bool horizontal = true, first_iteration = true;
            unsigned int amount = 10;
            blurShader.use();
            for (unsigned int i = 0; i < amount; i++)
            {
                glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
                glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT);
                blurShader.setInt("image", 0);
                blurShader.setInt("horizontal", horizontal);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongColorbuffers[!horizontal]);
                glBindVertexArray(quadVAO);
                glDrawArrays(GL_TRIANGLES, 0, 6);
                glBindVertexArray(0);
                horizontal = !horizontal;
                if (first_iteration)
                    first_iteration = false;
            }

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glDisable(GL_DEPTH_TEST);
            glClear(GL_COLOR_BUFFER_BIT);
            HDRshader.use();
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);
            HDRshader.setInt("scene", 0);
            HDRshader.setInt("bloomBlur", 1);
            HDRshader.setInt("bloom", 1);
            HDRshader.setFloat("exposure", 3.0f);
            glBindVertexArray(quadVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);

            if (dead1)
            {
                currLevel = 5;
                break;
            }

            if (dead2)
            {
                currLevel = 5;
                break;
            }

            if (distance > 14)
            {
                currLevel++;
                break;
            }

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }

    velocity = 0;
    move_x -= 0;

    if (currLevel == 3)
    {
        game_floor_3.createVAO();
        game_ceiling_3.createVAO();
        bobby_3.createVAO();
        coin1c.createVAO(1);
        coin2c.createVAO(2);
        coin3c.createVAO(3);
        zapper1c.createVAO(1);
        zapper2c.createVAO(2);
        zapper3c.createVAO(3);

        past = glfwGetTime();
        start = glfwGetTime();
        die = 0;

        while (!glfwWindowShouldClose(window))
        {
            processInput(window);
            ourShader.use();

            glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
            move_x -= 0.01;

            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            glm::mat4 trans = glm::mat4(1.0f);

            ourShader.use();
            glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));

            ourShader.setInt("Texture", 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture1);
            glUniform2f(transformbackground, move_x, 0.0f);
            glBindVertexArray(VAO_texture);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);

            glUniform1f(OpaqueLoc, 1.0f);
            glUniform3fv(BlurLoc, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f, 0.0f)));

            glUniform2f(transformbackground, 0.0f, 0.0f);

            char score[100];
            char level[100];
            sprintf(score, "Total Coins: %d", coins_collected);
            sprintf(level, "Current Level: %d", currLevel);
            string lev = level;
            string message = score;
            string target = "Target: 20";

            RenderText(shader, message, 20.0f, 570.0f, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f));
            RenderText(shader, lev, 320.0f, 15.0f, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f));
            RenderText(shader, target, 660.0f, 570.0f, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f));

            present = glfwGetTime();
            delta = present - past;
            velocity += gravity * delta;
            if (bobby_3.y > 0)
            {
                bobby_3.y -= velocity * delta * 0.4;
                bobby_3.abs_y -= velocity * delta * 0.4;
            }
            past = present;

            int distance = present - start;
            char dist[100];
            sprintf(dist, "Distance travelled: %d", distance);
            string curr_dist = dist;
            RenderText(shader, curr_dist, 400.0f, 570.0f, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f));

            ourShader.use();
            ourShader.setInt("Texture", 1);
            blur = glm::vec3(1, 1, 1);
            glUniform3fv(BlurLoc, 1, glm::value_ptr(blur));
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, texture2);
            bobby_3.draw(ourShader.ID);

            glUseProgram(shaderProgram);

            coin1c.draw(shaderProgram, coin1c.visible);
            coin1c.visible = 1;
            bool collected1 = game.coin_collision(bobby_3, coin1c);
            if (collected1)
            {
                coin1c.visible = 0;
                coins_collected++;
            }

            coin2c.draw(shaderProgram, coin2c.visible);
            coin2c.visible = 1;
            bool collected2 = game.coin_collision(bobby_3, coin2c);
            if (collected2)
            {
                coin2c.visible = 0;
                coins_collected++;
            }

            coin3c.draw(shaderProgram, coin3c.visible);
            coin3c.visible = 1;
            bool collected3 = game.coin_collision(bobby_3, coin3c);
            if (collected3)
            {
                coin3c.visible = 0;
                coins_collected++;
            }

            blur = glm::vec3(0);
            glUniform3fv(BlurLoc, 1, glm::value_ptr(blur));

            ourShader.use();
            ourShader.setInt("Texture", 2);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, texture3);
            zapper1c.draw(ourShader.ID);
            bool dead1 = game.zapper_collision(bobby_3, zapper1c);

            ourShader.use();
            ourShader.setInt("Texture", 2);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, texture3);
            zapper2c.draw(ourShader.ID);
            bool dead2 = game.zapper_collision(bobby_3, zapper2c);

            ourShader.use();
            ourShader.setInt("Texture", 2);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, texture3);
            zapper3c.draw(ourShader.ID);
            glUniform3fv(BlurLoc, 1, glm::value_ptr(glm::vec3(0.0f)));
            bool dead3 = game.zapper_collision(bobby_3, zapper3c);

            bool horizontal = true, first_iteration = true;
            unsigned int amount = 10;
            blurShader.use();
            for (unsigned int i = 0; i < amount; i++)
            {
                glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
                glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT);
                blurShader.setInt("image", 0);
                blurShader.setInt("horizontal", horizontal);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongColorbuffers[!horizontal]);
                glBindVertexArray(quadVAO);
                glDrawArrays(GL_TRIANGLES, 0, 6);
                glBindVertexArray(0);
                horizontal = !horizontal;
                if (first_iteration)
                    first_iteration = false;
            }

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glDisable(GL_DEPTH_TEST);
            glClear(GL_COLOR_BUFFER_BIT);
            HDRshader.use();
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);
            HDRshader.setInt("scene", 0);
            HDRshader.setInt("bloomBlur", 1);
            HDRshader.setInt("bloom", 1);
            HDRshader.setFloat("exposure", 3.0f);
            glBindVertexArray(quadVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);

            if (dead1)
            {
                currLevel = 5;
                break;
            }

            if (dead2)
            {
                currLevel = 5;
                break;
            }

            if (dead3)
            {
                currLevel = 5;
                break;
            }

            if (distance > 19)
            {
                currLevel++;
                break;
            }

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }

    if (currLevel == 4)
    {
        while (!glfwWindowShouldClose(window))
        {
            processInput(window);

            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            char Final_score[100];

            sprintf(Final_score, "Your final score was: %d", coins_collected);

            string win = "CONGRATULATIONS! YOU WON";
            string Final = Final_score;
            string skill = "Why dont you try making some friends";

            RenderText(shader, win, 130.0f, 400.0f, 0.8f, glm::vec3(1.0f, 1.0f, 1.0f));
            RenderText(shader, Final, 250.0f, 350.0f, 0.6f, glm::vec3(1.0f, 1.0f, 1.0f));
            RenderText(shader, skill, 200.0f, 300.0f, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f));

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }

    if (currLevel == 5)
    {
        while (!glfwWindowShouldClose(window))
        {
            processInput(window);

            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            char Final_score[100];

            sprintf(Final_score, "Your final score was: %d", coins_collected);

            string loss = "GAME OVER. YOU LOSE";
            string Final = Final_score;
            string skill = "skill issue";

            RenderText(shader, loss, 170.0f, 400.0f, 0.8f, glm::vec3(1.0f, 1.0f, 1.0f));
            RenderText(shader, Final, 250.0f, 350.0f, 0.6f, glm::vec3(1.0f, 1.0f, 1.0f));
            RenderText(shader, skill, 350.0f, 300.0f, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f));

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }

    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
    else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        if (currLevel == 1)
            bobby_1.fly();
        else if (currLevel == 2)
            bobby_2.fly();
        else if (currLevel == 3)
            bobby_3.fly();
        velocity = 0;
    }
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void RenderText(Shader &shader, std::string text, float x, float y, float scale, glm::vec3 color)
{
    // activate corresponding render state
    shader.use();
    glUniform3f(glGetUniformLocation(shader.ID, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    // iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = Characters[*c];

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;
        // update VBO for each character
        float vertices[6][4] = {
            {xpos, ypos + h, 0.0f, 0.0f},
            {xpos, ypos, 0.0f, 1.0f},
            {xpos + w, ypos, 1.0f, 1.0f},

            {xpos, ypos + h, 0.0f, 0.0f},
            {xpos + w, ypos, 1.0f, 1.0f},
            {xpos + w, ypos + h, 1.0f, 0.0f}};
        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}
