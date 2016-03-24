#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace std;
float radius = 0.6;
int score=0;
bool fl=true;
GLFWwindow* window;
int hei = 700,wid = 1360;
double bx=-65,by=-15,vx=0,vy=0,fl1=1,velo=7;
struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};
typedef struct VAO VAO;
typedef struct subs {
  double x,y,vxx,vyy,radi,flag;
  VAO *vao;
} subs;
int count=0;
double angle1;
struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;
vector<subs> obst;
GLuint programID;
subs ball,triangle;
vector<subs> arr;
int leds[10][7] = {1,1,1,1,1,1,0,
                  0,0,0,0,1,1,0,
                  1,0,1,1,0,1,1,
                  1,0,0,1,1,1,1,
                  0,1,0,0,1,1,1,
                  1,1,0,1,1,0,1,
                  1,1,1,1,1,0,1,
                  1,0,0,0,1,1,0,
                  1,1,1,1,1,1,1,
                  1,1,0,1,1,1,1};
float pt[10][12] = {2,4,2,3.2,-2,4,-2,4,2,3.2,-2,3.2,
                      -2,4,-1.2,4,-2,-.4,-1.2,4,-2,-.4,-1.2,-.4,
                      -2,.4,-1.2,.4,-2,-4,-1.2,.4,-2,-4,-1.2,-4,
                      -2,-4,2,-4,-2,-3.2,2,-4,-2,-3.2,2,-3.2,
                      2,-4,1.2,-4,2,.4,1.2,-4,2,.4,1.2,.4,
                      2,4,2,-.4,1.2,4,2,-.4,1.2,4,1.2,-.4,
                      2,.4,2,-.4,-2,.4,2,-.4,-2,.4,-2,-.4};
/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
                          0,                  // attribute 0. Vertices
                          3,                  // size (x,y,z)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
                          1,                  // attribute 1. Color
                          3,                  // size (r,g,b)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    return vao;
}
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
        color_buffer_data [3*i] = red;
        color_buffer_data [3*i + 1] = green;
        color_buffer_data [3*i + 2] = blue;
    }


    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}
void draw3DObject (struct VAO* vao)
{
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);
    glBindVertexArray (vao->VertexArrayID);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/

void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_RELEASE) {
        switch (key) {
            case GLFW_KEY_P:
              velo+=0.5;
              break;
            case GLFW_KEY_M:
              velo-=0.5;
              break;
            case GLFW_KEY_SPACE:
              if(by < -35 || bx > 75 || fl) {
                double xpos, ypos;
                int width,height;
                glfwGetCursorPos(window,&xpos, &ypos);
                glfwGetFramebufferSize(window, &width, &height);
                xpos =- 77+154.0/width*xpos;
                ypos =- 40+80.0/height*ypos;
                ypos *= -1;
                angle1 = (max(ypos,-14.0)+15)/(max(xpos,-65.0)+65);
                vy = velo*sin(atan(angle1));
                vx = velo*cos(atan(angle1));
                fl = false;
                fl1 = 1;
              }
              break;
            default:
                break;
        }
    }
    else if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                quit(window);
                break;
            default:
                break;
        }
    }
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
	switch (key) {
		case 'Q':
		case 'q':
            quit(window);
            break;
		default:
			break;
	}
}

// void mouseButton (GLFWwindow* window, int button, int action, int mods)
// {
//     switch (button) {
//         case GLFW_MOUSE_BUTTON_LEFT:
//             if (action == GLFW_RELEASE)
//                 triangle_rot_dir *= -1;
//             break;
//         case GLFW_MOUSE_BUTTON_RIGHT:
//             if (action == GLFW_RELEASE) {
//                 rectangle_rot_dir *= -1;
//             }
//             break;
//         default:
//             break;
//     }
// }


void reshapeWindow (GLFWwindow* window, int width, int height)
{
    int fbwidth=width, fbheight=height;
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = 90.0f;
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

    Matrices.projection = glm::ortho(-77.0f, 77.0f, -40.0f, 40.0f, 0.1f, 500.0f);

}

int tot=0;
void createTriangle (float x1,float y1,float x2,float y2,float x3,float y3)
{
  static const GLfloat vertex_buffer_data [] = {
    x1,y1,0, // vertex 1
    x2,y2,0, // vertex 2
    x3,y3,0, // vertex 3
  };

  static const GLfloat color_buffer_data [] = {
    0.1,0.1,0.1, // color 1
    0.1,0.1,0.1, // color 2
    0.1,0.1,0.1, // color 3
  };

 triangle.vao = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
}

