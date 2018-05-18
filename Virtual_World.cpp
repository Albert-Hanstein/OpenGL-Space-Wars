/*
   This is a variation of tutorial3 using a single VBO for specifying the vertex
   attribute data; it is done by setting the VertexAttribPointer parameters
   "stride" and "pointer" to suitable values.
   In particular for the pointer parameter, macro "offsetof" should be used so to
   avoid problem with alignment and padding for different architecture.

   Modified to use GLM

   By consultit@katamail.com

 */
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stddef.h> /* must include for the offsetof macro */
/*
 *
 * Include files for Windows, Linux and OSX
 * __APPLE is defined if OSX, otherwise Windows and Linux.
 *
 */

#ifdef __APPLE__
#define GLFW_INCLUDE_GLCOREARB 1
#include <GLFW/glfw3.h>
#else
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#endif

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <string>
#include <stdlib.h>
#include <math.h>

// #define STB_IMAGE_IMPLEMENTATION
// #include "include/stb_image.h"
#define STB_PERLIN_IMPLEMENTATION
#include "include/stb_perlin.h"
#include "shaderClass.h"
#include "importObj.h"

using namespace std;

void Check(const char *where) { // Function to check OpenGL error status
  const char * what;
  int err = glGetError();   //0 means no error
  if(!err)
    return;
  if(err == GL_INVALID_ENUM)
    what = "GL_INVALID_ENUM";
  else if(err == GL_INVALID_VALUE)
    what = "GL_INVALID_VALUE";
  else if(err == GL_INVALID_OPERATION)
    what = "GL_INVALID_OPERATION";
  else if(err == GL_INVALID_FRAMEBUFFER_OPERATION)
    what = "GL_INVALID_FRAMEBUFFER_OPERATION";
  else if(err == GL_OUT_OF_MEMORY)
    what = "GL_OUT_OF_MEMORY";
  else
    what = "Unknown Error";
  fprintf(stderr, "Error (%d) %s  at %s\n", err, what, where);
  exit(1);
}

void CheckShader(int sp, const char *x){
  int length;
  char text[1001];
  glGetProgramInfoLog(sp, 1000, &length, text);   // Check for errors
  if(length > 0) {
    fprintf(stderr, "Validate Shader Program\nMessage from:%s\n%s\n", x, text );
    exit(1);
  }
}

char* filetobuf(char *file) { /* A simple function that will read a file into an allocated char pointer buffer */
  FILE *fptr;
  long length;
  char *buf;
  fprintf(stderr, "Loading %s\n", file);
        #pragma warning (disable : 4996)
  fptr = fopen(file, "rb");   /* Open file for reading */
  if (!fptr) {   /* Return NULL on failure */
    fprintf(stderr, "failed to open %s\n", file);
    return NULL;
  }
  fseek(fptr, 0, SEEK_END);   /* Seek to the end of the file */
  length = ftell(fptr);   /* Find out how many bytes into the file we are */
  buf = (char*)malloc(length + 1);   /* Allocate a buffer for the entire length of the file and a null terminator */
  fseek(fptr, 0, SEEK_SET);   /* Go back to the beginning of the file */
  fread(buf, length, 1, fptr);   /* Read the contents of the file in to the buffer */
  fclose(fptr);   /* Close the file */
  buf[length] = 0;   /* Null terminator */
  return buf;   /* Return the buffer */
}

glm::vec3 planetPositions[] = {//location of planets
  glm::vec3( -5.0f,  0.0f,  0.0f),
  glm::vec3( 2.0f,  5.0f, -5.0f),
  glm::vec3(-1.5f, -2.2f, -2.5f),
  glm::vec3(-3.8f, -2.0f, -1.3f),
  glm::vec3( 2.4f, -0.4f, -3.5f),
  glm::vec3(-1.7f,  3.0f, -7.5f),
  glm::vec3( 1.3f, -2.0f, -2.5f)
};

// Used in View matrix
glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f,  5.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);
float speed_factor = 0.0f;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

