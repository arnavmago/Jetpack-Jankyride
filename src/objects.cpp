#include "main.h"
#include "objects.h"
const float pi = 3.14159265;
int i = 1;

void Coin::createVAO(int setLevel)
{
    unsigned int VBO, EBO;
    float coin[9 * num];
    level = setLevel;
    size = 0.032;

    for (int i = 0; i < num; i++)
    {
        coin[9 * i + 0] = (0.032 * cos(i * 2 * pi / num));
        coin[9 * i + 1] = (0.032 * sin(i * 2 * pi / num));
        coin[9 * i + 2] = 0.0;

        coin[9 * i + 3] = (0.032 * cos((i + 1) * 2 * pi / num));
        coin[9 * i + 4] = (0.032 * sin((i + 1) * 2 * pi / num));
        coin[9 * i + 5] = 0.0;

        coin[9 * i + 6] = 0.0;
        coin[9 * i + 7] = 0.0;
        coin[9 * i + 8] = 0.0;
    }

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(coin), coin, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Coin::draw(unsigned int shaderProgram, int visible)
{
    x -= 0.01;

    random_device rd;
    mt19937 gen(rd() * level);
    uniform_real_distribution<> distr(-0.6, 0.6);

    if (!visible)
    {
        x = 1.1;
        y = distr(gen);
    }

    if (x < -1.1)
    {
        x = 1.1;
        y = distr(gen);
    }

    glUseProgram(shaderProgram);
    trans = glm::translate(trans, glm::vec3(x, y, 0));

    unsigned int transformLoc = glGetUniformLocation(shaderProgram, "transform");
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));

    trans = glm::mat4(1.0f);

    int vertexColorLocation = glGetUniformLocation(shaderProgram, "col");
    glUniform4f(vertexColorLocation, 1.0f, 0.843f, 0.0f, 1.0f);

    glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
    glDrawArrays(GL_TRIANGLES, 0, 3 * num);
    glBindVertexArray(0);
}

bool Game::coin_collision(Bobby player, Coin coin)
{
    bool collisionX = player.abs_x + player.size_x >= coin.x - coin.size && coin.x + coin.size >= player.abs_x - player.size_x;
    bool collisionY = player.abs_y + player.size_y >= coin.y - coin.size && coin.y + coin.size >= player.abs_y - player.size_y;
    return collisionX && collisionY;
}

bool Game::zapper_collision(Bobby player, Zapper zapper)
{
    float x1 = zapper.abs_x - (zapper.size_y * sin(zapper.rotation));
    float y1 = zapper.abs_y - (zapper.size_y * cos(zapper.rotation));
    float x2 = zapper.abs_x + (zapper.size_y * sin(zapper.rotation));
    float y2 = zapper.abs_y + (zapper.size_y * cos(zapper.rotation));

    float player_top = player.abs_y + player.size_y;
    float player_bottom = player.abs_y - player.size_y;
    float player_right = player.abs_x + player.size_x;
    float player_left = player.abs_x - player.size_x;

    bool left_collision = check_collision(x1, y1, x2, y2, player_left, player_top, player_left, player_bottom);
    bool right_collision = check_collision(x1, y1, x2, y2, player_right, player_top, player_right, player_bottom);
    bool top_collision = check_collision(x1, y1, x2, y2, player_left, player_top, player_right, player_top);
    bool bottom_collision = check_collision(x1, y1, x2, y2, player_left, player_bottom, player_right, player_bottom);

    if (left_collision || right_collision || top_collision || bottom_collision)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool check_collision(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4)
{
    float A = ((x4 - x3) * (y1 - y3) - (y4 - y3) * (x1 - x3)) / ((y4 - y3) * (x2 - x1) - (x4 - x3) * (y2 - y1));
    float B = ((x2 - x1) * (y1 - y3) - (y2 - y1) * (x1 - x3)) / ((y4 - y3) * (x2 - x1) - (x4 - x3) * (y2 - y1));

    if (A >= 0 && A <= 1 && B >= 0 && B <= 1)
    {
        return true;
    }

    return false;
}

void Zapper::createVAO(int setLevel)
{
    unsigned int VBO, EBO;
    level = setLevel;
    size_y = 0.15;
    size_x = 0.03;

    float zapper[] = {
        0.8f, -0.3f, 0.0f, 1.0f, 1.0f,
        0.8f, -0.6f, 0.0f, 1.0f, 0.0f,
        0.74f, -0.6f, 0.0f, 0.0f, 0.0f,
        0.74f, -0.3f, 0.0f, 0.0f, 1.0f};

    unsigned int zapper_indices[] = {
        0, 1, 2,
        0, 2, 3};

    abs_x = 0.77f;
    abs_y = -0.45f;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(zapper), zapper, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(zapper_indices), zapper_indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Zapper::draw(unsigned int shaderProgram)
{
    x -= 0.01;

    rotation += level * pi / 120;

    random_device rd;
    mt19937 gen(rd() * level);
    uniform_real_distribution<> distr(-0.1, 0.95);

    glUseProgram(shaderProgram);

    if (x < -1.8)
    {
        x = 0.5;
        y = distr(gen);
    }

    abs_x = 0.77f + x;
    abs_y = -0.45f + y;

    trans = glm::translate(trans, glm::vec3(abs_x, abs_y, 0));
    trans = glm::rotate(trans, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
    trans = glm::translate(trans, glm::vec3(-abs_x, -abs_y, 0));
    trans = glm::translate(trans, glm::vec3(x, y, 0));

    unsigned int transformLoc = glGetUniformLocation(shaderProgram, "transform");
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));

    trans = glm::mat4(1.0f);

    int vertexColorLocation = glGetUniformLocation(shaderProgram, "col");
    glUniform4f(vertexColorLocation, 1.0f, 0.0f, 0.0f, 1.0f);

    glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}