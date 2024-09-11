/*
CPE/CSC 474 Lab base code Eckhardt/Dahl
based on CPE/CSC 471 Lab base code Wood/Dunn/Eckhardt
*/

#include <iostream>
#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "GLSL.h"
#include "Program.h"
#include "WindowManager.h"
#include "Shape.h"
#include "line.h"
// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace std;
using namespace glm;
shared_ptr<Shape> shape;

static int cur_point = 0;
const int MAX_LENGTH = 100;
char input[MAX_LENGTH];
static float total_time;

struct LetterDict {
    char letter;
    std::vector<float> interpFactors;
};

struct factorList {
    std::vector<float> interpFactors;
};

double get_last_elapsed_time()
{
	static double lasttime = glfwGetTime();
	double actualtime =glfwGetTime();
	double difference = actualtime- lasttime;
	lasttime = actualtime;
	return difference;
}
class camera
{
public:
	glm::vec3 pos, rot;
	int w, a, s, d;
	camera()
	{
		w = a = s = d = 0;
		pos = rot = glm::vec3(0, 0, 0);
        pos = glm::vec3(0, 0, -5);
	}
	glm::mat4 process(double ftime)
	{
		float speed = 0;
		if (w == 1)
		{
			speed = 10*ftime;
		}
		else if (s == 1)
		{
			speed = -10*ftime;
		}
		float yangle=0;
		if (a == 1)
			yangle = -3*ftime;
		else if(d==1)
			yangle = 3*ftime;
		rot.y += yangle;
		glm::mat4 R = glm::rotate(glm::mat4(1), rot.y, glm::vec3(0, 1, 0));
		glm::vec4 dir = glm::vec4(0, 0, a,1);
		dir = dir*R;
		pos += glm::vec3(dir.x, dir.y, dir.z);
		glm::mat4 T = glm::translate(glm::mat4(1), pos);
		return R*T;
	}
};

camera mycam;

