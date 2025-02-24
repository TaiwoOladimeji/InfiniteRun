#include <iostream>
#include <GL/glew.h>
#include <3dgl/3dgl.h>
#include <GL/glut.h>
#include <GL/freeglut_ext.h>

// Include GLM core features
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#pragma comment (lib, "glew32.lib")

using namespace std;
using namespace _3dgl;
using namespace glm;

// 3D Models
C3dglModel street;
C3dglSkyBox skybox;
C3dglModel Aj;			// the boy's name is Aj
C3dglModel idle, run;	// additional animations (skinless)


// GLSL Objects (Shader Program)
C3dglProgram program;

//Skybox day light
GLuint dayLight = 1;
// light attenuation
GLuint lightAtt = 0;

// The View Matrix
mat4 matrixView;

// Camera & navigation
float maxspeed = 4.f;	// camera max speed
float accel = 4.f;		// camera acceleration
vec3 _acc(0), _vel(0);	// camera acceleration and velocity vectors
float _fov = 60.f;		// field of view (zoom)

void initLights(GLuint dayLight)
{
	program.sendUniform("lightAmbient.color", vec3(dayLight * 0.1, dayLight * 0.1, dayLight * 0.1));
	program.sendUniform("lightEmissive.color", vec3(0.0, 0.0, 0.0));
	program.sendUniform("lightDir.direction", vec3(1.0, 0.5, 1.0));
	program.sendUniform("lightDir.diffuse", vec3(dayLight * 1.0, dayLight * 1.0, dayLight * 1.0));

	GLuint nightLight = 1 - dayLight;
	program.sendUniform("lightPoint1.diffuse", vec3(nightLight * 0.5, nightLight * 0.5, nightLight * 0.5));
	program.sendUniform("lightPoint1.specular", vec3(nightLight * 1.0, nightLight * 1.0, nightLight * 1.0));
	program.sendUniform("lightPoint2.diffuse", vec3(nightLight * 0.5, nightLight * 0.5, nightLight * 0.5));
	program.sendUniform("lightPoint2.specular", vec3(nightLight * 1.0, nightLight * 1.0, nightLight * 1.0));
	program.sendUniform("lightPoint3.diffuse", vec3(nightLight * 0.5, nightLight * 0.5, nightLight * 0.5));
	program.sendUniform("lightPoint3.specular", vec3(nightLight * 1.0, nightLight * 1.0, nightLight * 1.0));
	program.sendUniform("lightPoint4.diffuse", vec3(nightLight * 0.5, nightLight * 0.5, nightLight * 0.5));
	program.sendUniform("lightPoint4.specular", vec3(nightLight * 1.0, nightLight * 1.0, nightLight * 1.0));
	program.sendUniform("lightPoint5.diffuse", vec3(nightLight * 0.5, nightLight * 0.5, nightLight * 0.5));
	program.sendUniform("lightPoint5.specular", vec3(nightLight * 1.0, nightLight * 1.0, nightLight * 1.0));
}

