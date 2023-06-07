// Copyright (C) 2020 Emilio J. Padrón
// Released as Free Software under the X11 License
// https://spdx.org/licenses/X11.html

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>

// GLM library to deal with matrix operations
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>               // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::perspective
#include <glm/gtc/type_ptr.hpp>

#include "textfile_ALT.h"

int gl_width = 640;
int gl_height = 480;

void glfw_window_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
void render(double, GLuint *vaos[]);
void getAllNormals(GLfloat *normals, const GLfloat polygon[], const int size);
void calcPolygon(const GLfloat vertex_positions[], int index);

GLuint shader_program = 0; // shader program to set render pipeline

GLint view_location, projection_location, model_location, normal_matrix_location;
GLint lightPositionLocation, lightAmbientLocation, lightDiffuseLocation, lightSpecularLocation;
GLint lightPositionLocation2, lightAmbientLocation2, lightDiffuseLocation2, lightSpecularLocation2;
GLint materialAmbientLocation, materialDiffuseLocation, materialSpecularLocation, materialShininessLocation;
GLint viewPosLocation;

// Shader names
const char *vertexFileName = "spinningcube_withlight_vs.glsl";
const char *fragmentFileName = "spinningcube_withlight_fs.glsl";

// Camera
glm::vec3 camera_pos(0.0f, 0.0f, 2.0f);

// Lighting
struct Light
{
  glm::vec3 position;
  glm::vec3 ambient;
  glm::vec3 diffuse;
  glm::vec3 specular;
};

Light light = {
    glm::vec3(1.2f, 1.0f, 2.0f), // position
    glm::vec3(0.2f, 0.2f, 0.2f), // ambient
    glm::vec3(0.5f, 0.5f, 0.5f), // diffuse
    glm::vec3(1.0f, 1.0f, 1.0f)  // specular
};

Light light2 = {
    glm::vec3(1.2f, -1.0f, 2.0f), // position
    glm::vec3(0.2f, 0.2f, 0.2f),  // ambient
    glm::vec3(0.5f, 0.5f, 0.5f),  // diffuse
    glm::vec3(1.0f, 1.0f, 1.0f)   // specular
};

// Material
struct Material
{
  glm::vec3 ambient;
  glm::vec3 diffuse;
  glm::vec3 specular;
  float shininess;
};

Material material = {
    glm::vec3(1.0f, 0.5f, 0.31f), // ambient
    glm::vec3(1.0f, 0.5f, 0.31f), // diffuse
    glm::vec3(0.5f, 0.5f, 0.5f),  // specular
    32.0f                         // shininess
};

glm::vec3 translation(1.0f, 0.0f, 0.0f);

