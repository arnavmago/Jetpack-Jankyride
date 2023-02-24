#include "main.h"
#include "bobby.h"
#include "objects.h"

void Bobby::fly()
{
    if (y < 1.01)
    {
        y += 0.03;
        abs_y += 0.03;
    }
}

void Bobby::createVAO()
{
    unsigned int VBO, EBO;

    float bobby[] = {
        -0.64f, -0.4f, 0.0f, 1.0f, 1.0f,
        -0.64f, -0.6f, 0.0f, 1.0f, 0.0f,
        -0.8f, -0.6f, 0.0f, 0.0f, 0.0f,
        -0.8f, -0.4f, 0.0f, 0.0f, 1.0f};

    abs_x = -0.72;
    abs_y = -0.5;
    size_x = 0.08;
    size_y = 0.1;

    unsigned int bobby_indices[] = {
        0, 1, 3,
        1, 2, 3};

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(bobby), bobby, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(bobby_indices), bobby_indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

void Bobby::draw(unsigned int shaderProgram)
{
    glUseProgram(shaderProgram);
    trans = glm::translate(trans, glm::vec3(0, y, 0));

    unsigned int transformLoc = glGetUniformLocation(shaderProgram, "transform");
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));

    trans = glm::mat4(1.0f);

    glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}