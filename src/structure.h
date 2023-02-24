#include "main.h"

#ifndef STRUCTURE_H
#define STRUCTURE_H

class Floor
{
public:
    unsigned int VAO;
    void createVAO();
    glm::mat4 trans;
    Floor() { trans = glm::mat4(1.0f); }
    void draw(unsigned int shaderProgram);
};

class Ceiling
{
public:
    unsigned int VAO;
    void createVAO();
    glm::mat4 trans;
    Ceiling() { trans = glm::mat4(1.0f); }
    void draw(unsigned int shaderProgram);
};

#endif