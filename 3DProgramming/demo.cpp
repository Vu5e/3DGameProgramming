#define GLFW_INCLUDE_ES2 1
#define GLFW_DLL 1
//#define GLFW_EXPOSE_NATIVE_WIN32 1
//#define GLFW_EXPOSE_NATIVE_EGL 1

#include <GLES2/gl2.h>
#include <EGL/egl.h>

#include <GLFW/glfw3.h>
//#include <GLFW/glfw3native.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <fstream>

#include "angle_util/Matrix.h"
#include "angle_util/geometry_utils.h"
#include "bitmap.h"
#include <fmod.hpp>
#include <fmod_errors.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define TEXTURE_COUNT 13

//Must be power of 2
#define SPECTRUM_SIZE 1024
#define SPECTRUM_EXP_2 10

GLint GprogramID = -1;
GLuint GtextureID[TEXTURE_COUNT];

GLuint Gframebuffer;
GLuint GdepthRenderbuffer;

GLuint GfullscreenTexture;
GLuint GpTexture_0;
GLuint GpTexture_1;

GLFWwindow* window;

Matrix4 gPerspectiveMatrix;
Matrix4 gViewMatrix;

float m_spectrumLeft[SPECTRUM_SIZE];
float m_spectrumRight[SPECTRUM_SIZE];

float m_highestSpectrumLeft[SPECTRUM_SIZE];
float m_highestSpectrumRight[SPECTRUM_SIZE];

float m_highestSpectrum[SPECTRUM_EXP_2];
float m_spectrum[SPECTRUM_EXP_2];

float rotateSpeed = 360.0f;

static void error_callback(int error, const char* description)
{
	fputs(description, stderr);
}

//FMOD Error Check
void ERRCHECK(FMOD_RESULT result)
{
	if (result != FMOD_OK)
	{
		printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
	}
}

FMOD::System* m_fmodSystem;
FMOD::Sound* m_music;
FMOD::Channel* m_musicChannel;

GLuint LoadShader(GLenum type, const char *shaderSrc)
{
	GLuint shader;
	GLint compiled;

	// Create the shader object
	shader = glCreateShader(type);

	if (shader == 0)
		return 0;

	// Load the shader source
	glShaderSource(shader, 1, &shaderSrc, NULL);

	// Compile the shader
	glCompileShader(shader);

	// Check the compile status
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

	if (!compiled)
	{
		GLint infoLen = 0;

		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);

		if (infoLen > 1)
		{
			char infoLog[4096];
			glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
			printf("Error compiling shader:\n%s\n", infoLog);
		}

		glDeleteShader(shader);
		return 0;
	}

	return shader;
}

GLuint LoadShaderFromFile(GLenum shaderType, std::string path)
{
	GLuint shaderID = 0;
	std::string shaderString;
	std::ifstream sourceFile(path.c_str());

	if (sourceFile)
	{
		shaderString.assign((std::istreambuf_iterator< char >(sourceFile)), std::istreambuf_iterator< char >());
		const GLchar* shaderSource = shaderString.c_str();

		return LoadShader(shaderType, shaderSource);
	}
	else
		printf("Unable to open file %s\n", path.c_str());

	return shaderID;
}


void loadTexture(const char* path, GLuint textureID)
{
	CBitmap bitmap(path);

	glBindTexture(GL_TEXTURE_2D, textureID);

	// Repeat the texture after exceeding 1.0f
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // Apply texture wrapping along horizontal part
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // Apply texture wrapping along vertical part

																  // Bilinear filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Near filtering (For when texture needs to scale...)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Far filtering (For when texture needs to scale...)

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bitmap.GetWidth(), bitmap.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bitmap.GetBits());
}

void initFmod()
{
	FMOD_RESULT result;
	unsigned int version;

	result = FMOD::System_Create(&m_fmodSystem);
	ERRCHECK(result);

	result = m_fmodSystem->getVersion(&version);
	ERRCHECK(result);

	if (version < FMOD_VERSION)
	{
		printf("FMOD Error! You are using an old version of FMOD.", version, FMOD_VERSION);
	}

	//Initialise fmod system
	result = m_fmodSystem->init(32, FMOD_INIT_NORMAL, 0);
	ERRCHECK(result);

	//Load and Set up Music
	result = m_fmodSystem->createStream("../media/entrance.mp3", FMOD_SOFTWARE, 0, &m_music);
	ERRCHECK(result);

	//Play the loaded mp3 music
	result = m_fmodSystem->playSound(FMOD_CHANNEL_FREE, m_music, false, &m_musicChannel);
	ERRCHECK(result);
}

