#define GLM_FORCE_RADIANS
#define GLM_SWIZZLE
#include "View.h"
#include <GL/glew.h>
#include <cstdlib>
#include <fstream>
#include <vector>
using namespace std;

//glm headers to access various matrix producing functions, like ortho below in resize
#include <glm/gtc/matrix_transform.hpp>
//the glm header required to convert glm objects into normal float/int pointers expected by OpenGL
//see value_ptr function below for an example
#include <glm/gtc/type_ptr.hpp>

#include "SceneXMLReader.h"

View::View()
{
    trackballTransform = glm::mat4(1.0);
	time = 0.0;
	debugBool=false;
	cameraHeight = 40;
	doMe = false;
}

View::~View()
{
    
}

void View::resize(int w, int h)
{
    //record the new dimensions
    WINDOW_WIDTH = w;
    WINDOW_HEIGHT = h;

	//cout<<"WINDOW: "<<WINDOW_WIDTH<<" "<<WINDOW_HEIGHT<<endl;

    /*
     * This program uses orthographic projection. The corresponding matrix for this projection is provided by the glm function below.
     *The last two parameters are for the near and far planes.
     *
     *Very important: the last two parameters specify the position of the near and far planes with respect
     *to the eye, in the direction of gaze. Thus positive values are in front of the camera, and negative
     *values are in the back!
     **/
   
    while (!proj.empty())
        proj.pop();

	//proj.push(glm::ortho(-200.0f,200.0f,-200.0f*WINDOW_HEIGHT/WINDOW_WIDTH,200.0f*WINDOW_HEIGHT/WINDOW_WIDTH,0.1f,10000.0f));
    proj.push(glm::perspective(120.0f*3.14159f/180,(float)WINDOW_WIDTH/WINDOW_HEIGHT,0.1f,10000.0f));
}

void View::openFile(string filename)
{

	/*if(!debugBool)
		cout<<glGetError()<<endl;*/



	SceneXMLReader reader;
	cout << "Loading...";
	reader.importScenegraph(filename,sgraph);
	cout << "Done" << endl;

	/*if(!debugBool)
		cout<<glGetError()<<endl;*/




	//GATHER THE LIGHTING AND INIT LIGHTING FOR GPU

	//cout<<"ABOUT TO GATHER LIGHTING"<<endl;
	gatheredLights = sgraph.gatherLightingObjects();
	cout<<"GOT "<<gatheredLights.size()<<" LIGHTS!"<<endl;

	//cout<<"Your Lights: "<<endl;


	//FOR DEBUGGING
	for(int i = 0 ; i < gatheredLights.size(); i++){
		glm::vec4 pos = gatheredLights[i].getPosition();	
		cout<<"Light["<<i<<"]: Position(x,y,z)"<<pos[0]<<", "<<pos[1]<<", "<<pos[2]<<endl;
	}
	
	//cout<<"Error: " <<glGetError()<<endl;

	for (int i=0;i<gatheredLights.size();i++)
    {

		LightLocation lightLocation;
        stringstream name;

        name << "light[" << i << "].ambient";

        lightLocation.ambientLocation = glGetUniformLocation(program,name.str().c_str());

        name.clear();//clear any bits set
        name.str(std::string());

        name << "light[" << i << "].diffuse";

        lightLocation.diffuseLocation = glGetUniformLocation(program,name.str().c_str());

        name.clear();//clear any bits set
        name.str(std::string());

        name << "light[" << i << "].specular";

        lightLocation.specularLocation = glGetUniformLocation(program,name.str().c_str());

        name.clear();//clear any bits set
        name.str(std::string());

        name << "light[" << i << "].position";

        lightLocation.positionLocation = glGetUniformLocation(program,name.str().c_str());

        name.clear();//clear any bits set
        name.str(std::string());


		lightLocations.push_back(lightLocation);
    }

	/*if(!debugBool)
		cout<<glGetError()<<endl;*/

	
	//cout<<glGetError()<<endl;
	//END LIGHTING GATHERING

	/*mrMarshmellow = sgraph.getMrMarshmellow();

	if(mrMarshmellow!=NULL){
		cout<<"GOT EMM!"<<endl;
		cout<<mrMarshmellow->getName()<<endl;
	}*/
}