bool init()
{
	// rendering states
	glEnable(GL_DEPTH_TEST);	// depth test is necessary for most 3D scenes
	glEnable(GL_NORMALIZE);		// normalization is needed by AssImp library models
	glShadeModel(GL_SMOOTH);	// smooth shading mode is the default one; try GL_FLAT here!
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	// this is the default one; try GL_LINE!

	// Enable Alpha Blending
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Initialise Shaders
	C3dglShader vertexShader;
	C3dglShader fragmentShader;

	if (!vertexShader.create(GL_VERTEX_SHADER)) return false;
	if (!vertexShader.loadFromFile("shaders/basic.vert")) return false;
	if (!vertexShader.compile()) return false;

	if (!fragmentShader.create(GL_FRAGMENT_SHADER)) return false;
	if (!fragmentShader.loadFromFile("shaders/basic.frag")) return false;
	if (!fragmentShader.compile()) return false;

	if (!program.create()) return false;
	if (!program.attach(vertexShader)) return false;
	if (!program.attach(fragmentShader)) return false;
	if (!program.link()) return false;
	if (!program.use(true)) return false;

	// create & load textures
	glActiveTexture(GL_TEXTURE0);

	// glut additional setup
	glutSetVertexAttribCoord3(program.getAttribLocation("aVertex"));
	glutSetVertexAttribNormal(program.getAttribLocation("aNormal"));




	// load your 3D models here!
	street.load("models\\street\\Street environment_V01.obj");
	street.loadMaterials("models\\street\\");

	idle.load("models\\idle.fbx");
	run.load("models\\run.fbx");

	Aj.load("models\\Aj.fbx");
	Aj.loadMaterials("models\\");
	Aj.loadAnimations(&idle);	// Aj model has no animations
	Aj.loadAnimations(&run);	// but can load them from idle and run

	// load Sky Box
	if (!skybox.load("models\\TropicalSunnyDay\\TropicalSunnyDayFront1024.jpg",
		"models\\TropicalSunnyDay\\TropicalSunnyDayLeft1024.jpg",
		"models\\TropicalSunnyDay\\TropicalSunnyDayBack1024.jpg",
		"models\\TropicalSunnyDay\\TropicalSunnyDayRight1024.jpg",
		"models\\TropicalSunnyDay\\TropicalSunnyDayUp1024.jpg",
		"models\\TropicalSunnyDay\\TropicalSunnyDayDown1024.jpg")) return false;

	// Send the texture info to the shaders
	program.sendUniform("texture0", 0);

	// setup lights:
	initLights(dayLight);
	program.sendUniform("lightAttOn", lightAtt);

	// setup lights:
	program.sendUniform("lightAmbient.color", vec3(0.1, 0.1, 0.1));
	program.sendUniform("lightDir1.direction", vec3(0.75, 2.0, -1.0));
	program.sendUniform("lightDir1.diffuse", vec3(1.0, 1.0, 1.0));
	program.sendUniform("lightDir2.direction", vec3(-0.75, -2.0, 1.0));
	program.sendUniform("lightDir2.diffuse", vec3(0.1, 0.1, 0.2));

	// setup materials
	program.sendUniform("materialAmbient", vec3(1.0, 1.0, 1.0));
	program.sendUniform("materialDiffuse", vec3(1.0, 1.0, 1.0));

	// Initialise the View Matrix (initial position of the camera)
	matrixView = rotate(mat4(1), radians(12.f), vec3(1, 0, 0));
	matrixView *= lookAt(
		vec3(0, 1.5, 0.0),
		vec3(0, 1.5, 10.0),
		vec3(0.0, 1.0, 0.0));

	// setup the screen background colour
	glClearColor(0.2f, 0.6f, 1.f, 1.0f);   // blue sky colour

	cout << endl;
	cout << "Use:" << endl;
	cout << "  WASD or arrow key to navigate" << endl;
	cout << "  QE or PgUp/Dn to move the camera up and down" << endl;
	cout << "  Drag the mouse to look around" << endl;
	cout << endl;

	return true;
}