void updateFmod()
{
	m_fmodSystem->update();

	//Get spectrum for left and right stereo channels
	m_musicChannel->getSpectrum(m_spectrumLeft, SPECTRUM_SIZE, 0, FMOD_DSP_FFT_WINDOW_RECT);
	m_musicChannel->getSpectrum(m_spectrumRight, SPECTRUM_SIZE, 1, FMOD_DSP_FFT_WINDOW_RECT);

	//Legacy system
	/*
	for (int i = 0; i < SPECTRUM_SIZE; i++)
	{
	if (m_spectrumLeft[i] > m_highestSpectrumLeft[i])
	m_highestSpectrumLeft[i] = m_spectrumLeft[i];
	if (m_spectrumRight[i] > m_highestSpectrumRight[i])
	m_highestSpectrumRight[i] = m_spectrumRight[i];

	//Reduce by delta time
	m_highestSpectrumLeft[i] -= 0.005f;
	m_highestSpectrumRight[i] -= 0.005f;
	if (m_highestSpectrumLeft[i] < 0.0f) m_highestSpectrumLeft[i] = 0.0f;
	if (m_highestSpectrumRight[i] < 0.0f) m_highestSpectrumRight[i] = 0.0f;
	}
	*/

	int exp = 0;
	for (int i = 1; i < SPECTRUM_SIZE; i *= 2)
	{
		m_spectrum[exp] = 0.0f;
		for (int j = i - 1; j < (i * 2); j++)
		{
			m_spectrum[exp] += (m_spectrumLeft[j] + m_spectrumRight[j]) / 2.0f;
		}

		if (exp > 0)
			m_spectrum[exp] = m_spectrum[exp - 1];

		if (m_spectrum[exp] > m_highestSpectrum[exp])
			m_highestSpectrum[exp] = m_spectrum[exp];

		//Reduce by delta time
		m_highestSpectrum[exp] -= 0.05f * m_highestSpectrum[exp];
		if (m_highestSpectrum[exp] < 0.0f) m_highestSpectrum[exp] = 0.0f;

		exp++;
	}
}