void calcPolygon(const GLfloat vertex_positions[], int size, GLuint *vao)
{
  int index = 0;

  // Vertex Array Object
  glGenVertexArrays(1, vao);
  glBindVertexArray(*vao);

  // Vertex Buffer Object (for vertex coordinates)
  GLuint vbo = 0;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * size, vertex_positions, GL_STATIC_DRAW);

  // Vertex attributes
  // 0: vertex position (x, y, z)
  glVertexAttribPointer(index, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  glEnableVertexAttribArray(index);

  // 1: vertex normals (x, y, z)
  GLfloat normals[sizeof(GLfloat) * size] = {};
  getAllNormals(normals, vertex_positions, size);

  GLuint normalsBuffer = 0;
  glGenBuffers(1, &normalsBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, normalsBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);

  glVertexAttribPointer(index + 1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  glEnableVertexAttribArray(index + 1);

  // Unbind vbo (it was conveniently registered by VertexAttribPointer)
  glBindBuffer(GL_ARRAY_BUFFER, index);
  glBindBuffer(GL_ARRAY_BUFFER, index + 1);

  // Unbind vao
  glBindVertexArray(0);
}

int main()
{
  // start GL context and O/S window using the GLFW helper library
  if (!glfwInit())
  {
    fprintf(stderr, "ERROR: could not start GLFW3\n");
    return 1;
  }

  GLFWwindow *window = glfwCreateWindow(gl_width, gl_height, "My spinning cube", NULL, NULL);
  if (!window)
  {
    fprintf(stderr, "ERROR: could not open window with GLFW3\n");
    glfwTerminate();
    return 1;
  }
  glfwSetWindowSizeCallback(window, glfw_window_size_callback);
  glfwMakeContextCurrent(window);

  // start GLEW extension handler
  // glewExperimental = GL_TRUE;
  glewInit();

  // get version info
  const GLubyte *vendor = glGetString(GL_VENDOR);                        // get vendor string
  const GLubyte *renderer = glGetString(GL_RENDERER);                    // get renderer string
  const GLubyte *glversion = glGetString(GL_VERSION);                    // version as a string
  const GLubyte *glslversion = glGetString(GL_SHADING_LANGUAGE_VERSION); // version as a string
  printf("Vendor: %s\n", vendor);
  printf("Renderer: %s\n", renderer);
  printf("OpenGL version supported %s\n", glversion);
  printf("GLSL version supported %s\n", glslversion);
  printf("Starting viewport: (width: %d, height: %d)\n", gl_width, gl_height);

  // Enable Depth test: only draw onto a pixel if fragment closer to viewer
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS); // set a smaller value as "closer"

  // Vertex Shader
  char *vertex_shader = textFileRead(vertexFileName);

  // Fragment Shader
  char *fragment_shader = textFileRead(fragmentFileName);

  // Shaders compilation
  GLuint vs = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vs, 1, &vertex_shader, NULL);
  free(vertex_shader);
  glCompileShader(vs);

  int success;
  char infoLog[512];
  glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
  if (!success)
  {
    glGetShaderInfoLog(vs, 512, NULL, infoLog);
    printf("ERROR: Vertex Shader compilation failed!\n%s\n", infoLog);

    return (1);
  }

  GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fs, 1, &fragment_shader, NULL);
  free(fragment_shader);
  glCompileShader(fs);

  glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
  if (!success)
  {
    glGetShaderInfoLog(fs, 512, NULL, infoLog);
    printf("ERROR: Fragment Shader compilation failed!\n%s\n", infoLog);

    return (1);
  }

  // Create program, attach shaders to it and link it
  shader_program = glCreateProgram();
  glAttachShader(shader_program, fs);
  glAttachShader(shader_program, vs);
  glLinkProgram(shader_program);

  glValidateProgram(shader_program);
  glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
  if (!success)
  {
    glGetProgramInfoLog(shader_program, 512, NULL, infoLog);
    printf("ERROR: Shader Program linking failed!\n%s\n", infoLog);

    return (1);
  }

  // Release shader objects
  glDeleteShader(vs);
  glDeleteShader(fs);

  // Cube to be rendered
  //
  //          0        3
  //       7        4 <-- top-right-near
  // bottom
  // left
  // far ---> 1        2
  //       6        5
  //
  const GLfloat vertex_positions_pyramid[] = {
      -0.0f, 0.289f, 0.0f,  // 1
      -0.5f, -0.289f, 0.0f, // 0
      0.5f, -0.289f, 0.0f,  // 2

      -0.0f, 0.289f, 0.0f,  // 1
      0.5f, -0.289f, 0.0f,  // 2
      0.0f, 0.00f, 0.8165f, // 4

      -0.5f, -0.289f, 0.0f, // 0
      0.5f, -0.289f, 0.0f,  // 2
      0.0f, 0.00f, 0.8165f, // 4

      -0.0f, 0.289f, 0.0f,  // 1
      -0.5f, -0.289f, 0.0f, // 0
      0.0f, 0.00f, 0.8165f, // 4

  };

  const GLfloat vertex_positions_cube[] = {
      -0.25f, -0.25f, -0.25f, // 1
      -0.25f, 0.25f, -0.25f,  // 0
      0.25f, -0.25f, -0.25f,  // 2

      0.25f, 0.25f, -0.25f,  // 3
      0.25f, -0.25f, -0.25f, // 2
      -0.25f, 0.25f, -0.25f, // 0

      0.25f, -0.25f, -0.25f, // 2
      0.25f, 0.25f, -0.25f,  // 3
      0.25f, -0.25f, 0.25f,  // 5

      0.25f, 0.25f, 0.25f,  // 4
      0.25f, -0.25f, 0.25f, // 5
      0.25f, 0.25f, -0.25f, // 3

      0.25f, -0.25f, 0.25f,  // 5
      0.25f, 0.25f, 0.25f,   // 4
      -0.25f, -0.25f, 0.25f, // 6

      -0.25f, 0.25f, 0.25f,  // 7
      -0.25f, -0.25f, 0.25f, // 6
      0.25f, 0.25f, 0.25f,   // 4

      -0.25f, -0.25f, 0.25f,  // 6
      -0.25f, 0.25f, 0.25f,   // 7
      -0.25f, -0.25f, -0.25f, // 1

      -0.25f, 0.25f, -0.25f,  // 0
      -0.25f, -0.25f, -0.25f, // 1
      -0.25f, 0.25f, 0.25f,   // 7

      0.25f, -0.25f, -0.25f,  // 2
      0.25f, -0.25f, 0.25f,   // 5
      -0.25f, -0.25f, -0.25f, // 1

      -0.25f, -0.25f, 0.25f,  // 6
      -0.25f, -0.25f, -0.25f, // 1
      0.25f, -0.25f, 0.25f,   // 5

      0.25f, 0.25f, 0.25f,  // 4
      0.25f, 0.25f, -0.25f, // 3
      -0.25f, 0.25f, 0.25f, // 7

      -0.25f, 0.25f, -0.25f, // 0
      -0.25f, 0.25f, 0.25f,  // 7
      0.25f, 0.25f, -0.25f   // 3
  };

  GLuint cubeVao;    // Vertext Array Object to set input data
  GLuint pyramidVao; // Vertext Array Object to set input data

  int vertexCount = sizeof(vertex_positions_pyramid) / sizeof(vertex_positions_pyramid[0]);

  calcPolygon(vertex_positions_pyramid, vertexCount, &pyramidVao);

  vertexCount = sizeof(vertex_positions_cube) / sizeof(vertex_positions_cube[0]);

  calcPolygon(vertex_positions_cube, vertexCount, &cubeVao);

  GLuint *vaos[] = {&pyramidVao, &cubeVao};

  // Uniforms
  // - Model matrix
  model_location = glGetUniformLocation(shader_program, "model");
  // - View matrix
  view_location = glGetUniformLocation(shader_program, "view");
  // - Projection matrix
  projection_location = glGetUniformLocation(shader_program, "projection");
  // - Normal matrix: normal vectors from local to world coordinates
  normal_matrix_location = glGetUniformLocation(shader_program, "normal_matrix");
  // - Camera position
  // - Light data
  lightPositionLocation = glGetUniformLocation(shader_program, "light.position");
  lightAmbientLocation = glGetUniformLocation(shader_program, "light.ambient");
  lightDiffuseLocation = glGetUniformLocation(shader_program, "light.diffuse");
  lightSpecularLocation = glGetUniformLocation(shader_program, "light.specular");

  lightPositionLocation2 = glGetUniformLocation(shader_program, "light2.position");
  lightAmbientLocation2 = glGetUniformLocation(shader_program, "light2.ambient");
  lightDiffuseLocation2 = glGetUniformLocation(shader_program, "light2.diffuse");
  lightSpecularLocation2 = glGetUniformLocation(shader_program, "light2.specular");
  // - Material data
  materialAmbientLocation = glGetUniformLocation(shader_program, "material.ambient");
  materialDiffuseLocation = glGetUniformLocation(shader_program, "material.diffuse");
  materialSpecularLocation = glGetUniformLocation(shader_program, "material.specular");
  materialShininessLocation = glGetUniformLocation(shader_program, "material.shininess");
  // [...]
  viewPosLocation = glGetUniformLocation(shader_program, "view_pos");

  // Render loop
  while (!glfwWindowShouldClose(window))
  {

    processInput(window);

    render(glfwGetTime(), vaos);

    glfwSwapBuffers(window);

    glfwPollEvents();
  }

  glfwTerminate();

  return 0;
}