class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> prog, psky, pplane;

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our box to OpenGL
	GLuint VertexBufferID, VertexNormDBox, VertexTexBox, IndexBufferIDBox;

	//texture data
	GLuint Texture;
	GLuint Texture2;

    int face_vertexCount;
    
    float interp_factor_1;
    float interp_factor_2;
    float interp_factor_3;
    float interp_factor_4;
    
    vector<LetterDict> letterDict;
    
    vector<factorList> readFactorList;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		
		if (key == GLFW_KEY_W && action == GLFW_PRESS)
		{
			mycam.w = 1;
		}
		if (key == GLFW_KEY_W && action == GLFW_RELEASE)
		{
			mycam.w = 0;
		}
		if (key == GLFW_KEY_S && action == GLFW_PRESS)
		{
			mycam.s = 1;
		}
		if (key == GLFW_KEY_S && action == GLFW_RELEASE)
		{
			mycam.s = 0;
		}
		if (key == GLFW_KEY_A && action == GLFW_PRESS)
		{
			mycam.a = 1;
		}
		if (key == GLFW_KEY_A && action == GLFW_RELEASE)
		{
			mycam.a = 0;
		}
		if (key == GLFW_KEY_D && action == GLFW_PRESS)
		{
			mycam.d = 1;
		}
		if (key == GLFW_KEY_D && action == GLFW_RELEASE)
		{
			mycam.d = 0;
		}

	}

	// callback for the mouse when clicked move the triangle when helper functions
	// written
	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		double posX, posY;
		float newPt[2];
		if (action == GLFW_PRESS)
		{
			glfwGetCursorPos(window, &posX, &posY);
			//std::cout << "Pos X " << posX <<  " Pos Y " << posY << std::endl;

			//change this to be the points converted to WORLD
			//THIS IS BROKEN< YOU GET TO FIX IT - yay!
			newPt[0] = (float)posX;
			newPt[1] = (float)posY;
            
            newPt[0] /= 1920.0f;
            newPt[1] /= 1080.0f;
            
//            interp_factor_x = newPt[0];
//            interp_factor_y = newPt[1];
//
//            std::cout << "Pos X " << newPt[0] <<  " Pos Y " << newPt[1] << std::endl;

//			std::cout << "converted:" << newPt[0] << " " << newPt[1] << std::endl;
//			glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);
//			//update the vertex array with the updated points
//			glBufferSubData(GL_ARRAY_BUFFER, sizeof(float)*6, sizeof(float)*2, newPt);
//			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}
	}

	//if the window is resized, capture the new size and reset the viewport
	void resizeCallback(GLFWwindow *window, int in_width, int in_height)
	{
		//get the window size - may be different then pixels for retina
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
	}

	/*Note that any gl calls must always happen after a GL state is initialized */
	void initGeom(const std::string& resourceDirectory)
	{

		// Initialize mesh.
		shape = make_shared<Shape>();
		shape->loadMesh(resourceDirectory + "/sphere.obj");
		shape->resize();
		shape->init();
        
        //------------------------------------------------------------------
        
        //generate the VAO
        glGenVertexArrays(1, &VertexArrayID);
        glBindVertexArray(VertexArrayID);
        
        //------------------------------------------------------------------
        
        vector<float> buf_1;
        vector<float> norBuf;
        loadFaceMesh(buf_1, norBuf, resourceDirectory + "/mouthDefault.obj");
        face_vertexCount = buf_1.size() / 3.0f;

		//generate vertex buffer to hand off to OGL
		glGenBuffers(1, &VertexBufferID);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);
		//actually memcopy the data - only do this once
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * buf_1.size(), &buf_1[0], GL_DYNAMIC_DRAW);
		//we need to set up the vertex array
		glEnableVertexAttribArray(0);
		//key function to get up how many elements to pull out at a time (3)
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        
        //------------------------------------------------------------------
        
        vector<float> buf_2;
        loadFaceMesh(buf_2, norBuf, resourceDirectory + "/mouthOpen.obj");
        
        //generate vertex buffer to hand off to OGL
        glGenBuffers(1, &VertexBufferID);
        //set the current state to focus on our vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);
        //actually memcopy the data - only do this once
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * buf_2.size(), &buf_2[0], GL_DYNAMIC_DRAW);
        //we need to set up the vertex array
        glEnableVertexAttribArray(1);
        //key function to get up how many elements to pull out at a time (3)
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        
        //------------------------------------------------------------------
        
        vector<float> buf_3;
        loadFaceMesh(buf_3, norBuf, resourceDirectory + "/mouthO.obj");
        
        //generate vertex buffer to hand off to OGL
        glGenBuffers(1, &VertexBufferID);
        //set the current state to focus on our vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);
        //actually memcopy the data - only do this once
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * buf_3.size(), &buf_3[0], GL_DYNAMIC_DRAW);
        //we need to set up the vertex array
        glEnableVertexAttribArray(2);
        //key function to get up how many elements to pull out at a time (3)
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        
        //------------------------------------------------------------------
        
        vector<float> buf_4;
        loadFaceMesh(buf_4, norBuf, resourceDirectory + "/mouthE.obj");
        
        //generate vertex buffer to hand off to OGL
        glGenBuffers(1, &VertexBufferID);
        //set the current state to focus on our vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);
        //actually memcopy the data - only do this once
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * buf_4.size(), &buf_4[0], GL_DYNAMIC_DRAW);
        //we need to set up the vertex array
        glEnableVertexAttribArray(3);
        //key function to get up how many elements to pull out at a time (3)
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        
        //------------------------------------------------------------------
        
        //generate vertex buffer to hand off to OGL
        glGenBuffers(1, &VertexBufferID);
        //set the current state to focus on our vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);
        //actually memcopy the data - only do this once
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * norBuf.size(), &norBuf[0], GL_DYNAMIC_DRAW);
        //we need to set up the vertex array
        glEnableVertexAttribArray(4);
        //key function to get up how many elements to pull out at a time (3)
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        

		glBindVertexArray(0);
        
        //----------------------------------


		int width, height, channels;
		char filepath[1000];

		//texture 1
		string str = resourceDirectory + "/explosion.jpg";
		strcpy(filepath, str.c_str());
		unsigned char* data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &Texture);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		//texture 2
		str = resourceDirectory + "/sky.jpg";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &Texture2);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, Texture2);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
        
        // Default, Open, O, E
        initFactors('a', 0.5f,0,0,0.5f);
        initFactors('b', 0.4f,0,0,0.6f);
        initFactors('c', 0.3f,0,0,0.7f);
        initFactors('d', 0.2f,0,0,0.8f);
        initFactors('e', 0.0f,0,0,1.0f);
        initFactors('f', 1.0f,0,0,0);
        initFactors('g', 0,0.5f,0,0.5f);
        initFactors('h', 0.5f,0,0,0.5f);
        initFactors('i', 0,0,0.3f,0.7f);
        initFactors('j', 0,0.7f,0.3f,0);
        initFactors('k', 0.3f,0,0,0.7f);
        initFactors('l', 0.5f,0,0,0.5f);
        initFactors('m', 0.8f,0.2f,0,0);
        initFactors('n', 1.0f,0,0,0);
        initFactors('o', 0,0,1.0f,0);
        initFactors('p', 0,0,0,1.0f);
        initFactors('q', 0,0.2f,0.8f,0);
        initFactors('r', 0.3f,0.7f,0,0);
        initFactors('s', 0.4f,0,0,0.6f);
        initFactors('t', 0.2f,0,0,0.8f);
        initFactors('u', 0,0.3f,0.7f,0);
        initFactors('v', 0.2f,0,0,0.8f);
        initFactors('w', 0,0.3f,0.7f,0);
        initFactors('x', 0.2f,0,0,0.8f);
        initFactors('y', 0,0,0.3f,0.7f);
        initFactors('z', 0,0,0,1.0f);

        for (int i = 0; input[i] != '\0'; i++){
            for (int j = 0; j < 26; j++){
                if (input[i] == letterDict[j].letter){
                    
                    factorList factors;
                    factors.interpFactors.push_back(letterDict[j].interpFactors[0]);
                    factors.interpFactors.push_back(letterDict[j].interpFactors[1]);
                    factors.interpFactors.push_back(letterDict[j].interpFactors[2]);
                    factors.interpFactors.push_back(letterDict[j].interpFactors[3]);
                    readFactorList.push_back(factors);
                }
            }
        }
        total_time = readFactorList.size();
	}

	//General OGL initialization - set OGL state here
	void init(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();

		// Set background color.
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);
		//glDisable(GL_DEPTH_TEST);
		// Initialize the GLSL program.
		psky = std::make_shared<Program>();
		psky->setVerbose(true);
		psky->setShaderNames(resourceDirectory + "/skyvertex.glsl", resourceDirectory + "/skyfrag.glsl");
		if (!psky->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		psky->addUniform("P");
		psky->addUniform("V");
		psky->addUniform("M");
		psky->addUniform("campos");
		psky->addAttribute("vertPos");
		psky->addAttribute("vertNor");
		psky->addAttribute("vertTex");

		pplane = std::make_shared<Program>();
		pplane->setVerbose(true);
		pplane->setShaderNames(resourceDirectory + "/plane_vertex.glsl", resourceDirectory + "/plane_frag.glsl");
		if (!pplane->init())
			{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
			}
		pplane->addUniform("P");
		pplane->addUniform("V");
		pplane->addUniform("M");
		pplane->addUniform("campos");
		pplane->addAttribute("vertPos1");
		pplane->addAttribute("vertPos2");
		pplane->addAttribute("vertPos3");
        pplane->addAttribute("vertPos4");
        pplane->addAttribute("norPos");
        pplane->addUniform("interp_factor_1");
        pplane->addUniform("interp_factor_2");
        pplane->addUniform("interp_factor_3");
        pplane->addUniform("interp_factor_4");
	}

    void initFactors(char a, float f1, float f2, float f3, float f4){
        LetterDict ld;
        ld.letter = a;
        ld.interpFactors.push_back(f1);
        ld.interpFactors.push_back(f2);
        ld.interpFactors.push_back(f3);
        ld.interpFactors.push_back(f4);
        letterDict.push_back(ld);
    }
    
    void interp_factors(float total_time, float cur_time, vector<factorList>& data) {
        
        int size = (int)data.size();
        float unitTime = total_time/size;
        int cur_face = (int)cur_time / unitTime;
        float interp_t = cur_time - (cur_face * unitTime);
        interp_t /= unitTime;
        
        if (cur_face < size - 1){
            interp_factor_1 = mix(data[cur_face].interpFactors[0], data[cur_face +1].interpFactors[0], interp_t);
            interp_factor_2 = mix(data[cur_face].interpFactors[1], data[cur_face +1].interpFactors[1], interp_t);
            interp_factor_3 = mix(data[cur_face].interpFactors[2], data[cur_face +1].interpFactors[2], interp_t);
            interp_factor_4 = mix(data[cur_face].interpFactors[3], data[cur_face +1].interpFactors[3], interp_t);
        }
        else if (cur_face < size){
            interp_factor_1 = mix(data[cur_face].interpFactors[0], 1.0f, interp_t);
            interp_factor_2 = mix(data[cur_face].interpFactors[1], 0.0f, interp_t);
            interp_factor_3 = mix(data[cur_face].interpFactors[2], 0.0f, interp_t);
            interp_factor_4 = mix(data[cur_face].interpFactors[3], 0.0f, interp_t);
        }
        else{
            interp_factor_1 = 1.0f;
            interp_factor_2 = 0.0f;
            interp_factor_3 = 0.0f;
            interp_factor_4 = 0.0f;
        }

        
    }
    
	/****DRAW
	This is the most important function in your program - this is where you
	will actually issue the commands to draw any geometry you have set up to
	draw
	********/
	void render()
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		double frametime = get_last_elapsed_time();

		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		float aspect = width/(float)height;
		glViewport(0, 0, width, height);
        
        static float curTime = 0;

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Create the matrix stacks - please leave these alone for now
		
		glm::mat4 V, M, P; //View, Model and Perspective matrix
		V = mycam.process(frametime);
		M = glm::mat4(1);
		// Apply orthographic projection....
		P = glm::ortho(-1 * aspect, 1 * aspect, -1.0f, 1.0f, -2.0f, 100.0f);		
		if (width < height)
			{
			P = glm::ortho(-1.0f, 1.0f, -1.0f / aspect,  1.0f / aspect, -2.0f, 100.0f);
			}
		// ...but we overwrite it (optional) with a perspective projection.
		P = glm::perspective((float)(3.14159 / 4.), (float)((float)width/ (float)height), 0.1f, 1000.0f); //so much type casting... GLM metods are quite funny ones
		float sangle = 3.1415926 / 2.;
		glm::mat4 RotateXSky = glm::rotate(glm::mat4(1.0f), sangle, glm::vec3(1.0f, 0.0f, 0.0f));
		glm::vec3 camp = -mycam.pos;
		glm::mat4 TransSky = glm::translate(glm::mat4(1.0f), camp);
		glm::mat4 SSky = glm::scale(glm::mat4(1.0f), glm::vec3(0.8f, 0.8f, 0.8f));

		M = TransSky * RotateXSky * SSky;

		// Draw the sky using GLSL.
		psky->bind();		
		glUniformMatrix4fv(psky->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(psky->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(psky->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform3fv(psky->getUniform("campos"), 1, &mycam.pos[0]);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture2);
		glDisable(GL_DEPTH_TEST);
		shape->draw(psky, false);			//render!!!!!!!
		glEnable(GL_DEPTH_TEST);	
		psky->unbind();

		// Draw the plane using GLSL.
		glm::mat4 Trans = glm::translate(glm::mat4(1.0f), vec3(0,0,-10));
		glm::mat4 Scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.05f, 0.05f, 0.05f));
		sangle = -3.1415926;
		glm::mat4 RotX = glm::rotate(glm::mat4(1.0f), sangle, vec3(0,1,0));
        
        curTime += frametime;
        
        interp_factors(total_time, curTime, readFactorList);
		
		
		M = Trans * RotX * Scale;

		pplane->bind();
		glUniformMatrix4fv(pplane->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(pplane->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(pplane->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform3fv(pplane->getUniform("campos"), 1, &mycam.pos[0]);
        glUniform1f(pplane->getUniform("interp_factor_1"), interp_factor_1);
        glUniform1f(pplane->getUniform("interp_factor_2"), interp_factor_2);
        glUniform1f(pplane->getUniform("interp_factor_3"), interp_factor_3);
        glUniform1f(pplane->getUniform("interp_factor_4"), interp_factor_4);
        
        glBindVertexArray(VertexArrayID);

        //glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
        glDrawArrays(GL_TRIANGLES, 0, face_vertexCount);
        //glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
        pplane->unbind();
    
	}

};
//******************************************************************************************
int main(int argc, char **argv)
{
	std::string resourceDir = "../../resources"; // Where the resources are loaded from
	if (argc >= 2)
	{
		resourceDir = argv[1];
	}
    
    std::cout << "Enter a string: ";
    std::cin.getline(input, MAX_LENGTH);

	Application *application = new Application();

	/* your main will always include a similar set up to establish your window
		and GL context, etc. */
	WindowManager * windowManager = new WindowManager();
	windowManager->init(1920, 1080);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	/* This is the code that will likely change program to program as you
		may need to initialize or set up different data and state */
	// Initialize scene.
	application->init(resourceDir);
	application->initGeom(resourceDir);

	// Loop until the user closes the window.
	while(! glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Render scene.
		application->render();

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