int Init(void)
{
	GLuint vertexShader;
	GLuint fragmentShader;
	GLuint programObject;
	GLint linked;

	// Load Textures
	glGenTextures(TEXTURE_COUNT, GtextureID);
	loadTexture("../media/rocks.bmp", GtextureID[0]);
	loadTexture("../media/rainbow-blocks.bmp", GtextureID[1]);
	loadTexture("../media/rgb.bmp", GtextureID[2]);
	loadTexture("../media/barack-obama.bmp", GtextureID[3]);
	loadTexture("../media/rainbow-stripes.bmp", GtextureID[4]);
	loadTexture("../media/gtx-promo.bmp", GtextureID[5]);
	loadTexture("../media/Top.bmp", GtextureID[6]);
	loadTexture("../media/Back.bmp", GtextureID[7]);
	loadTexture("../media/Left.bmp", GtextureID[8]);
	loadTexture("../media/Down.bmp", GtextureID[9]);
	loadTexture("../media/Front.bmp", GtextureID[10]);
	loadTexture("../media/Right.bmp", GtextureID[11]);
	loadTexture("../media/pepe.bmp", GtextureID[12]);

	// Initialize FMOD
	//initFmod();

	// Create a new FBO (Frame Bufffer Object)
	glGenFramebuffers(1, &Gframebuffer);

	// Create a new empty texture for rendering original scene
	glGenTextures(1, &GfullscreenTexture);
	glBindTexture(GL_TEXTURE_2D, GfullscreenTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// Create 2 new empty textures for processing textures
	glGenTextures(1, &GpTexture_0);
	glBindTexture(GL_TEXTURE_2D, GpTexture_0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glGenTextures(1, &GpTexture_1);
	glBindTexture(GL_TEXTURE_2D, GpTexture_1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// Create and bind render buffer, and create a 16-bit depth buffer
	glGenRenderbuffers(1, &GdepthRenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, GdepthRenderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, WINDOW_WIDTH, WINDOW_HEIGHT);

	// Load Shaders
	vertexShader = LoadShaderFromFile(GL_VERTEX_SHADER, "../vertexShader1.vert");
	fragmentShader = LoadShaderFromFile(GL_FRAGMENT_SHADER, "../fragmentShader1.frag");

	// Create the program object
	programObject = glCreateProgram();

	if (programObject == 0)
		return 0;

	glAttachShader(programObject, fragmentShader);
	glAttachShader(programObject, vertexShader);

	// (Send from CPU to GPU)
	// Bind vPosition to attribute 0
	glBindAttribLocation(programObject, 0, "vPosition");
	// Bind vColor to attribute 1   
	glBindAttribLocation(programObject, 1, "vColor");
	// Bind vPosition to attribute 2
	glBindAttribLocation(programObject, 2, "vTexCoord");

	// Link the program
	glLinkProgram(programObject);

	// Check the link status
	glGetProgramiv(programObject, GL_LINK_STATUS, &linked);

	if (!linked)
	{
		GLint infoLen = 0;

		glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &infoLen);

		if (infoLen > 1)
		{
			//char* infoLog = malloc (sizeof(char) * infoLen );
			char infoLog[512];
			glGetProgramInfoLog(programObject, infoLen, NULL, infoLog);
			printf("Error linking program:\n%s\n", infoLog);

			//free ( infoLog );
		}

		glDeleteProgram(programObject);
		return 0;
	}

	// Store the program object
	GprogramID = programObject;

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//Initialize Matrices
	gPerspectiveMatrix = Matrix4::perspective
	(
		60.0f,
		(float)WINDOW_WIDTH / (float)WINDOW_HEIGHT,
		0.5f, 30.0f
	);

	gViewMatrix = Matrix4::translate(Vector3(0.0f, 0.0f, -2.0f));

	return 1;
}

void UpdateCamera(void)
{
	static float yaw = 0.0f;
	static float pitch = 0.0f;
	static float distance = 1.5f;

	if (glfwGetKey(window, 'A')) pitch -= 1.0f;
	if (glfwGetKey(window, 'D')) pitch += 1.0f;
	if (glfwGetKey(window, 'W')) yaw -= 1.0f;
	if (glfwGetKey(window, 'S')) yaw += 1.0f;

	if (glfwGetKey(window, 'R'))
	{
		distance -= 0.06f;
		if (distance < 1.0f)
			distance = 1.0f;
	}
	if (glfwGetKey(window, 'F')) distance += 0.06f;

	gViewMatrix = Matrix4::translate
	(
		Vector3(0.0f, 0.0f, -distance)) *
		Matrix4::rotate(yaw, Vector3(1.0f, 0.0f, 0.0f)) *
		Matrix4::rotate(pitch, Vector3(0.0f, 1.0f, 0.0f)
		);
}

void DrawSquare(GLuint texture)
{
	static GLfloat vVertices[] =
	{
		-1.0f,  1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,
		 1.0f, -1.0f,  0.0f,
		 1.0f,  -1.0f, 0.0f,
		 1.0f, 1.0f, 0.0f,
		-1.0f, 1.0f,  0.0f
	};

	static GLfloat vColors[] = // !! Color for each vertex
	{
		1.0f, 0.0f, 0.0f, 1.0f,
		0.0f, 1.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 0.0f, 1.0f
	};

	static GLfloat vTexCoords[] = // !! TexCoord for each vertex
	{
		0.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f
	};

	glBindTexture(GL_TEXTURE_2D, texture);


	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vVertices);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, vColors);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, vTexCoords);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);


	glDrawArrays(GL_TRIANGLES, 0, 6);


	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
}