void render(double currentTime, GLuint *vaos[])
{

  float f = (float)currentTime * 0.2f;

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glViewport(0, 0, gl_width, gl_height);

  glUseProgram(shader_program);
  glBindVertexArray(*vaos[0]);

  glm::mat4 model_matrix, view_matrix, proj_matrix;
  glm::mat3 normal_matrix;

  model_matrix = glm::mat4(1.f);

  // Camara
  view_matrix = glm::lookAt(camera_pos,                   // pos
                            glm::vec3(0.0f, 0.0f, 0.0f),  // target
                            glm::vec3(0.0f, 1.0f, 0.0f)); // up

  // Moving cube
  // Teniendo en cuenta el tiempo actual, se rota el cubo tanto horizontal
  // como verticalmente
  model_matrix = glm::rotate(model_matrix,
                             glm::radians((float)currentTime * 30.0f),
                             glm::vec3(0.0f, 1.0f, 0.0f));

  model_matrix = glm::rotate(model_matrix,
                             glm::radians((float)currentTime * 81.0f),
                             glm::vec3(1.0f, 0.0f, 0.0f));

  // Projection
  proj_matrix = glm::perspective(glm::radians(50.0f),
                                 (float)gl_width / (float)gl_height,
                                 0.1f, 1000.0f);

  glUniformMatrix4fv(view_location, 1, GL_FALSE, glm::value_ptr(view_matrix));
  glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(model_matrix));
  glUniformMatrix4fv(projection_location, 1, GL_FALSE, glm::value_ptr(proj_matrix));

  // Normal matrix: normal vectors to world coordinates
  normal_matrix = glm::transpose(glm::inverse(glm::mat3(model_matrix)));
  glUniformMatrix3fv(normal_matrix_location, 1, GL_FALSE, glm::value_ptr(normal_matrix));

  // Enviar los valores de light y material al programa de sombreado
  glUniform3fv(lightPositionLocation, 1, glm::value_ptr(light.position));
  glUniform3fv(lightAmbientLocation, 1, glm::value_ptr(light.ambient));
  glUniform3fv(lightDiffuseLocation, 1, glm::value_ptr(light.diffuse));
  glUniform3fv(lightSpecularLocation, 1, glm::value_ptr(light.specular));

  glUniform3fv(lightPositionLocation2, 1, glm::value_ptr(light2.position));
  glUniform3fv(lightAmbientLocation2, 1, glm::value_ptr(light2.ambient));
  glUniform3fv(lightDiffuseLocation2, 1, glm::value_ptr(light2.diffuse));
  glUniform3fv(lightSpecularLocation2, 1, glm::value_ptr(light2.specular));

  glUniform3fv(materialAmbientLocation, 1, glm::value_ptr(material.ambient));
  glUniform3fv(materialDiffuseLocation, 1, glm::value_ptr(material.diffuse));
  glUniform3fv(materialSpecularLocation, 1, glm::value_ptr(material.specular));
  glUniform1f(materialShininessLocation, material.shininess);

  glUniform3fv(viewPosLocation, 1, glm::value_ptr(camera_pos));

  glDrawArrays(GL_TRIANGLES, 0, 36);

  model_matrix = glm::translate(model_matrix, translation);

  glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(model_matrix));

  glBindVertexArray(*vaos[1]);

  glDrawArrays(GL_TRIANGLES, 0, 36);
}

