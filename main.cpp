#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <iostream>
#include "my_math.h"
#include <vector>
#include <string>
#define GLAD_GL_IMPLEMENTATION
#include <glad/glad.h> 
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

const float L = 7;
const float A = 2;
const float B = A / 2;
static bool odwrotna = true;
static bool grappled = false;
static float step = 0.1f;

class box {
public:
    float x, y, z;
    float size;
    //północ - dodatnie x
    //południe - ujemne x
    //wschód - dodatnie z
    //zachód - ujemne z

    box(float x = 2.f * A + B, float y = B, float z = 0.f, float s = A)
        :x(x), y(y), z(z), size(s) {}

    lib::Vec3 getcoordinates() {
        lib::Vec3 a = { x, y, z };
        return a;
    }

    float getx() {
        return x;
    }
    float gety() {
        return y;
    }
    float getz() {
        return z;
    }

    void follow_hand(float handx, float handy, float handz) {
        x = handx;
        y = handy - (B + step);
        z = handz;
    } 

    int check_collision(float handx, float handy, float handz) {
        //0 - brak kolizji, 1 - północ, 2- południe, 3 - wschód, 4 - zachód, 5 - górna ściana, 6 - możliwa interakcja
        if (fabs(handx - x) <= B + (step) && fabs(handy - y) <= B && fabs(handz - z) <= B) {
            if (handx >= x and !grappled) return 7;
            else if (handx < x and !grappled) return 2;
        }
        else if (fabs(handx - x) <= B && fabs(handy - y) <= B && fabs(handz - z) <= B + (step)) {
            if (handz >= z and !grappled) return 3;
            else if (handz < z and !grappled) return 4;
        }
        else if (fabs(handx - x) <= B && fabs(handy - y) <= B + (step) && fabs(handz - z) <= B) {
            if(!grappled)return 5;
        }
        else if (fabs(handx - x) <= B && fabs(handy - y) <= 2*B && fabs(handz - z) <= B) {
            return 6;
        }
        else return 0;

    }
    void update_gravity() {
        if (y > B and !grappled) {
            y-=0.05f;
        }
    }

};

struct ArmState {
    float alpha; // obrót wokół osi y
    float beta;  //obrót wokól osi x
    float gamma; // obrót wokół osi z 
};

struct Robot {
    float x;
    float y;
    float z;
    lib::Vec3 rotationAxis;
    ArmState arm1, arm2;
};
struct CamPoint {
    float x;
    float y;
    float z;
    lib::Vec3 rotationAxis;
    float r;
    float theta;
    float psi;
};

static ArmState arm1 = { 0.0f, 0.0f, -0.5f };
static ArmState arm2 = { 0.0f, PI32 / 2 , 0.0f };
static Robot hand = { 0.1f,    3.f,     0.1f, {0.f, 0.f, 0.f}, arm1, arm2 };
static CamPoint cam = { 0.620777f,     4.79426f,     8.75384f,  {0.f, 0.f, 0.f}, 10.0f, 0.5f, 1.5f };
static box box1;
static GLuint floor_vao, floor_vbo, floor_ebo;
static GLuint box_vao, box_vbo, box_ebo;

//to jest stworzone zeby mozna bylo dane od kuli wpisywac
GLuint sphere_vao, sphere_vbo, sphere_ebo, sphere1_vao, sphere1_vbo, sphere1_ebo;
std::vector<GLuint> sphereIndices, sphere1Indices;
//jak bys rozkminiał oznaczenia to "sphere" to ta kula na przegubie a "sphere1" to ta kulka zaczepiona do ziemii

struct Vertex
{
    lib::Vec3 pos;
    lib::Vec3 col;
    lib::Vec3 nor;

};

//struktura do pozycji i koloru dla kulki, w sumie to chyba mozna uzyc tej wyzej ale to czysta kosemtyka na pozniej
struct SphereVertex {
    lib::Vec3 pos;
    lib::Vec3 col;

};

