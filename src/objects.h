#include "main.h"
#include "bobby.h"

#ifndef OBJECTS_H
#define OBJECTS_H

class Coin
{
public:
    unsigned int VAO;
    unsigned int VBO;
    void createVAO(int setLevel);
    glm::mat4 trans;
    Coin() { trans = glm::mat4(1.0f); }
    void draw(unsigned int shaderProgram, int visible);
    int num = 100;
    float x = 0;
    float y = 0;
    float size;
    int visible = 1;
    int level;
};

class Zapper
{
public:
    unsigned int VAO;
    void createVAO(int setLevel);
    glm::mat4 trans;
    Zapper() { trans = glm::mat4(1.0f); }
    void draw(unsigned int shaderProgram);
    float x = 0;
    float y = 0;
    float abs_x;
    float abs_y;
    float size_x;
    float size_y;
    float rotation = 0;
    int level;
};

class Game
{
public:
    bool coin_collision(Bobby player, Coin coin);
    bool zapper_collision(Bobby player, Zapper zapper);
    void game_over(GLFWwindow *window);
};

bool check_collision(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4);

#endif