void View::initialize()
{
    //populate our shader information. The two files below are present in this project.
    ShaderInfo shaders[] =
    {
        {GL_VERTEX_SHADER,"phong-multiple.vert"},
        {GL_FRAGMENT_SHADER,"phong-multiple.frag"},
        {GL_NONE,""} //used to detect the end of this array
    };

     program = p_program = createShaders(shaders);

	if (p_program<=0)
		exit(1);

	shaders[0].filename = "gouraud-multiple.vert";
	shaders[1].filename = "gouraud-multiple.frag";

    g_program = createShaders(shaders);

	if (g_program<=0)
		exit(1);

    glUseProgram(program);

	//cout<<glGetError()<<endl;

	projectionLocation = glGetUniformLocation(program,"projection");

	sgraph.initShaderProgram(program);

	//cout<<glGetError()<<endl;

	/*if(!debugBool)
		cout<<glGetError()<<endl;*/

	
	glUseProgram(0);
}

void View::draw()
{
	time += 0.01f;
	sgraph.animate(time);
	gatheredLights = sgraph.gatherLightingObjects();
    /*
     *The modelview matrix for the View class is going to store the world-to-view transformation
     *This effectively is the transformation that changes when the camera parameters chang
     *This matrix is provided by glm::lookAt
     */
	glUseProgram(program);

    while (!modelview.empty())
        modelview.pop();

    modelview.push(glm::mat4(1.0));

	modelview.top() = modelview.top() * glm::lookAt(glm::vec3(2,1,7),glm::vec3(0,0,0),glm::vec3(0,1,0)) * trackballTransform;

	/*if(!debugBool)
		cout<<glGetError()<<endl;*/

	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	sgraph.draw(modelview);

	/*if(!debugBool)
		cout<<glGetError()<<endl;*/


	gatheredLights = sgraph.gatherLightingObjects();
	
	glUniformMatrix4fv(projectionLocation,1,GL_FALSE,glm::value_ptr(proj.top()));

	//START LIGHTING
	glUniform1i(sgraph.numLightsLocation,gatheredLights.size());

	/*if(!debugBool)
		cout<<glGetError()<<endl;*/

	for (int i=0;i<lightLocations.size();i++)
    {
		//The equivalent to glBufferData, accept for lighting
		glUniform3fv(lightLocations[i].ambientLocation,1,glm::value_ptr(gatheredLights[i].getAmbient()));
        glUniform3fv(lightLocations[i].diffuseLocation,1,glm::value_ptr(gatheredLights[i].getDiffuse()));
        glUniform3fv(lightLocations[i].specularLocation,1,glm::value_ptr(gatheredLights[i].getSpecular()));
		glUniform4fv(lightLocations[i].positionLocation,1,glm::value_ptr(modelview.top() * gatheredLights[i].getPosition())); //Removed modelView.top() multiplication
    }
	/*if(!debugBool)
		cout<<glGetError()<<endl;*/

	//END LIGHTING

	if(doMe){
		//cout<<"WINDOW WHEN RAY TRACING: "<<WINDOW_WIDTH<<" "<<WINDOW_HEIGHT<<endl;
		this->raytrace(WINDOW_WIDTH,WINDOW_HEIGHT);
		doMe=false;
	}


    glFinish();
	glUseProgram(0);
    modelview.pop();
	debugBool=true;
}

void View::flagForRaytrace(bool doMe){

	this->doMe = doMe;

}

void View::raytrace(int width, int height){
	cout<<"Creating image..."<<endl;
	sf::Image raytraced;
	sf::Uint8* pixels = new sf::Uint8[width * height * 4];
	float* arr = sgraph.raytrace(width, height, modelview);

	//cout << "Pixels Uint8 size: " << width * height * 4 << endl;
	//cout << "Float array size: " << 

	for(int i = 0; i < width*height*4; i++){
		pixels[i] = arr[i] * 255;								//Uint8 goes from 0 - 255 ?

		//if(arr[i]!=1 && arr[i] !=0)
		//cout<<"Pixel @"<<i%4<<": "<<arr[i]<<endl;
	}

	raytraced.create(width, height, pixels);

	raytraced.flipVertically();

	raytraced.saveToFile("Raytraced.png");

	delete[] pixels;
	delete[] arr;

	cout<<"Done."<<endl;
}