void processInput(GLFWwindow *window)
{
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, 1);
}

// Callback function to track window size and update viewport
void glfw_window_size_callback(GLFWwindow *window, int width, int height)
{
  gl_width = width;
  gl_height = height;
  printf("New viewport: (width: %d, height: %d)\n", width, height);
}

// Obtenemos las normales de todo el poligono
// size es la longitud del array del poligono
void getAllNormals(GLfloat *normals, const GLfloat polygon[], const int size)
{
  printf("Tamaño: %d\n", size);
  for (int i = 0; i < size; i += 9)
  {

    GLfloat v1[] = {polygon[i], polygon[i + 1], polygon[i + 2]};
    GLfloat v2[] = {polygon[i + 3], polygon[i + 4], polygon[i + 5]};
    GLfloat v3[] = {polygon[i + 6], polygon[i + 7], polygon[i + 8]};

    GLfloat U[] = {v2[0] - v1[0], v2[1] - v1[1], v2[2] - v1[2]};
    GLfloat V[] = {v3[0] - v1[0], v3[1] - v1[1], v3[2] - v1[2]};

    GLfloat x = U[1] * V[2] - U[2] * V[1];
    GLfloat y = U[2] * V[0] - U[0] * V[2];
    GLfloat z = U[0] * V[1] - U[1] * V[0];

    normals[i] = x;
    normals[i + 3] = x;
    normals[i + 6] = x;

    normals[i + 1] = y;
    normals[i + 4] = y;
    normals[i + 7] = y;

    normals[i + 2] = z;
    normals[i + 5] = z;
    normals[i + 8] = z;
  }
}