void DrawPepeCube(int num)
{
	float radius = 1.5f;
	float radius2 = 0.25f;
	float size = 0.1f;

	for (int i = 0; i < num; i++)
	{
		for (int j = 0; j < num; j++)
		{
			Matrix4 modelMatrix, mvpMatrix;
			modelMatrix = Matrix4::translate(Vector3(sinf(rotateSpeed + 360.0f * i / (num + 1)) * radius + sinf(rotateSpeed * 20.0f + 360.0f * j / (num + 1)) * radius2,
				sinf(rotateSpeed * 50.0f + (180.0f * num) * j / (num + 1)) * radius2 + size,
				cosf(rotateSpeed + 360.0f * i / (num + 1)) * radius + cosf(rotateSpeed * 20.0f + 360.0f * j / (num + 1)) * radius2)) *
				Matrix4::rotate(90, Vector3(1.0f, 0.0f, 0.0f)) *
				Matrix4::scale(Vector3(size, size, 1.0f));
			mvpMatrix = gPerspectiveMatrix * gViewMatrix * modelMatrix;
			glUniformMatrix4fv(glGetUniformLocation(GprogramID, "uMvpMatrix"), 1, GL_FALSE, mvpMatrix.data);
			DrawSquare(GtextureID[12]); //draw top rectangle

			modelMatrix = Matrix4::translate(Vector3(sinf(rotateSpeed + 360.0f * i / (num + 1)) * radius + sinf(rotateSpeed * 20.0f + 360.0f * j / (num + 1)) * radius2,
				sinf(rotateSpeed * 50.0f + (180.0f * num) * j / (num + 1)) * radius2,
				cosf(rotateSpeed + 360.0f * i / (num + 1)) * radius + cosf(rotateSpeed * 20.0f + 360.0f * j / (num + 1)) * radius2 - size)) *
				Matrix4::rotate(0, Vector3(0.0f, 1.0f, 0.0f)) *
				Matrix4::scale(Vector3(size, size, 1.0f));
			mvpMatrix = gPerspectiveMatrix * gViewMatrix * modelMatrix;
			glUniformMatrix4fv(glGetUniformLocation(GprogramID, "uMvpMatrix"), 1, GL_FALSE, mvpMatrix.data);
			DrawSquare(GtextureID[12]); //draw front rectangle

			modelMatrix = Matrix4::translate(Vector3(sinf(rotateSpeed + 360.0f * i / (num + 1)) * radius + sinf(rotateSpeed * 20.0f + 360.0f * j / (num + 1)) * radius2,
				sinf(rotateSpeed * 50.0f + (180.0f * num) * j / (num + 1)) * radius2,
				cosf(rotateSpeed + 360.0f * i / (num + 1)) * radius + cosf(rotateSpeed * 20.0f + 360.0f * j / (num + 1)) * radius2 + size)) *
				Matrix4::rotate(180, Vector3(0.0f, 1.0f, 0.0f)) *
				Matrix4::scale(Vector3(size, size, 1.0f));
			mvpMatrix = gPerspectiveMatrix * gViewMatrix * modelMatrix;
			glUniformMatrix4fv(glGetUniformLocation(GprogramID, "uMvpMatrix"), 1, GL_FALSE, mvpMatrix.data);
			DrawSquare(GtextureID[12]); //draw back rectangle

			modelMatrix = Matrix4::translate(Vector3(sinf(rotateSpeed + 360.0f * i / (num + 1)) * radius + sinf(rotateSpeed * 20.0f + 360.0f * j / (num + 1)) * radius2,
				sinf(rotateSpeed * 50.0f + (180.0f * num) * j / (num + 1)) * radius2 - size,
				cosf(rotateSpeed + 360.0f * i / (num + 1)) * radius + cosf(rotateSpeed * 20.0f + 360.0f * j / (num + 1)) * radius2)) *
				Matrix4::rotate(-90, Vector3(1.0f, 0.0f, 0.0f)) *
				Matrix4::scale(Vector3(size, size, 1.0f));
			mvpMatrix = gPerspectiveMatrix * gViewMatrix * modelMatrix;
			glUniformMatrix4fv(glGetUniformLocation(GprogramID, "uMvpMatrix"), 1, GL_FALSE, mvpMatrix.data);
			DrawSquare(GtextureID[12]); //draw bottom rectangle

			modelMatrix = Matrix4::translate(Vector3(sinf(rotateSpeed + 360.0f * i / (num + 1)) * radius + sinf(rotateSpeed * 20.0f + 360.0f * j / (num + 1)) * radius2 - size,
				sinf(rotateSpeed * 50.0f + (180.0f * num) * j / (num + 1)) * radius2,
				cosf(rotateSpeed + 360.0f * i / (num + 1)) * radius + cosf(rotateSpeed * 20.0f + 360.0f * j / (num + 1)) * radius2)) *
				Matrix4::rotate(90, Vector3(0.0f, 1.0f, 0.0f)) *
				Matrix4::scale(Vector3(size, size, 1.0f));
			mvpMatrix = gPerspectiveMatrix * gViewMatrix * modelMatrix;
			glUniformMatrix4fv(glGetUniformLocation(GprogramID, "uMvpMatrix"), 1, GL_FALSE, mvpMatrix.data);
			DrawSquare(GtextureID[12]); //draw left rectangle

			modelMatrix = Matrix4::translate(Vector3(sinf(rotateSpeed + 360.0f * i / (num + 1)) * radius + sinf(rotateSpeed * 20.0f + 360.0f * j / (num + 1)) * radius2 + size,
				sinf(rotateSpeed * 50.0f + (180.0f * num) * j / (num + 1)) * radius2,
				cosf(rotateSpeed + 360.0f * i / (num + 1)) * radius + cosf(rotateSpeed * 20.0f + 360.0 * j / (num + 1)) * radius2)) *
				Matrix4::rotate(-90, Vector3(0.0f, 1.0f, 0.0f)) *
				Matrix4::scale(Vector3(size, size, 1.0f));
			mvpMatrix = gPerspectiveMatrix * gViewMatrix * modelMatrix;
			glUniformMatrix4fv(glGetUniformLocation(GprogramID, "uMvpMatrix"), 1, GL_FALSE, mvpMatrix.data);
			DrawSquare(GtextureID[12]); //draw right rectangle
		}
	}

}

