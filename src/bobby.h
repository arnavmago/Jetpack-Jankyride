#include "main.h"

#ifndef BOBBY_H
#define BOBBY_H

class Bobby
{
public:
    unsigned int VAO;
    void createVAO();
    glm::mat4 trans;
    Bobby() { trans = glm::mat4(1.0f); }
    void fly();
    void draw(unsigned int shaderProgram);
    float abs_x;
    float abs_y;
    float y = 0;
    float size_x;
    float size_y;
};

#endif