void createRectangle (float x1,float y1,float x2,float y2,float x3,float y3,float x4,float y4,float x5,float y5,float x6,float y6)
{
    GLfloat vertex_buffer_data [] = {
    x1,y1,0, // vertex 1
    x2,y2,0, // vertex 2
    x3,y3,0, // vertex 3

    x4,y4,0, // vertex 1
    x5,y5,0, // vertex 2
    x6,y6,0, // vertex 3
  };

  static const GLfloat color_buffer_data [] = {
    0.1,0.1,0.1, // color 1
    0.1,0.1,0.1, // color 2
    0.1,0.1,0.1, // color 3

    0.1,0.1,0.1, // color 1
    0.1,0.1,0.1, // color 2
    0.1,0.1,0.1, // color 3
  };
  subs u;
  u.vao = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
  arr.push_back(u);
}
subs rect;
void createRectangle1 (float x1,float y1,float x2,float y2,float x3,float y3,float x4,float y4,float x5,float y5,float x6,float y6)
{
    GLfloat vertex_buffer_data [] = {
    x1,y1,0, // vertex 1
    x2,y2,0, // vertex 2
    x3,y3,0, // vertex 3

    x4,y4,0, // vertex 1
    x5,y5,0, // vertex 2
    x6,y6,0, // vertex 3
  };

  static const GLfloat color_buffer_data [] = {
    0,0,0, // color 1
    0,0,0, // color 2
    0,0,0, // color 3

    0,0,0, // color 1
    0,0,0, // color 2
    0,0,0, // color 3
  };
  rect.vao = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}


void createNumber(int num) {
  // for(int i=0;i<10;i++)
  //   if(leds[num][i]) {
      GLfloat vertex_buffer_data [] = {
        pt[num][0],pt[num][1],0, // vertex 1
        pt[num][2],pt[num][3],0, // vertex 2
        pt[num][4],pt[num][5],0, // vertex 3

        pt[num][6],pt[num][7],0, // vertex 1
        pt[num][8],pt[num][9],0, // vertex 2
        pt[num][10],pt[num][11],0, // vertex 3
      };

      GLfloat color_buffer_data [] = {
        0,0,0, // color 1
        0,0,0, // color 2
        0,0,0, // color 3

        0,0,0, // color 1
        0,0,0, // color 2
        0,0,0, // color 3
      };
      subs u;
      u.vao = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
      draw3DObject(u.vao);
    // }
}
void createBall(){
	GLfloat vertex_buffer_data [9005];
	int j=0;
	for(int i=0;i<1000;i++){
		vertex_buffer_data[j++]=radius*cos(2*3.14159*i/1000.0);
		vertex_buffer_data[j++]=radius*sin(2*3.14159*i/1000.0);
		vertex_buffer_data[j++]=0;
		vertex_buffer_data[j++]=radius*cos(2*3.14159*(i+1)/1000.0);
		vertex_buffer_data[j++]=radius*sin(2*3.14159*(i+1)/1000.0);
		vertex_buffer_data[j++]=0;
		vertex_buffer_data[j++]=0;
		vertex_buffer_data[j++]=0;
		vertex_buffer_data[j++]=0;
	}

	GLfloat color_buffer_data [9005];
	for(int i=0;i<9000;i+=3){
		color_buffer_data[i]=0;
		color_buffer_data[i+1]=0;
		color_buffer_data[i+2]=0;
	}
	ball.vao = create3DObject(GL_TRIANGLES, 3000, vertex_buffer_data, color_buffer_data, GL_LINE);
}

void createBall1(double radius,double xx,double yy) {
	GLfloat vertex_buffer_data [9005];
	int j=0;
	for(int i=0;i<1000;i++){
		vertex_buffer_data[j++]=radius*cos(2*3.14159*i/1000.0);
		vertex_buffer_data[j++]=radius*sin(2*3.14159*i/1000.0);
		vertex_buffer_data[j++]=0;
		vertex_buffer_data[j++]=radius*cos(2*3.14159*(i+1)/1000.0);
		vertex_buffer_data[j++]=radius*sin(2*3.14159*(i+1)/1000.0);
		vertex_buffer_data[j++]=0;
		vertex_buffer_data[j++]=0;
		vertex_buffer_data[j++]=0;
		vertex_buffer_data[j++]=0;
	}

	GLfloat color_buffer_data [9005];
	for(int i=0;i<9000;i+=3){
		color_buffer_data[i]=0;
		color_buffer_data[i+1]=0;
		color_buffer_data[i+2]=0;
	}
  subs ball;
	ball.vao = create3DObject(GL_TRIANGLES, 3000, vertex_buffer_data, color_buffer_data, GL_LINE);
  ball.x = xx;
  ball.y = yy;
  ball.radi = radius;
  obst.push_back(ball);
}

float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float triangle_rotation = 0;