static const Vertex vertices[] = {

    //POZYCJA                   KOLOR              WEKTOR NORMALNY          TEKSTURA ?


    //tylna sciana
    { {-0.2f, 0.f, -0.2f},  {0.1f, 0.1f, 0.1f}, {-0.2f, 0.0f, -1.0f} }, // 0 lewy dół tył
    { {-0.2f, L, -0.2f},   {0.2f, 0.2f, 0.2f}, { -0.2f, L, -1.0f }}, // 1 lewa góra tył
    { {0.2f, L, -0.2f},    {0.3f, 0.3f, 0.3f}, {0.2f, L, -1.0f} }, // 2 prawa góra tył
    { {0.2f, 0.f, -0.2f},   {0.4f, 0.4f, 0.4f}, {0.2f, 0.0f, -1.0f} }, // 3 prawy dół tył

    //przednia sciana
    { {-0.2f, 0.f, 0.2f},   {0.1f, 0.1f, 0.1f}, {-0.2f, 0.0f, 1.0f} }, // 4 lewy dół przód
    { {-0.2f, L, 0.2f},    {0.2f, 0.2f, 0.2f}, { -0.2f, L, 1.0f } }, // 5 lewa góra przód
    { { 0.2f, L, 0.2f},    {0.3f, 0.3f, 0.3f}, {0.2f, L, 1.0f} }, // 6 prawa góra przód
    { { 0.2f, 0.f, 0.2f},   {0.4f, 0.4f, 0.4f}, {0.2f, 0.0f, 1.0f} },  // 7 prawy dół przód

    //podstawa dol
    { { -0.2f, 0.f, -0.2f },  {0.1f, 0.1f, 0.1f}, {-0.2f, -1.0f, -0.2f} }, // 0 lewy dół tył
    { {0.2f, 0.f, -0.2f},   {0.4f, 0.4f, 0.4f}, {0.2f, -1.0f, -0.2f} }, // 3 prawy dół tył
    { {-0.2f, 0.f, 0.2f},   {0.1f, 0.1f, 0.1f}, {-0.2f, -1.0f, 0.2f} }, // 4 lewy dół przód
    { { 0.2f, 0.f, 0.2f},   {0.4f, 0.4f, 0.4f}, {0.2f, -1.0f, 0.2f} },  // 7 prawy dół przód

    //podstawa gora 
     { { -0.2f, L, -0.2f }, {0.2f, 0.2f, 0.2f}, {-0.2f, 1.0f, -0.2f}  }, // 1 lewa góra tył
     { {0.2f, L, -0.2f},    {0.3f, 0.3f, 0.3f}, {0.2f, 1.0f, -0.2f} }, // 2 prawa góra tył
     { {-0.2f, L, 0.2f},    {0.2f, 0.2f, 0.2f}, {-0.2f, 1.0f, 0.2f} }, // 5 lewa góra przód
     { { 0.2f, L, 0.2f},    {0.3f, 0.3f, 0.3f}, {0.2f, 1.0f, 0.2f} } // 6 prawa góra przód





};

static const Vertex vertices2[] = {
    { {-B, -B, -B},  {0.0f, 0.2f, 0.5f} }, // 0 lewy dół tył
    { {-B, B, -B},   {0.0f, 0.4f, 0.7f} }, // 1 lewa góra tył
    { {B, B, -B},    {0.0f, 0.6f, 0.8f} }, // 2 prawa góra tył
    { {B, -B, -B},   {0.0f, 0.3f, 0.6f} }, // 3 prawy dół tył
    { {-B, -B, B},   {0.0f, 0.2f, 0.5f} }, // 4 lewy dół przód
    { {-B, B, B},    {0.0f, 0.4f, 0.7f} }, // 5 lewa góra przód
    { { B, B, B},    {0.0f, 0.6f, 0.8f} }, // 6 prawa góra przód
    { { B, -B, B},   {0.0f, 0.3f, 0.6f} }  // 7 prawy dół przód
};

static const Vertex floor_points[] = {
    {{-40.0f, 0.f, -40.f}, { 0.18f, 0.35f, 0.85f } },
    {{-40.0f, 0.f, 40.f}, { 0.23f, 0.55f, 0.95f } },
    {{40.0f, 0.f, -40.f}, { 0.3f, 0.85f, 0.95f } },
    {{40.0f, 0.f, 40.f}, { 0.15f, 0.15f, 0.85f } }
};

static const GLuint indices[] = {
                          0, 1, 2,
                          0, 2, 3,
                          4, 6, 5,
                          4, 7, 6,
                          4, 5, 1,
                          4, 1, 0,
                          3, 2, 6,
                          3, 6, 7,
                          1, 5, 6,
                          1, 6, 2,
                          4, 0, 3,
                          4, 3, 7
};
static const GLuint indices2[] = {
0,1,2,2,1,3
};