glm::mat4 Projection = glm::mat4(1.0);
glm::mat4 View = glm::mat4(1.0);

// For viewing direction
float pitch = 0.0; //up or down
float yaw = -90.0;  // left or right
float mouse_sensitivity = 0.2;
glm::vec3 front;

bool tour_mode = false;
bool pause = false;
float camX_pause, camZ_pause;

struct localVertex {
  localVertex(): color{0.0,0.5,0.0} {};
  GLfloat position[3];
  GLfloat color[3];
};

typedef struct{
  localVertex p1, p2, p3;
} Facet;
/* These pointers will receive the contents of our shader source code files */
GLchar *vertexsource, *fragmentsource;
/* These are handles used to reference the shaders */
GLuint vertexshader, fragmentshader;
/* This is a handle to the shader program */
GLuint shaderprogram;
GLuint vao, vbo[1]; /* Create handles for our Vertex Array Object and One Vertex Buffer Object */
std::vector<localVertex> v;

GLchar *skyboxvertexsource, *skyboxfragmentsource;
GLuint skyboxvertexshader, skyboxfragmentshader;
GLuint skyboxshaderprogram;

GLuint skyboxvao, skyboxvbo;
float skyboxVertices[] = {
        // positions
        -40.0f,  40.0f, -40.0f,
        -40.0f, -40.0f, -40.0f,
         40.0f, -40.0f, -40.0f,
         40.0f, -40.0f, -40.0f,
         40.0f,  40.0f, -40.0f,
        -40.0f,  40.0f, -40.0f,

        -40.0f, -40.0f,  40.0f,
        -40.0f, -40.0f, -40.0f,
        -40.0f,  40.0f, -40.0f,
        -40.0f,  40.0f, -40.0f,
        -40.0f,  40.0f,  40.0f,
        -40.0f, -40.0f,  40.0f,

         40.0f, -40.0f, -40.0f,
         40.0f, -40.0f,  40.0f,
         40.0f,  40.0f,  40.0f,
         40.0f,  40.0f,  40.0f,
         40.0f,  40.0f, -40.0f,
         40.0f, -40.0f, -40.0f,

        -40.0f, -40.0f,  40.0f,
        -40.0f,  40.0f,  40.0f,
         40.0f,  40.0f,  40.0f,
         40.0f,  40.0f,  40.0f,
         40.0f, -40.0f,  40.0f,
        -40.0f, -40.0f,  40.0f,

        -40.0f,  40.0f, -40.0f,
         40.0f,  40.0f, -40.0f,
         40.0f,  40.0f,  40.0f,
         40.0f,  40.0f,  40.0f,
        -40.0f,  40.0f,  40.0f,
        -40.0f,  40.0f, -40.0f,

        -40.0f, -40.0f, -40.0f,
        -40.0f, -40.0f,  40.0f,
         40.0f, -40.0f, -40.0f,
         40.0f, -40.0f, -40.0f,
        -40.0f, -40.0f,  40.0f,
         40.0f, -40.0f,  40.0f
    };