void Draw(void)
{
	rotateSpeed += 0.001f;
	// Use the program object, it's possible that you have multiple shader programs and switch it accordingly
	glUseProgram(GprogramID);

	// Set the sampler2D varying variable to the first texture unit (index 0)
	glUniform1i(glGetUniformLocation(GprogramID, "sampler2d"), 0);

	// Pass texture size to shader
	glUniform2fv(glGetUniformLocation(GprogramID, "resolution"), 1, new GLfloat[2]{ (GLfloat)WINDOW_WIDTH, (GLfloat)WINDOW_HEIGHT });

	// Time
	static float time = 0.0f;
	time += 0.01f;
	GLint timeLoc = glGetUniformLocation(GprogramID, "Time");
	if (timeLoc != -1)
	{
		glUniform1f(timeLoc, time);
	}

	// Update Camera
	UpdateCamera();

	// Set the viewport
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

	// Clear the buffers (Clear the screen basically)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GLenum status;
	//******************************************************************
	// To render the entire screen texture
	// Use the modelMatrix :: translate, rotate, scale
	// Then draw the texture using the DrawSquare(GtextureID[6]
	// This can be also do in function
	// Here for bloom + blur effect
	//******************************************************************
	// Bind the framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, Gframebuffer);

	// Specify texture as color attachment
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, GfullscreenTexture, 0);

	// specify depth_renderbufer as depth attachment
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, GdepthRenderbuffer);

	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status == GL_FRAMEBUFFER_COMPLETE)
	{
		// Clear the buffers (Clear the screen basically)
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Set state to normal
		glUniform1i(glGetUniformLocation(GprogramID, "uState"), -1);

		// Set to no blur
		glUniform1i(glGetUniformLocation(GprogramID, "uBlurDirection"), -1);

		Matrix4 modelMatrix, mvpMatrix;

		// Draw Top Side
		modelMatrix = Matrix4::translate(Vector3(0.0f, 4.0f, 0.0f)) *
			Matrix4::rotate(90, Vector3(1.0f, 0.0f, 0.0f)) *
			Matrix4::scale(Vector3(4.0f, 4.0f, 4.0f));
		mvpMatrix = gPerspectiveMatrix * gViewMatrix * modelMatrix;
		glUniformMatrix4fv(glGetUniformLocation(GprogramID, "uMvpMatrix"), 1, GL_FALSE, mvpMatrix.data);
		DrawSquare(GtextureID[6]);

		// Draw Back Side
		modelMatrix = Matrix4::translate(Vector3(-4.0f, 0.0f, 0.0f)) *
			Matrix4::rotate(90, Vector3(0.0f, 1.0f, 0.0f)) *
			Matrix4::scale(Vector3(4.0f, 4.0f, 4.0f));
		mvpMatrix = gPerspectiveMatrix * gViewMatrix * modelMatrix;
		glUniformMatrix4fv(glGetUniformLocation(GprogramID, "uMvpMatrix"), 1, GL_FALSE, mvpMatrix.data);
		DrawSquare(GtextureID[7]);

		// Draw Left Side
		modelMatrix = Matrix4::translate(Vector3(0.0f, 0.0f, -4.0f)) *
			Matrix4::rotate(0, Vector3(0.0f, 1.0f, 0.0f)) *
			Matrix4::scale(Vector3(4.0f, 4.0f, 4.0f));
		mvpMatrix = gPerspectiveMatrix * gViewMatrix * modelMatrix;
		glUniformMatrix4fv(glGetUniformLocation(GprogramID, "uMvpMatrix"), 1, GL_FALSE, mvpMatrix.data);
		DrawSquare(GtextureID[8]);

		// Draw Down Side
		modelMatrix = Matrix4::translate(Vector3(0.0f, -4.0f, 0.0f)) *
			Matrix4::rotate(-90, Vector3(1.0f, 0.0f, 0.0f)) *
			Matrix4::scale(Vector3(4.0f, 4.0f, 4.0f));
		mvpMatrix = gPerspectiveMatrix * gViewMatrix * modelMatrix;
		glUniformMatrix4fv(glGetUniformLocation(GprogramID, "uMvpMatrix"), 1, GL_FALSE, mvpMatrix.data);
		DrawSquare(GtextureID[9]);

		// Draw Front Side
		modelMatrix = Matrix4::translate(Vector3(4.0f, 0.0f, 0.0f)) *
			Matrix4::rotate(-90, Vector3(0.0f, 1.0f, 0.0f)) *
			Matrix4::scale(Vector3(4.0f, 4.0f, 4.0f));
		mvpMatrix = gPerspectiveMatrix * gViewMatrix * modelMatrix;
		glUniformMatrix4fv(glGetUniformLocation(GprogramID, "uMvpMatrix"), 1, GL_FALSE, mvpMatrix.data);
		DrawSquare(GtextureID[10]);


		// Draw Right Side
		modelMatrix = Matrix4::translate(Vector3(0.0f, 0.0f, 4.0f)) *
			Matrix4::rotate(180, Vector3(0.0f, 1.0f, 0.0f)) *
			Matrix4::scale(Vector3(4.0f, 4.0f, 4.0f));
		mvpMatrix = gPerspectiveMatrix * gViewMatrix * modelMatrix;
		glUniformMatrix4fv(glGetUniformLocation(GprogramID, "uMvpMatrix"), 1, GL_FALSE, mvpMatrix.data);
		DrawSquare(GtextureID[11]);

		/*
		// Draw first rectangle
		modelMatrix = Matrix4::translate(Vector3(-1.2f, 0.0f, 0.0f)) *
			Matrix4::rotate(0, Vector3(0.0f, 1.0f, 0.0f));
		mvpMatrix = gPerspectiveMatrix * gViewMatrix * modelMatrix;
		glUniformMatrix4fv(glGetUniformLocation(GprogramID, "uMvpMatrix"), 1, GL_FALSE, mvpMatrix.data);
		DrawSquare(GtextureID[5]);

		// Draw second rectangle
		modelMatrix = Matrix4::translate(Vector3(1.2f, 0.0f, 0.0f)) *
			Matrix4::rotate(0, Vector3(0.0f, 1.0f, 0.0f));
		mvpMatrix = gPerspectiveMatrix * gViewMatrix * modelMatrix;
		glUniformMatrix4fv(glGetUniformLocation(GprogramID, "uMvpMatrix"), 1, GL_FALSE, mvpMatrix.data);
		DrawSquare(GtextureID[5]);
		*/
	}
	else
	{
		printf("Framebuffer is not ready!\n");
	}

	//******************************************************************
	// High Pass Filter on the texture
	//******************************************************************
	// Bind the framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, Gframebuffer);

	// Specify texture as color attachment
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, GpTexture_0, 0);

	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status == GL_FRAMEBUFFER_COMPLETE)
	{
		// Clear the buffers (Clear the screen basically)
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Reset the mvpMatrix to identity matrix so that it renders fully on texture in normalized device coordinates
		glUniformMatrix4fv(glGetUniformLocation(GprogramID, "uMvpMatrix"), 1, GL_FALSE, Matrix4::identity().data);

		// Set state to high pass filter
		glUniform1i(glGetUniformLocation(GprogramID, "uState"), 0);

		// Set to no blur
		glUniform1i(glGetUniformLocation(GprogramID, "uBlurDirection"), -1);

		DrawSquare(GfullscreenTexture);
	}
	else
	{
		printf("Framebuffer is not ready!\n");
	}

	//******************************************************************
	// To Blur the Texture Horizontally
	//******************************************************************
	// Bind the framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, Gframebuffer);

	// Specify texture as color attachment
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, GpTexture_1, 0);

	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status == GL_FRAMEBUFFER_COMPLETE)
	{
		// Clear the buffers (Clear the screen basically)
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Reset the mvpMatrix to identity matrix so that it renders fully on texture in normalized device coordinates
		glUniformMatrix4fv(glGetUniformLocation(GprogramID, "uMvpMatrix"), 1, GL_FALSE, Matrix4::identity().data);

		// Set state to gaussian blur
		glUniform1i(glGetUniformLocation(GprogramID, "uState"), 1);

		// Draw the texture that has been screen captured, apply Horizontal blurring
		glUniform1i(glGetUniformLocation(GprogramID, "uBlurDirection"), 0);

		DrawSquare(GpTexture_0);
	}
	else
	{
		printf("Framebuffer is not ready!\n");
	}

	//******************************************************************
	// To Blur Texture Vertically
	//******************************************************************
	// Bind the framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, Gframebuffer);

	// Specify texture as color attachment
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, GpTexture_0, 0);

	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status == GL_FRAMEBUFFER_COMPLETE)
	{
		// Clear the buffers (Clear the screen basically)
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Reset the mvpMatrix to identity matrix so that it renders fully on texture in normalized device coordinates
		glUniformMatrix4fv(glGetUniformLocation(GprogramID, "uMvpMatrix"), 1, GL_FALSE, Matrix4::identity().data);

		// Set state to gaussian blur
		glUniform1i(glGetUniformLocation(GprogramID, "uState"), 1);

		// Draw the texture that has been screen captured, apply Vertical blurring
		glUniform1i(glGetUniformLocation(GprogramID, "uBlurDirection"), 1);

		DrawSquare(GpTexture_1);
	}
	else
	{
		printf("Framebuffer is not ready!\n");
	}
	
	//******************************************************************
	// Plus all the texture together to create Bloom
	// High Pass Filter = Texture A
	// Save Texture A as Texture B
	// Blur Texture B and save as Texture C
	// Render Texture A + Texture C (on top Texture A by adding blend)
	//******************************************************************
	// Render directly to Windows System Framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Reset the mvpMatrix to identity matrix so that it renders fully on texture in normalized device coordinates
	glUniformMatrix4fv(glGetUniformLocation(GprogramID, "uMvpMatrix"), 1, GL_FALSE, Matrix4::identity().data);

	// Set state to normal
	glUniform1i(glGetUniformLocation(GprogramID, "uState"), -1);

	// Set to no blur
	glUniform1i(glGetUniformLocation(GprogramID, "uBlurDirection"), -1);

	// Draw the textures
	glDepthMask(GL_FALSE);

	DrawSquare(GfullscreenTexture);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	DrawSquare(GpTexture_0);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDepthMask(GL_TRUE);



	//Fmod Update
	//updateFmod();
}

int main(void)
{
	glfwSetErrorCallback(error_callback);

	// Initialize GLFW library
	if (!glfwInit())
		return -1;

	glfwDefaultWindowHints();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	// Create and open a window
	window = glfwCreateWindow(WINDOW_WIDTH,
		WINDOW_HEIGHT,
		"Bloom Effect Skybox Animation",
		NULL,
		NULL);

	if (!window)
	{
		glfwTerminate();
		printf("glfwCreateWindow Error\n");
		exit(1);
	}

	glfwMakeContextCurrent(window);

	Init();

	// Repeat
	while (!glfwWindowShouldClose(window)) 
	{
		Draw();
		DrawPepeCube(10);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}