//funkcja do generowania tych smiesznych wierzcholkow i indeksow zeby nadac ksztalt a potem wypelnic kulke
//x,y,z i kolor podobnie jak tamte inne vertexy ale wrzucane do wektora bo imo latwiej wrzucac w wektor niz sie bawic w tablice
//sectory i stacki to do okreslenia jak dokladnie ma to byc przyblizone do kuli tez jest w tutorialu

void generateSphere(std::vector<SphereVertex>& vertices, std::vector<GLuint>& indices, float radius, int sectors, int stacks) {

    //ustawienia ograniczen dla kata, ustawienie parametrów, do tego jest tutorial na jakiejs stronce moge podrzucic

    float const PI = 3.141592f;
    float const sectorStep = 2 * PI / sectors;
    float const stackStep = PI / stacks;
    float const lengthInv = 1.0f / radius;

    //tu ze wsp. sferycznych sa wyrzucane atrybuty do wektora dla wierzchołków czyli pozycja i kolor

    for (int i = 0; i <= stacks; ++i) {
        float const stackAngle = PI / 2 - i * stackStep;
        float const xy = radius * cosf(stackAngle);
        float const z = radius * sinf(stackAngle);

        for (int j = 0; j <= sectors; ++j) {
            float const sectorAngle = j * sectorStep;

            SphereVertex vertex;
            vertex.pos.x = xy * cosf(sectorAngle);
            vertex.pos.y = xy * sinf(sectorAngle);
            vertex.pos.z = z;
            vertex.col = { 0.05f, 0.05f, 0.05f };

            vertices.push_back(vertex);
        }
    }

    //tutaj są generowane indeksy żeby to optymalnie narysować z tego zlozenia dwoch trojkatow iteracyjnie po calej kulce
    //dobrze to widac na rysunku 

    for (int i = 0; i < stacks; ++i) {
        int k1 = i * (sectors + 1);
        int k2 = k1 + sectors + 1;

        for (int j = 0; j < sectors; ++j, ++k1, ++k2) {
            if (i != 0) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            if (i != (stacks - 1)) {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }
}



static const char* vertex_shader_text =
"#version 410 core\n"
"uniform mat4 Model;\n"
"layout (std140) uniform Matrices\n"
"{\n"
"    mat4 Proj;\n"
"    mat4 View;\n"
"};\n"
"uniform float time;\n"
"layout(location = 0) in vec3 vPos;\n"
"layout(location = 1) in vec3 vCol;\n"
"out vec3 color;\n"
"void main()\n"
"{\n"
"    gl_Position = Proj * View * Model *  vec4(vPos, 1.0);\n"
"    color = vCol;\n"
"}\n";

static const char* fragment_shader_text =
"#version 410\n"
"in vec3 color;\n"
"out vec4 fragment;\n"
"void main()\n"
"{\n"
"    fragment = vec4(color, 1.0);\n"
"}\n";

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void CalculateArmsOdwrotna() {
    arm1.alpha = -atan2f(hand.z, hand.x);
    arm1.gamma = -atan2f((sqrtf((hand.x) * (hand.x) + (hand.z) * (hand.z))), hand.y) + acosf(sqrtf((hand.x) * (hand.x) + (hand.y) * (hand.y) + (hand.z) * (hand.z)) / (2 * L));
    arm2.gamma = -2 * acosf(sqrtf((hand.x) * (hand.x) + (hand.y) * (hand.y) + (hand.z) * (hand.z)) / (2 * L));
    hand.rotationAxis = { sinf(arm1.alpha), 0.f , cosf(arm1.alpha) };
}
static void CalculateArmsNieOdwrotna() {
    hand.rotationAxis = { sinf(arm1.alpha), 0.f , cosf(arm1.alpha) };
}
static void CalculateCam() {
    cam.x = cam.r * cosf(cam.theta) * cosf(cam.psi);
    cam.z = cam.r * cosf(cam.theta) * sinf(cam.psi);
    cam.y = cam.r * sinf(cam.theta);
}
static void handle_input(int key) {
    float arm_length = sqrtf(powf(hand.x, 2) + powf(hand.y, 2) + powf(hand.z, 2));

        switch (key) {
        case GLFW_KEY_1: odwrotna = !odwrotna; break;

        case GLFW_KEY_A: if (odwrotna and (arm_length < 2 * L - 0.05f or sqrtf(powf(hand.x - 0.1f, 2) + powf(hand.y, 2) + powf(hand.z, 2)) < arm_length) and box1.check_collision(hand.x, hand.y, hand.z) != 7) hand.x -= 0.1f;
                       else arm1.alpha -= 0.1f;  break;
        case GLFW_KEY_D: if (odwrotna and (arm_length < 2 * L - 0.05f or sqrtf(powf(hand.x + 0.1f, 2) + powf(hand.y, 2) + powf(hand.z, 2)) < arm_length) and box1.check_collision(hand.x, hand.y, hand.z) != 2) hand.x += 0.1f;
                       else arm1.alpha += 0.1f; break;
        case GLFW_KEY_S: if (odwrotna and (arm_length < 2 * L - 0.05f or sqrtf(powf(hand.x, 2) + powf(hand.y, 2) + powf(hand.z + 0.1f, 2)) < arm_length) and box1.check_collision(hand.x, hand.y, hand.z) != 4) hand.z += 0.1f;
                       else if (arm1.gamma > (-0.5f * arm2.gamma - 1.5f))arm1.gamma -= 0.1f; break;
        case GLFW_KEY_W: if (odwrotna and (arm_length < 2 * L - 0.05f or sqrtf(powf(hand.x, 2) + powf(hand.y, 2) + powf(hand.z - 0.1f, 2)) < arm_length) and box1.check_collision(hand.x, hand.y, hand.z) != 3) hand.z -= 0.1f;
                       else if (arm1.gamma < 1.2f) arm1.gamma += 0.1f; break;
        case GLFW_KEY_Q: if (odwrotna and hand.y > 0.1f and !grappled and box1.check_collision(hand.x, hand.y, hand.z) != 5)hand.y -= 0.1f;
                       else if (odwrotna and hand.y > A and grappled and box1.check_collision(hand.x, hand.y, hand.z) != 5)hand.y -= 0.1f;
                       else if (arm2.gamma > (-2.f * arm1.gamma - 2.4f)) arm2.gamma -= 0.1f; break;
        case GLFW_KEY_E: if (odwrotna and (arm_length < 2 * L - 0.05f or sqrtf(powf(hand.x, 2) + powf(hand.y + 0.1f, 2) + powf(hand.z, 2)) < arm_length)) hand.y += 0.1f;
                       else if (arm2.gamma < 0.f) arm2.gamma += 0.1f; break;
        case GLFW_KEY_U: if (cam.theta > 0.2f)cam.theta -= 0.1f;  break;
        case GLFW_KEY_O: if (cam.theta < 1.5f) cam.theta += 0.1f; break;
        case GLFW_KEY_L: cam.psi += 0.1f; break;
        case GLFW_KEY_J: cam.psi -= 0.1f; break;
        case GLFW_KEY_I: if (cam.r > 5.f) cam.r -= 1.0f; break;
        case GLFW_KEY_K: if (cam.r < 100.f) cam.r += 1.0f; break;
        case GLFW_KEY_G: if (box1.check_collision(hand.x, hand.y, hand.z) == 6) { grappled = true; }
                       else { grappled = false; } break;
        }
    
    

    if (grappled)box1.follow_hand(hand.x, hand.y, hand.z);
    if (odwrotna)CalculateArmsOdwrotna();
    else if (!odwrotna)CalculateArmsNieOdwrotna();
    CalculateCam();
    std::cout << box1.check_collision(hand.x, hand.y, hand.z) << "    " << grappled << std::endl;
    //std::cout << arm1.alpha << "    " << arm1.gamma << "     " << arm2.gamma << "     " << std::endl;
    //std::cout << hand.x << "    "<< hand.y << "     " << hand.z << "     " << std::endl;
    //std::cout << fabs(box1.x - hand.x) << "    " << fabs(box1.y - hand.y) << "     " << fabs(box1.z - hand.z) << "     " << std::endl;
    //std::cout << cam.x << "     " << cam.y << "     " << cam.z << "     " << std::endl;
    //theta <0.1 , 1.5> r < 5, 100> psi <-inf, inf> -0.25625     4.79426     8.77208
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    float arm_length = sqrtf(powf(hand.x, 2) + powf(hand.y, 2) + powf(hand.z, 2));

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        handle_input(key);
    }
}

GLuint create_shader_program() {
    const GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
    glCompileShader(vertex_shader);

    const GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
    glCompileShader(fragment_shader);

    const GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return program;
}

void setup_buffers(GLuint& vertex_array, GLuint& vertex_buffer, GLuint& EBO) {
    glGenVertexArrays(1, &vertex_array);
    glBindVertexArray(vertex_array);

    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, col));

    glBindVertexArray(0);
}