void firstSkyboxSettings(){
  glGenVertexArrays(1, &skyboxvao);
  glGenBuffers(1, &skyboxvbo);
  glBindVertexArray(skyboxvao);
  glBindBuffer(GL_ARRAY_BUFFER, skyboxvbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
}
std::vector<std::string> faces{
  // "./skybox/right.jpg",
  // "./skybox/left.jpg",
  // "./skybox/top.jpg",
  // "./skybox/bottom.jpg",
  // "./skybox/front.jpg",
  // "./skybox/back.jpg"
  "./ame_nebula/purplenebula_rt.tga",
  "./ame_nebula/purplenebula_lf.tga",
  "./ame_nebula/purplenebula_up.tga",
  "./ame_nebula/purplenebula_dn.tga",
  "./ame_nebula/purplenebula_ft.tga",
  "./ame_nebula/purplenebula_bk.tga"
};
unsigned int loadCubemap(std::vector<std::string> faces){
  unsigned int textureID;
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

  int width, height, nrChannels;
  for(unsigned int i = 0; i < faces.size(); i++){
    unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
    if(data){
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
      stbi_image_free(data);
    } else{
      printf("Cubemap texture failed to load at path: %s\n", faces[i]);
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

/* Return the midpoint of two vectors */
localVertex Midpoint(localVertex p1, localVertex p2){
  localVertex p;
  p.position[0] = (p1.position[0] + p2.position[0])/2;
  p.position[1] = (p1.position[1] + p2.position[1])/2;
  p.position[2] = (p1.position[2] + p2.position[2])/2;
  return p;
}

/* Normalise a vector */
void Normalise(localVertex *p){
  float length;
  length = sqrt(pow(p->position[0],2) + pow(p->position[1],2) + pow(p->position[2],2));
  if(length != 0){
    p->position[0] /= length;
    p->position[1] /= length;
    p->position[2] /= length;
  } else{
    p->position[0] = 0;
    p->position[1] = 0;
    p->position[2] = 0;
  }
}

int CreateUnitSphere(int iterations, Facet *facets){
  int i, j, n, nstart;
  localVertex p1,p2,p3,p4,p5,p6;
  p1.position[0] = 0.0; p1.position[1] = 0.0; p1.position[2] = 1.0;
  p2.position[0] = 0.0; p2.position[1] = 0.0; p2.position[2] = -1.0;
  p3.position[0] = -1.0; p3.position[1] = -1.0; p3.position[2] = 0.0;
  p4.position[0] = 1.0; p4.position[1] = -1.0; p4.position[2] = 0.0;
  p5.position[0] = 1.0; p5.position[1] = 1.0; p5.position[2] = 0.0;
  p6.position[0] = -1.0; p4.position[1] = -1.0; p4.position[2] = 0.0;
  Normalise(&p1); Normalise(&p2); Normalise(&p3); Normalise(&p4); Normalise(&p5); Normalise(&p6);

  facets[0].p1 = p1;facets[0].p2 = p4;facets[0].p3 = p5;
  facets[1].p1 = p1;facets[1].p2 = p5;facets[1].p3 = p6;
  facets[2].p1 = p1;facets[2].p2 = p6;facets[2].p3 = p3;
  facets[3].p1 = p1;facets[3].p2 = p3;facets[3].p3 = p4;
  facets[4].p1 = p2;facets[4].p2 = p5;facets[4].p3 = p4;
  facets[5].p1 = p2;facets[5].p2 = p6;facets[5].p3 = p5;
  facets[6].p1 = p2;facets[6].p2 = p3;facets[6].p3 = p6;
  facets[7].p1 = p2;facets[7].p2 = p4;facets[7].p3 = p3;

  n = 8;

  for(i = 1; i<iterations; i++){
    nstart = n;

    for(j = 0; j<nstart; j++){
      /* Create initial copies for the new facets */
      facets[n] = facets[j];
      facets[n+1] = facets[j];
      facets[n+2] = facets[j];

      /* Calculate the midpoints */
      p1 = Midpoint(facets[j].p1, facets[j].p2);
      p2 = Midpoint(facets[j].p2, facets[j].p3);
      p3 = Midpoint(facets[j].p3, facets[j].p1);

      /* Replace the current facet */
      facets[j].p2 = p1;
      facets[j].p3 = p3;

      /* Create the changed vertices in the new facets */
      facets[n].p1 = p1;
      facets[n].p3 = p2;
      facets[n+1].p1 = p3;
      facets[n+1].p2 = p2;
      facets[n+2].p1 = p1;
      facets[n+2].p2 = p2;
      facets[n+2].p3 = p3;
      n += 3;
    }
  }
  printf("Hello\n");
  for(j = 0; j<n; j++){
    Normalise(&facets[j].p1);
    Normalise(&facets[j].p2);
    Normalise(&facets[j].p3);
  }
  return(n);
}

glm::vec3 perlin_noise(localVertex point){
  float Noise, m_Noise;
  glm::vec4 inter;
  Noise = sin(stb_perlin_noise3(point.position[0], point.position[1], point.position[2], 100, 100, 100)) * 20;
  m_Noise = Noise - int(Noise);
  inter = Noise * glm::vec4(1.0, 0., 0., 1.);
  inter = inter + m_Noise * glm::vec4(0., 1., 0., 1.);
  glm::vec3 return_inter(inter);
  return return_inter;
}

void SetupGeometry() {
  int i;
  int n = 5;
  Facet *f = NULL;
  f = (Facet *)malloc((int)pow(4,n) * 8 * sizeof(Facet)); // I added *8 because that's the expected number of facets
  n = CreateUnitSphere(n,f);
  printf("%d facets generated\n", n);
  GLfloat perlin_color[3];
  glm::vec3 perlin;

  for(i = 0; i<n; i++){
    perlin = perlin_noise(f[i].p1);
    perlin_color[0] = perlin.x; perlin_color[1] = perlin.y; perlin_color[2] = perlin.z;
    f[i].p1.color[0] = perlin_color[0] + f[i].p1.color[0]; f[i].p1.color[1] = perlin_color[1] + f[i].p1.color[1]; f[i].p1.color[2] = perlin_color[2] + f[i].p1.color[1];
    v.push_back(f[i].p1);
    perlin = perlin_noise(f[i].p2);
    perlin_color[0] = perlin.x; perlin_color[1] = perlin.y; perlin_color[2] = perlin.z;
    f[i].p2.color[0] = perlin_color[0] + f[i].p2.color[0]; f[i].p2.color[1] = perlin_color[1] + f[i].p2.color[1]; f[i].p2.color[2] = perlin_color[2] + f[i].p2.color[1];
    v.push_back(f[i].p2);
    perlin = perlin_noise(f[i].p3);
    perlin_color[0] = perlin.x; perlin_color[1] = perlin.y; perlin_color[2] = perlin.z;
    f[i].p3.color[0] = perlin_color[0] + f[i].p3.color[0]; f[i].p3.color[1] = perlin_color[1] + f[i].p3.color[1]; f[i].p3.color[2] = perlin_color[2] + f[i].p3.color[1];
    v.push_back(f[i].p3);
  }

  printf("Size %d\n", v.size());
  //
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  /* Allocate and assign One Vertex Buffer Object to our handle */
  glGenBuffers(1, vbo);
  /* Bind our VBO as being the active buffer and storing vertex attributes (coordinates + colors) */
  glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
  /* Copy the vertex data from cone to our buffer */
  /* v,size() * sizeof(GLfloat) is the size of the cone array, since it contains 12 Vertex values */
  glBufferData ( GL_ARRAY_BUFFER, v.size() * sizeof ( struct localVertex ), v.data(), GL_STATIC_DRAW );
  /* Specify that our coordinate data is going into attribute index 0, and contains three doubles per vertex */
  /* Note stride = sizeof ( struct Vertex ) and pointer = ( const GLvoid* ) 0 */
  glVertexAttribPointer ( ( GLuint ) 0, 3, GL_FLOAT, GL_FALSE,  sizeof ( struct localVertex ), ( const GLvoid* ) offsetof (struct localVertex, position) );
  /* Enable attribute index 0 as being used */
  glEnableVertexAttribArray(0);
  /* Specify that our color data is going into attribute index 1, and contains three floats per vertex */
  /* Note stride = sizeof ( struct Vertex ) and pointer = ( const GLvoid* ) ( 3 * sizeof ( GLdouble ) ) i.e. the size (in bytes)
     occupied by the first attribute (position) */
  glVertexAttribPointer ( ( GLuint ) 1, 3, GL_FLOAT, GL_FALSE, sizeof ( struct localVertex ), ( const GLvoid* ) offsetof(struct localVertex, color) );   // bug );
  glEnableVertexAttribArray ( 1 );
  glBindVertexArray(0);
}

void SetupShaders(void) {
  /* Read our shaders into the appropriate buffers */
  vertexsource = filetobuf("./tutorial3.vert");
  fragmentsource = filetobuf("./tutorial3.frag");
  /* Assign our handles a "name" to new shader objects */
  vertexshader = glCreateShader(GL_VERTEX_SHADER);
  fragmentshader = glCreateShader(GL_FRAGMENT_SHADER);
  /* Associate the source code buffers with each handle */
  glShaderSource(vertexshader, 1, (const GLchar**)&vertexsource, 0);
  glShaderSource(fragmentshader, 1, (const GLchar**)&fragmentsource, 0);
  /* Compile our shader objects */
  glCompileShader(vertexshader);
  glCompileShader(fragmentshader);
  /* Assign our program handle a "name" */
  shaderprogram = glCreateProgram();
  glAttachShader(shaderprogram, vertexshader);  /* Attach our shaders to our program */
  glAttachShader(shaderprogram, fragmentshader);
  glBindAttribLocation(shaderprogram, 0, "in_Position");   /* Bind attribute 0 (coordinates) to in_Position and attribute 1 (colors) to in_Color */
  glBindAttribLocation(shaderprogram, 1, "in_Color");
  glLinkProgram(shaderprogram);  /* Link our program, and set it as being actively used */
  CheckShader(shaderprogram, "Basic Shader");
  glUseProgram(shaderprogram);
}

void SetupSkyboxShaders(void) {
  /* Read our shaders into the appropriate buffers */
  skyboxvertexsource = filetobuf("./skybox.vert");
  skyboxfragmentsource = filetobuf("./skybox.frag");
  /* Assign our handles a "name" to new shader objects */
  skyboxvertexshader = glCreateShader(GL_VERTEX_SHADER);
  skyboxfragmentshader = glCreateShader(GL_FRAGMENT_SHADER);
  /* Associate the source code buffers with each handle */
  glShaderSource(skyboxvertexshader, 1, (const GLchar**)&skyboxvertexsource, 0);
  glShaderSource(skyboxfragmentshader, 1, (const GLchar**)&skyboxfragmentsource, 0);
  /* Compile our shader objects */
  glCompileShader(skyboxvertexshader);
  glCompileShader(skyboxfragmentshader);
  /* Assign our program handle a "name" */
  skyboxshaderprogram = glCreateProgram();
  glAttachShader(skyboxshaderprogram, skyboxvertexshader);  /* Attach our shaders to our program */
  glAttachShader(skyboxshaderprogram, skyboxfragmentshader);
  glBindAttribLocation(skyboxshaderprogram, 0, "aPos");   /* Bind attribute 0 (coordinates) to in_Position and attribute 1 (colors) to in_Color */
  glLinkProgram(skyboxshaderprogram);  /* Link our program, and set it as being actively used */
  CheckShader(skyboxshaderprogram, "Basic Shader");
  glUseProgram(skyboxshaderprogram); // This corresponds to skyboxShader.use();
  glUniform1i(glGetUniformLocation(skyboxshaderprogram, "skybox"), 0); //Corresponds to skyboxShader.setInt("skybox", 0);
  // The above line might have a problem with name.c_str(), in which case just try replacing it with "skybox"
  glDeleteShader(skyboxvertexshader);
  glDeleteShader(skyboxfragmentshader);
}

void moveAround(GLFWwindow *window){
  float currentFrame = glfwGetTime();
  deltaTime = currentFrame - lastFrame;
  lastFrame = currentFrame;
  float cameraSpeed = speed_factor * deltaTime; // adjust accordingly
  cameraPos += cameraSpeed * cameraFront; // move forward
  if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS){ // accelerate
    speed_factor += mouse_sensitivity/10;
    if(speed_factor > 10.0f)
      speed_factor = 10.0f;
  }
  if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS){ // decelerate
    speed_factor -= mouse_sensitivity/10;
    if(speed_factor < 0.0f)
      speed_factor = 0.0f;
  }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
      cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
      cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
  if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS){
    cameraPos   = glm::vec3(0.0f, 0.0f,  5.0f);
    cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);
    pitch = 0.0f;
    yaw = -90.0f;
  }
  if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
    if(!tour_mode) {
      tour_mode = true;
      printf("Tour mode started\n");
    }
  }
  if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
    if(tour_mode) {
      tour_mode = false;
      pause = false;
      printf("Exit tour mode.\n");
    }
  }
  if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    yaw -= mouse_sensitivity;
  if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    yaw += mouse_sensitivity;
  if (glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS)
    pitch += mouse_sensitivity;
  if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS)
    pitch -= mouse_sensitivity;
}