void renderScene(mat4& matrixView, float time, float deltaTime)
{
	mat4 m;

	program.sendUniform("lightAttOn", lightAtt);

	if (dayLight)
	{
		// prepare ambient light for the skybox
		program.sendUniform("lightAmbient.color", vec3(1.0, 1.0, 1.0));
		program.sendUniform("materialAmbient", vec3(1.0, 1.0, 1.0));
		program.sendUniform("materialDiffuse", vec3(0.0, 0.0, 0.0));

		// render the skybox
		m = matrixView;
		skybox.render(m);

		// revert normal light after skybox
		program.sendUniform("lightAmbient.color", vec3(0.4, 0.4, 0.4));
	}

	// Camera position
	vec3 pos = getPos(matrixView);

	// This vector automatically amends the Y-coordinate of the wolf according to the terrain elevation
	//vec3 amendY = vec3(vec3(0, street.getInterpolatedHeight(wolfPos.x, wolfPos.z), 0));
 
	// render the street
	program.sendUniform("lightAmbient.color", vec3(0.4, 0.4, 0.4));
	m = matrixView;
	m = translate(m, vec3(0, -0.07, 47));	// 47 is half of the depth of the town tile
	street.render(m);
	program.sendUniform("lightAmbient.color", vec3(0.4, 0.4, 0.4));
	m = matrixView;
	m = translate(m, vec3(0, -0.07, 94));
	street.render(m,1);
	program .sendUniform("lightAmbient.color", vec3(0.4, 0.4, 0.4));
	m = matrixView;
	m = translate(m, vec3(0, -0.07, 188));
	street.render(m,2);
	program.sendUniform("lightAmbient.color", vec3(0.4, 0.4, 0.4));
	m = matrixView;
	m = translate(m, vec3(0, -0.07, 376));
	street.render(m,3);
	program.sendUniform("lightAmbient.color", vec3(0.4, 0.4, 0.4));
	m = matrixView;
	m = translate(m, vec3(0, -0.07, 752));
	street.render(m,4);
	program.sendUniform("lightAmbient.color", vec3(0.4, 0.4, 0.4));
	m = matrixView;
	m = translate(m, vec3(0, -0.07, 1504));
	street.render(m,5);
	program.sendUniform("lightAmbient.color", vec3(0.4, 0.4, 0.4));
	m = matrixView;
	m = translate(m, vec3(0, -0.07, 3008));
	street.render(m,6);
	program.sendUniform("lightAmbient.color", vec3(0.4, 0.4, 0.4));
	m = matrixView;
	m = translate(m, vec3(0, -0.07, 6016));
	street.render(m,7);

	// render Aj
	m = matrixView;
	m = translate(matrixView, vec3(-0.00, 0.0, 2));
	m = scale(m, vec3(0.01f, 0.01f, 0.01f));
	Aj.render(m);



	// Diagnostic strings
	print(0, 0, std::format("Camera position: ({:.2f}, {:.2f}, {:.2f})", pos.x, pos.y, pos.z));
	print(0, 20, std::format("Velocity: {:.2f}", _vel.z));
	print(0, 40, deltaTime);
}

void onRender()
{
	// these variables control time & animation
	static float prev = 0;
	float time = glutGet(GLUT_ELAPSED_TIME) * 0.001f;	// time since start in seconds
	float deltaTime = time - prev;						// time since last frame
	prev = time;										// framerate is 1/deltaTime

	// clear screen and buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// setup the View Matrix (camera)
	_vel = clamp(_vel + _acc * deltaTime, vec3(0, -maxspeed, 0), vec3(0, maxspeed, maxspeed));
	matrixView = translate(matrixView, -_vel * deltaTime);

	// setup View Matrix
	program.sendUniform("matrixView", matrixView);

	// render the scene objects
	renderScene(matrixView, time, deltaTime);

	// essential for double-buffering technique
	glutSwapBuffers();

	// proceed the animation
	glutPostRedisplay();
}

// called before window opened or resized - to setup the Projection Matrix
void onReshape(int w, int h)
{
	float ratio = w * 1.0f / h;      // we hope that h is not zero
	glViewport(0, 0, w, h);
	program.sendUniform("matrixProjection", perspective(radians(_fov), ratio, 0.02f, 1000.f));
}

// Handle WASDQE keys
void onKeyDown(unsigned char key, int x, int y)
{
	switch (tolower(key))
	{
	case 'w': _acc.z = accel; break;
	//case 's': _acc.z = -accel; break;
	//case 'a': _acc.x = accel; break;
	//case 'd': _acc.x = -accel; break;
	case 'e': _acc.y = accel; break;
	case 'q': _acc.y = -accel; break;
	}
}

// Handle WASDQE keys (key up)
void onKeyUp(unsigned char key, int x, int y)
{
	switch (tolower(key))
	{
	case 'w': _acc.z = -accel; break;
	//case 's': _acc.z = accel; break;
	//case 'a': _acc.x = -accel; break;
	//case 'd': _acc.x = accel; break;
	case 'q':
	case 'e': _acc.y = _vel.y = 0; break;
	}
}

// Handle arrow keys and Alt+F4
void onSpecDown(int key, int x, int y)
{
	maxspeed = glutGetModifiers() & GLUT_ACTIVE_SHIFT ? 20.f : 4.f;
	switch (key)
	{
	case GLUT_KEY_F4:		if ((glutGetModifiers() & GLUT_ACTIVE_ALT) != 0) exit(0); break;
	case GLUT_KEY_UP:		onKeyDown('w', x, y); break;
	case GLUT_KEY_DOWN:		onKeyDown('s', x, y); break;
	case GLUT_KEY_LEFT:		onKeyDown('a', x, y); break;
	case GLUT_KEY_RIGHT:	onKeyDown('d', x, y); break;
	case GLUT_KEY_PAGE_UP:	onKeyDown('q', x, y); break;
	case GLUT_KEY_PAGE_DOWN:onKeyDown('e', x, y); break;
	case GLUT_KEY_F11:		glutFullScreenToggle();
	}
}

