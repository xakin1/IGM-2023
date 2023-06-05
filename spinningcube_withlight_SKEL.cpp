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
void render(double);

GLuint shader_program = 0; // shader program to set render pipeline
GLuint vao = 0;            // Vertext Array Object to set input data

GLint view_location, projection_location, model_location, normal_matrix_location;
GLint lightPositionLocation, lightAmbientLocation, lightDiffuseLocation, lightSpecularLocation;
GLint materialAmbientLocation, materialDiffuseLocation, materialSpecularLocation, materialShininessLocation;
GLint viewPosLocation;

// Shader names
const char *vertexFileName = "spinningcube_withlight_vs.glsl";
const char *fragmentFileName = "spinningcube_withlight_fs.glsl";

// Camera
glm::vec3 camera_pos(0.0f, 0.0f, 3.0f);

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

int main()
{
  // start GL context and O/S window using the GLFW helper library
  if (!glfwInit())
  {
    fprintf(stderr, "ERROR: could not start GLFW3\n");
    return 1;
  }

  //  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  //  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  //  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  //  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

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

  // Vertex Array Object
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  // Cube to be rendered
  //
  //          0        3
  //       7        4 <-- top-right-near
  // bottom
  // left
  // far ---> 1        2
  //       6        5
  //
  const GLfloat vertex_positions[] = {
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

  // Vertex Buffer Object (for vertex coordinates)
  GLuint vbo = 0;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_positions), vertex_positions, GL_STATIC_DRAW);

  // Vertex attributes
  // 0: vertex position (x, y, z)
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  glEnableVertexAttribArray(0);

  // 1: vertex normals (x, y, z)

  // Unbind vbo (it was conveniently registered by VertexAttribPointer)
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Unbind vao
  glBindVertexArray(0);

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

    render(glfwGetTime());

    glfwSwapBuffers(window);

    glfwPollEvents();
  }

  glfwTerminate();

  return 0;
}

void render(double currentTime)
{

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glViewport(0, 0, gl_width, gl_height);

  glUseProgram(shader_program);
  glBindVertexArray(vao);

  glm::mat4 model_matrix, normal_matrix, projection_matrix, view_matrix;

  // view_matrix = glm::lookAt(camera_pos,                   // pos
  //                           glm::vec3(0.0f, 0.0f, 0.0f),  // target
  //                           glm::vec3(0.0f, 1.0f, 0.0f)); // up
  // glUniformMatrix4fv(view_location, 1, GL_FALSE, glm::value_ptr(view_matrix));

  // Moving cube
  view_matrix = glm::mat4(1.0f);
  view_matrix = glm::translate(glm::mat4(1.f), glm::vec3(0.0f, 0.0f, -2.0f));
  view_matrix = glm::rotate(view_matrix,
                            glm::radians(45.0f),
                            glm::vec3(0.0f, 1.0f, 0.0f));
  view_matrix = glm::rotate(view_matrix,
                            glm::radians(45.0f),
                            glm::vec3(1.0f, 0.0f, 0.0f));

  glUniformMatrix4fv(view_location, 1, GL_FALSE, glm::value_ptr(view_matrix));
  //
  // Projection
  projection_matrix = glm::perspective(glm::radians(50.0f), (float)gl_width / (float)gl_height, 0.1f, 1000.0f);
  glUniformMatrix4fv(projection_location, 1, GL_FALSE, glm::value_ptr(projection_matrix));

  // Normal
  model_matrix = glm::mat4(1.0f);
  glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(model_matrix));

  normal_matrix = glm::transpose(glm::inverse(glm::mat3(model_matrix)));
  glUniformMatrix4fv(normal_matrix_location, 1, GL_FALSE, glm::value_ptr(normal_matrix));

  // view pos
  glm::vec3 viewPosValue(0, 0, 0);
  glUniform3fv(viewPosLocation, 1, glm::value_ptr(viewPosValue));

  // Enviar los valores de light y material al programa de sombreado
  glUniform3fv(lightPositionLocation, 1, glm::value_ptr(light.position));
  glUniform3fv(lightAmbientLocation, 1, glm::value_ptr(light.ambient));
  glUniform3fv(lightDiffuseLocation, 1, glm::value_ptr(light.diffuse));
  glUniform3fv(lightSpecularLocation, 1, glm::value_ptr(light.specular));

  glUniform3fv(materialAmbientLocation, 1, glm::value_ptr(material.ambient));
  glUniform3fv(materialDiffuseLocation, 1, glm::value_ptr(material.diffuse));
  glUniform3fv(materialSpecularLocation, 1, glm::value_ptr(material.specular));
  glUniform1f(materialShininessLocation, material.shininess);

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