void setup_buffers2(GLuint& vertex_array, GLuint& vertex_buffer, GLuint& EBO) {
    glGenVertexArrays(1, &vertex_array);
    glBindVertexArray(vertex_array);

    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, col));

    glBindVertexArray(0);
}

void setup_floor_buffers(GLuint& vertex_array, GLuint& vertex_buffer, GLuint& EBO) {
    glGenVertexArrays(1, &vertex_array);
    glBindVertexArray(vertex_array);

    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floor_points), floor_points, GL_STATIC_DRAW);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices2), indices2, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, col));

    glBindVertexArray(0);
}

//funkcja do ustawiania buforow dla kulek

void setupSphereBuffers(GLuint& vertex_array, GLuint& vertex_buffer, GLuint& EBO, const std::vector<SphereVertex>& vertices, const std::vector<GLuint>& indices) {
    glGenVertexArrays(1, &vertex_array);
    glBindVertexArray(vertex_array);
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(SphereVertex), vertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    // tu sie ustawiaja atrybuty dla wierzcholkow
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SphereVertex), (void*)offsetof(SphereVertex, pos));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(SphereVertex), (void*)offsetof(SphereVertex, col));

    glBindVertexArray(0);
}

//funckja do faktycznego tworzenia tych kulek

void renderSphere(GLuint vao, GLuint indicesSize, const lib::Mat4& model, GLuint program) {
    glBindVertexArray(vao);

    // Tu sie pobiera pozycja do uniforma co jest w shaderze
    GLint mvp_location = glGetUniformLocation(program, "Model");
    glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*)&model);

    glDrawElements(GL_TRIANGLES, indicesSize, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}