// Handle arrow keys (key up)
void onSpecUp(int key, int x, int y)
{
	maxspeed = glutGetModifiers() & GLUT_ACTIVE_SHIFT ? 20.f : 4.f;
	switch (key)
	{
	case GLUT_KEY_UP:		onKeyUp('w', x, y); break;
	case GLUT_KEY_DOWN:		onKeyUp('s', x, y); break;
	case GLUT_KEY_LEFT:		onKeyUp('a', x, y); break;
	case GLUT_KEY_RIGHT:	onKeyUp('d', x, y); break;
	case GLUT_KEY_PAGE_UP:	onKeyUp('q', x, y); break;
	case GLUT_KEY_PAGE_DOWN:onKeyUp('e', x, y); break;
	}
}

// Handle mouse click
void onMouse(int button, int state, int x, int y)
{
	glutSetCursor(state == GLUT_DOWN ? GLUT_CURSOR_CROSSHAIR : GLUT_CURSOR_INHERIT);
	glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) / 2, glutGet(GLUT_WINDOW_HEIGHT) / 2);
	if (button == 1)
	{
		_fov = 60.0f;
		onReshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	}
}

// handle mouse move
void onMotion(int x, int y)
{
	glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) / 2, glutGet(GLUT_WINDOW_HEIGHT) / 2);

	// find delta (change to) pan & pitch
	float deltaYaw = 0.005f * (x - glutGet(GLUT_WINDOW_WIDTH) / 2);
	float deltaPitch = 0.005f * (y - glutGet(GLUT_WINDOW_HEIGHT) / 2);

	if (abs(deltaYaw) > 0.3f || abs(deltaPitch) > 0.3f)
		return;	// avoid warping side-effects

	// View = Pitch * DeltaPitch * DeltaYaw * Pitch^-1 * View;
	constexpr float maxPitch = radians(80.f);
	float pitch = getPitch(matrixView);
	float newPitch = glm::clamp(pitch + deltaPitch, -maxPitch, maxPitch);
	matrixView = rotate(rotate(rotate(mat4(1.f),
		newPitch, vec3(1.f, 0.f, 0.f)),
		deltaYaw, vec3(0.f, 1.f, 0.f)), 
		-pitch, vec3(1.f, 0.f, 0.f)) 
		* matrixView;
}

void onMouseWheel(int button, int dir, int x, int y)
{
	_fov = glm::clamp(_fov - dir * 5.f, 5.0f, 175.f);
	onReshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
}

int main(int argc, char **argv)
{
	// init GLUT and create Window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(1280, 720);
	glutCreateWindow("3DGL Scene: Infinite Run");

	// init glew
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		C3dglLogger::log("GLEW Error {}", (const char*)glewGetErrorString(err));
		return 0;
	}
	C3dglLogger::log("Using GLEW {}", (const char*)glewGetString(GLEW_VERSION));

	// register callbacks
	glutDisplayFunc(onRender);
	glutReshapeFunc(onReshape);
	glutKeyboardFunc(onKeyDown);
	glutSpecialFunc(onSpecDown);
	glutKeyboardUpFunc(onKeyUp);
	glutSpecialUpFunc(onSpecUp);
	glutMouseFunc(onMouse);
	glutMotionFunc(onMotion);
	glutMouseWheelFunc(onMouseWheel);

	C3dglLogger::log("Vendor: {}", (const char *)glGetString(GL_VENDOR));
	C3dglLogger::log("Renderer: {}", (const char *)glGetString(GL_RENDERER));
	C3dglLogger::log("Version: {}", (const char*)glGetString(GL_VERSION));
	C3dglLogger::log("");

	// init light and everything – not a GLUT or callback function!
	if (!init())
	{
		C3dglLogger::log("Application failed to initialise\r\n");
		return 0;
	}

	// enter GLUT event processing cycle
	glutMainLoop();

	return 1;
}