void draw ()
{
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUseProgram (programID);

  glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
  glm::vec3 target (0, 0, 0);
  glm::vec3 up (0, 1, 0);
  Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane
  glm::mat4 VP = Matrices.projection * Matrices.view;
  glm::mat4 MVP;	// MVP = Projection * View * Model
  Matrices.model = glm::mat4(1.0f);

  double xpos, ypos;
  int width,height;
  glfwGetCursorPos(window,&xpos, &ypos);
  glfwGetFramebufferSize(window, &width, &height);
  xpos = -77+(float)154.0/width*xpos;
  ypos = -40+(float)80.0/height*ypos;
  ypos *= -1;
  float angle2 = (max(ypos,-14.0)+15)/(max(xpos,-65.0)+65);

  glm::mat4 translateTriangle = glm::translate (glm::vec3(-65.0 + 30.0*cos(atan(angle2)),-15.0 + 30.0*sin(atan(angle2)), 0.0)); // glTranslatef
//  glm::mat4 rotrate = glm::rotate ((float)(angle2), glm::vec3(0,0,1)); // rotate about vector (-1,0.8,1)
  glm::mat4 triangleTransform = translateTriangle;
  Matrices.model = triangleTransform;
  MVP = VP * Matrices.model; // MVP = p * V * M
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(triangle.vao);

  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateRectangle = glm::translate (glm::vec3(-65, -30, 0));        // glTranslatef
  Matrices.model = translateRectangle;
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(arr[0].vao);

  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateRectangle1 = glm::translate (glm::vec3(-65, -20, 0));        // glTranslatef
  Matrices.model = translateRectangle1;
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(arr[1].vao);

  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateBall = glm::translate (glm::vec3(bx, by, 0));        // glTranslatef
  Matrices.model *= translateBall;
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(ball.vao);

  count++;
  count%=500;
  if(!count)
    createBall1(rand()%7,rand()%60,rand()%25 );

  for(int i=0;i<obst.size();i++) {
    if(abs(obst[i].vyy)<0.03 && abs(-35+obst[i].radi-obst[i].y)<=0.4) {
      obst[i].vyy = 0.0;
    }
    if(sqrt(pow(bx-obst[i].x,2)+pow(by-obst[i].y,2)) <= obst[i].radi+0.6) { //collision
      if(obst[i].flag !=1.0) {
        score++;
        obst[i].vxx = vx/max(2.0,obst[i].radi);
        vx *= -0.4;
      }
      obst[i].flag = 1.0;
    }
    obst[i].x = obst[i].vxx+obst[i].x;
    obst[i].vxx *= 0.92;
    if(obst[i].flag==1.0) {
      if(obst[i].y!=-35+obst[i].radi)
      obst[i].y = max(obst[i].vyy+obst[i].y,-35+obst[i].radi);
      obst[i].vyy -= 0.1*fl1;
    }
    if(obst[i].y==-35+obst[i].radi)
      obst[i].vyy *= -0.4;
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 translateBall = glm::translate (glm::vec3(obst[i].x,obst[i].y, 0));        // glTranslatef
    Matrices.model *= translateBall;
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(obst[i].vao);
  }
  int dup = score;
  double xax=60;
  int o;
  while(dup) {
    for(int i=0;i<7;i++) {
      o=dup%10;
      if(leds[dup%10][i]) {
        Matrices.model = glm::mat4(1.0f);
        glm::mat4 translateBa = glm::translate (glm::vec3(xax,30,0));        // glTranslatef
        Matrices.model = translateBa;
        MVP = VP * Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        createRectangle1(pt[i][0],pt[i][1],pt[i][2],pt[i][3],pt[i][4],pt[i][5],pt[i][6],pt[i][7],pt[i][8],pt[i][9],pt[i][10],pt[i][11]);
        draw3DObject(rect.vao);
      }
    }
    dup/=10;
    xax-=5;
  }
  bx += vx*0.3;
  by += vy*0.3;
  if(!fl) {
    vy -= 0.1*fl1;
//    vx = .90;
  }
  float increments = 1;
}

GLFWwindow* initGLFW (int width, int height) {
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );

    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);

    glfwSetWindowCloseCallback(window, quit);

    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

//    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks

    return window;
}

void initGL (GLFWwindow* window, int width, int height)
{
	createTriangle (1,1,1,-1,-1,-1); // Generate the VAO, VBOs, vertices data & copy into the array buffer
	createRectangle (5,5,5,-5,-5,-5,5,5,-5,5,-5,-5);
  createRectangle (-1,-5,1,5,1,-5,-1,-5,1,5,-1,5);
  createBall ();
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");

	reshapeWindow (window, width, height);

	glClearColor (0.3f, 0.3f, 0.3f, 0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

    cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
    cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
    cout << "VERSION: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
	int width = wid;//1360;
	int height = hei;//700;
    window = initGLFW(width, height);

	initGL (window, width, height);

    double last_update_time = glfwGetTime(), current_time;

    while (!glfwWindowShouldClose(window)) {

        draw();

        if(by < -40 || bx > 80 || bx < -80 || by > 40) { bx = -65;by = -15;vx = 0;vy = 0;fl = true; }
        glfwSwapBuffers(window);

        glfwPollEvents();

        current_time = glfwGetTime(); // Time in seconds
        if ((current_time - last_update_time) >= 0.5)
            last_update_time = current_time;
    }
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