void render_floor(GLuint program, GLuint vertex_array, GLuint EBO, GLuint uboMatrices, const lib::Mat4& projection, const lib::Mat4& view, const lib::Mat4& model) {
    glUseProgram(program);

    glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(lib::Mat4), &projection);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(lib::Mat4), sizeof(lib::Mat4), &view);

    GLint mvp_location = glGetUniformLocation(program, "Model");
    glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*)&model);

    glBindVertexArray(vertex_array);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glDrawElements(GL_TRIANGLES, sizeof(indices2) / sizeof(indices2[0]), GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
}

void setup_uniforms(GLuint program, GLuint& uboMatrices) {
    GLuint Matrices_binding = 0;
    GLint matricesIndex = glGetUniformBlockIndex(program, "Matrices");
    glUniformBlockBinding(program, matricesIndex, Matrices_binding);

    glGenBuffers(1, &uboMatrices);
    glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(lib::Mat4), NULL, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, Matrices_binding, uboMatrices);
}

void render_cube(GLuint program, GLuint vertex_array, GLuint EBO, GLuint uboMatrices, const lib::Mat4& projection, const lib::Mat4& view, const lib::Mat4& model) {
    glUseProgram(program);

    glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(lib::Mat4), &projection);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(lib::Mat4), sizeof(lib::Mat4), &view);

    GLint mvp_location = glGetUniformLocation(program, "Model");
    glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*)&model);

    glBindVertexArray(vertex_array);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(indices[0]), GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
}


void render_box(GLuint program, GLuint vertex_array, GLuint EBO, GLuint uboMatrices, const lib::Mat4& projection, const lib::Mat4& view, const lib::Mat4& model) {
    glUseProgram(program);

    glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(lib::Mat4), &projection);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(lib::Mat4), sizeof(lib::Mat4), &view);

    GLint mvp_location = glGetUniformLocation(program, "Model");
    glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*)&model);

    glBindVertexArray(vertex_array);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(indices[0]), GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
}