void View::mousepress(int x, int y)
{
    prev_mouse = glm::vec2(x,y);
}

void View::mousemove(int x, int y)
{

    int dx,dy;

    dx = x - prev_mouse.x;
    dy = (y) - prev_mouse.y;

	if ((dx==0) && (dy==0))
		return;

    //(-dy,dx) gives the axis of rotation

    //the angle of rotation is calculated in radians by assuming that the radius of the trackball is 300
    float angle = sqrt((float)dx*dx+dy*dy)/300;
   
    prev_mouse = glm::vec2(x,y);
	
    trackballTransform = glm::rotate(glm::mat4(1.0),angle,glm::vec3(-dy,dx,0)) * trackballTransform;
}

GLuint View::createShaders(ShaderInfo *shaders)
{
    ifstream file;
    GLuint shaderProgram;
    GLint linked;

    ShaderInfo *entries = shaders;

    shaderProgram = glCreateProgram();


    while (entries->type !=GL_NONE)
    {
        file.open(entries->filename.c_str());
        GLint compiled;


        if (!file.is_open())
            return false;

        string source,line;


        getline(file,line);
        while (!file.eof())
        {
            source = source + "\n" + line;
            getline(file,line);
        }
        file.close();


        const char *codev = source.c_str();


        entries->shader = glCreateShader(entries->type);
        glShaderSource(entries->shader,1,&codev,NULL);
        glCompileShader(entries->shader);
        glGetShaderiv(entries->shader,GL_COMPILE_STATUS,&compiled);

        if (!compiled)
        {
            printShaderInfoLog(entries->shader);
            for (ShaderInfo *processed = shaders;processed->type!=GL_NONE;processed++)
            {
                glDeleteShader(processed->shader);
                processed->shader = 0;
            }
            return 0;
        }
        glAttachShader( shaderProgram, entries->shader );
        entries++;
    }

    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram,GL_LINK_STATUS,&linked);

    if (!linked)
    {
        printShaderInfoLog(entries->shader);
        for (ShaderInfo *processed = shaders;processed->type!=GL_NONE;processed++)
        {
            glDeleteShader(processed->shader);
            processed->shader = 0;
        }
        return 0;
    }

    return shaderProgram;
}

void View::printShaderInfoLog(GLuint shader)
{
    int infologLen = 0;
    int charsWritten = 0;
    GLubyte *infoLog;

    glGetShaderiv(shader,GL_INFO_LOG_LENGTH,&infologLen);
    //	printOpenGLError();
    if (infologLen>0)
    {
        infoLog = (GLubyte *)malloc(infologLen);
        if (infoLog != NULL)
        {
            glGetShaderInfoLog(shader,infologLen,&charsWritten,(char *)infoLog);
            printf("InfoLog: %s\n\n",infoLog);
            free(infoLog);
        }

    }
//	printOpenGLError();
}

void View::getOpenGLVersion(int *major,int *minor)
{
    const char *verstr = (const char *)glGetString(GL_VERSION);
    if ((verstr == NULL) || (sscanf_s(verstr,"%d.%d",major,minor)!=2))
    {
        *major = *minor = 0;
    }
}

void View::getGLSLVersion(int *major,int *minor)
{
    int gl_major,gl_minor;

    getOpenGLVersion(&gl_major,&gl_minor);
    *major = *minor = 0;

    if (gl_major==1)
    {
        /* GL v1.x can only provide GLSL v1.00 as an extension */
        const char *extstr = (const char *)glGetString(GL_EXTENSIONS);
        if ((extstr!=NULL) && (strstr(extstr,"GL_ARB_shading_language_100")!=NULL))
        {
            *major = 1;
            *minor = 0;
        }
    }
    else if (gl_major>=2)
    {
        /* GL v2.0 and greater must parse the version string */
        const char *verstr = (const char *)glGetString(GL_SHADING_LANGUAGE_VERSION);
        if ((verstr==NULL) || (sscanf_s(verstr,"%d.%d",major,minor) !=2))
        {
            *major = 0;
            *minor = 0;
        }
    }
}