void Render(unsigned int i) {
  glUseProgram(shaderprogram);
  Projection = glm::perspective(45.0f, 1.0f, 0.1f, 100.0f);
  float radius = 10.0f;
  float camX = sin(glfwGetTime()) * radius;
  float camZ = cos(glfwGetTime()) * radius;
  View = glm::mat4(1.);
  if(pitch > 89.0f)
    pitch = 89.0f;
  if(pitch < -89.0f)
    pitch = -89.0f;
  front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
  front.y = sin(glm::radians(pitch));
  front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
  cameraFront = glm::normalize(front);
  if(tour_mode) {
    if(!pause){
      View = glm::lookAt(glm::vec3(camX, 0.0, camZ), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
      camX_pause = camX;
      camZ_pause = camZ;
    } else {
      View = glm::lookAt(glm::vec3(camX_pause, 0.0, camZ_pause), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
    }
  }
  else
    View = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
  glm::mat4 Model = glm::mat4(1.0);
  /* Bind our modelmatrix variable to be a uniform called mvpmatrix in our shaderprogram */
  glClearColor(0.0, 0.0, 0.0, 1.0);  /* Make our background black */
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glBindVertexArray(vao);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  for(unsigned int j = 0; j < sizeof(planetPositions)/sizeof(*planetPositions); j++){
    Model = glm::translate(Model, planetPositions[j]);
    Model = glm::scale(Model, glm::vec3(1.5f, 1.5f, 1.5f));
    glm::vec3 current_centre = planetPositions[j];
    glUniform3fv(glGetUniformLocation(shaderprogram, "centre"), 1, glm::value_ptr(current_centre));
    glm::mat4 MVP = Projection * View * Model;
    glUniformMatrix4fv(glGetUniformLocation(shaderprogram, "mvpmatrix"), 1, GL_FALSE, glm::value_ptr(MVP));
    glDrawArrays(GL_TRIANGLES, 0, v.size());
  }

  glBindVertexArray(0);
  /* Invoke glDrawArrays telling that our data consists of a triangle fan */

  glDepthFunc(GL_LEQUAL);
  glUseProgram(skyboxshaderprogram);
  glm::mat4 view = glm::mat4(1.);
  view = glm::mat4(glm::mat3(View));
  glUniformMatrix4fv(glGetUniformLocation(skyboxshaderprogram, "view"), 1, GL_FALSE, glm::value_ptr(view)); //try view[0][0] if necessary
  glUniformMatrix4fv(glGetUniformLocation(skyboxshaderprogram, "projection"), 1, GL_FALSE, glm::value_ptr(Projection)); //try Projection[0][0] if necessary
  glBindVertexArray(skyboxvao);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, i); //i represents cubemapTexture
  glDrawArrays(GL_TRIANGLES, 0, 36);
  glBindVertexArray(0);
  glDepthFunc(GL_LESS);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  if ((key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q) && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GL_TRUE);
  if ((key == GLFW_KEY_T && mods == GLFW_MOD_SHIFT) && action == GLFW_PRESS) {
    if(tour_mode && !pause) {
      pause = true;
      printf("Paused tour.\n");
    } else if(tour_mode && pause) {
      pause = false;
      printf("Resuming tour.\n");
    }
  }
}

int main( void ) {
  int k = 0;
  GLFWwindow* window;
  if( !glfwInit() ) {
    printf("Failed to start GLFW\n");
    exit( EXIT_FAILURE );
  }

#ifdef __APPLE__
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif

  window = glfwCreateWindow(640, 480, "Space Wars", NULL, NULL);
  if (!window) {
    glfwTerminate();
    printf("GLFW Failed to start\n");
    return -1;
  }
  /* Make the window's context current */
  glfwMakeContextCurrent(window);

#ifndef __APPLE__
  // IMPORTANT: make window curren must be done so glew recognises OpenGL
  glewExperimental = GL_TRUE;
  int err = glewInit();
  if (GLEW_OK != err) {
    /* Problem: glewInit failed, something is seriously wrong. */
    fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
  }
#endif

  glfwSetKeyCallback(window, key_callback);
  fprintf(stderr, "GL INFO %s\n", glGetString(GL_VERSION));
  glEnable(GL_DEPTH_TEST);
  SetupGeometry();
  firstSkyboxSettings();
  SetupShaders();
  SetupSkyboxShaders();
  printf("While textures are loading, here's a foreword about the scene.\n");
  printf("(Star Wars music in background)\n");
  printf("A long time ago in a galaxy far, far away...\n");
  printf("In a world devastated by space wars, astronauts  in spaceships still roam the skies searching for a place to establish a colony.\n");
  printf("All they can find is planets destroyed: volcanic flames on one side, nuclear waste on the other.\n");
  printf("(I must admit, manipulating Perlin noise to give these destroyed planets their appearance was no joke!)\n");
  printf("By the time you finish reading this, the program will have loaded. I hope you enjoy it!\n");
  unsigned int cubemapTexture = loadCubemap(faces);
  Shader ourShader("1.model_loading.vs", "1.model_loading.fs");
  const char *filename = "E 45 Aircraft_obj.obj";
  Model ourModel(filename);
  printf("Ready to render\n");
  while(!glfwWindowShouldClose(window)) {  // Main loop
    moveAround(window);

    Render(cubemapTexture);        // OpenGL rendering goes here...

    ourShader.use();

    // view/projection transformations
    GLfloat angle;
    float t = glfwGetTime();
    float p = 400.;
    t = fmod(t, p);
    angle = t * 360. / p;

    // render the loaded model
    glm::mat4 model;
    model = glm::translate(model, glm::vec3(0.0f, -1.75f, 2.0f)); // translate it down so it's at the center of the scene
    model = glm::translate(model, glm::vec3(2 * sin(angle), 0.0f, cos(angle) - 1));
    model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));	// it's a bit too big for our scene, so scale it down
    model = glm::rotate(model, -angle * 0.5f, glm::vec3(0.f, 2.f, 0.f));
    glUniformMatrix4fv(glGetUniformLocation(ourShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(Projection));
    glUniformMatrix4fv(glGetUniformLocation(ourShader.ID, "view"), 1, GL_FALSE, glm::value_ptr(View));
    glUniformMatrix4fv(glGetUniformLocation(ourShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
    ourModel.Draw(ourShader);

    glm::mat4 model2;
    model2 = glm::translate(model2, glm::vec3(0.0f, 0.f, -17.0f));
    model2 = glm::translate(model2, glm::vec3(-3 * sin(angle), 2 * sin(angle) * cos(angle), 2 * cos(angle) - 1));
    model2 = glm::scale(model2, glm::vec3(0.5f, 0.5f, 0.5f));
    model2 = glm::rotate(model2, angle * 0.5f, glm::vec3(0.f, 2.f, 0.f));
    glUniformMatrix4fv(glGetUniformLocation(ourShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(model2));
    ourModel.Draw(ourShader);

    glfwSwapBuffers(window);        // Swap front and back rendering buffers
    glfwPollEvents();         // Poll for events.

  }
  glfwTerminate();  // Close window and terminate GLFW
  exit( EXIT_SUCCESS );  // Exit program
}