void render_scene(GLuint program, GLuint vertex_array, GLuint EBO, GLuint uboMatrices, const lib::Mat4& projection, const lib::Mat4& view, const ArmState& cube1, const ArmState& cube2) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    lib::Mat4 followalpha = lib::create_rotation_y(arm1.alpha);
    lib::Mat4 followgamma1 = lib::create_rotation(hand.rotationAxis, arm1.gamma);
    lib::Mat4 followgamma2 = lib::create_rotation(hand.rotationAxis, arm2.gamma);
    lib::Mat4 model1 = followgamma1 * followalpha;
    render_cube(program, vertex_array, EBO, uboMatrices, projection, view, model1);

    lib::Mat4 translate = lib::create_translate({ 0.0f, L, 0.0f });
    lib::Mat4 inv_trans = lib::create_translate({ 0.0f, -L, 0.0f });
    lib::Mat4 model2 = followgamma1 * translate * followgamma2 * followalpha;
    render_cube(program, vertex_array, EBO, uboMatrices, projection, view, model2);

    lib::Mat4 box_translate = lib::create_translate({ box1.x, box1.y, box1.z});
    render_box(program, box_vao, box_ebo, uboMatrices, projection, view, box_translate);

    lib::Mat4 floor_model = lib::create_diagonal_matrix();
    render_floor(program, floor_vao, floor_ebo, uboMatrices, projection, view, floor_model);

    //Tu sie pokazują te kulki na scenie
    //Macierze to jest poskładanie translacji i odpowiednich obrotów, zeby to sie faktycznie obracało jak łożysko

    lib::Mat4 sphere_translate = lib::create_translate({ 0.0f, L , 0.0f });
    lib::Mat4 sphere_model = followgamma1 * sphere_translate * followgamma2 * followalpha;
    renderSphere(sphere_vao, sphereIndices.size(), sphere_model, program);

    lib::Mat4 sphere1_translate = lib::create_translate({ 0.0f, 0.0f, 0.0f });
    lib::Mat4 sphere1_model = followalpha * sphere1_translate * followgamma1;

    renderSphere(sphere1_vao, sphere1Indices.size(), sphere1_model, program);

}
int main(void)
{
    CalculateArmsOdwrotna();
    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1200, 1200, "Manipulator robota PUMA", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glfwSwapInterval(1);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_FRAMEBUFFER_SRGB);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);

    GLuint program = create_shader_program();
    GLuint vertex_array, vertex_buffer, EBO;
    std::vector<SphereVertex> sphereVertices;
    std::vector<SphereVertex> sphere1Vertices;
    setup_buffers(vertex_array, vertex_buffer, EBO);
    setup_floor_buffers(floor_vao, floor_vbo, floor_ebo);
    setup_buffers2(box_vao, box_vbo, box_ebo);
    generateSphere(sphereVertices, sphereIndices, 0.58f, 36, 18); // Example parameters: radius = 1.0f, sectors = 36, stacks = 18
    setupSphereBuffers(sphere_vao, sphere_vbo, sphere_ebo, sphereVertices, sphereIndices);
    generateSphere(sphere1Vertices, sphere1Indices, 0.85f, 36, 18);
    setupSphereBuffers(sphere1_vao, sphere1_vbo, sphere1_ebo, sphere1Vertices, sphere1Indices);


    GLuint uboMatrices;
    setup_uniforms(program, uboMatrices);

    glClearColor(0.9f, 0.9f, 0.9f, 1.0f);

    while (!glfwWindowShouldClose(window))
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        const float ratio = (float)width / (float)height;

        glViewport(0, 0, width, height);

        float time = (float)glfwGetTime();
        box1.update_gravity();
        //std::cout << time << std::endl;
        lib::Vec3 camera_pos = { cam.x, cam.y, cam.z };
        lib::Vec3 camera_target = { 0.0f, 1.0f, 0.0f };
        lib::Mat4 view = lib::create_look_at(camera_pos, camera_target, { 0.0f, L / 2.f, 0.0f });
        lib::Mat4 projection = lib::create_perspective(lib::deg_to_rad(50.0f), (f32)width / height, 0.5f, 100.0f);

        render_scene(program, vertex_array, EBO, uboMatrices, projection, view, arm1, arm2);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
