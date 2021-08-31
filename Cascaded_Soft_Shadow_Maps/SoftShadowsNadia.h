// ----------------------------------------------------------------------------------------------------------------------------
//
// Version 2.02
//
// ----------------------------------------------------------------------------------------------------------------------------

#include <windows.h>

 #include "glmath.h"
#include "string.h"

#include <gl/glew.h> // http://glew.sourceforge.net/
#include <gl/wglew.h>
#include <gl/glu.h>
#include <gl/glut.h>
#include <FreeImage.h> // http://freeimage.sourceforge.net/

// ----------------------------------------------------------------------------------------------------------------------------

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "FreeImage.lib")

// ----------------------------------------------------------------------------------------------------------------------------

extern CString ModuleDirectory, ErrorLog;

// ----------------------------------------------------------------------------------------------------------------------------

#define BUFFER_SIZE_INCREMENT 1048576

// ----------------------------------------------------------------------------------------------------------------------------
float min1=20.0f,min2=30.0f,max1=0.0,max2=0.0;
class CBuffer
{
private:
	BYTE *Buffer;
	int BufferSize, Position;

public:
	CBuffer();
	~CBuffer();

	void AddData(void *Data, int DataSize);
	void Empty();
	void *GetData();
	int GetDataSize();

private:
	void SetDefaults();
};

// ----------------------------------------------------------------------------------------------------------------------------

extern int gl_max_texture_size, gl_max_texture_max_anisotropy_ext;
float Zz=13.0f;
// ----------------------------------------------------------------------------------------------------------------------------

class CTexture
{
protected:
	GLuint Texture;

public:
	CTexture();
	~CTexture();

	operator GLuint ();

	bool LoadTexture2D(char *FileName);
	bool LoadTextureCubeMap(char **FileNames);
	void Destroy();

protected:
	FIBITMAP *CTexture::GetBitmap(char *FileName, int &Width, int &Height, int &BPP);
};

// ----------------------------------------------------------------------------------------------------------------------------

class CShaderProgram
{
protected:
	GLuint VertexShader, FragmentShader, Program;

public:
	GLuint *UniformLocations, *AttribLocations;

public:
	CShaderProgram();
	~CShaderProgram();

	operator GLuint ();

	bool Load(char *VertexShaderFileName, char *FragmentShaderFileName);
	void Destroy();

protected:
	GLuint LoadShader(char *FileName, GLenum Type);
	void SetDefaults();
};

// ----------------------------------------------------------------------------------------------------------------------------

class CCamera
{
protected:
	mat4x4 *ViewMatrix, *ViewMatrixInverse;

public:
	vec3 X, Y, Z, Position, Reference;

	CCamera();
	~CCamera();

	void Look(const vec3 &Position, const vec3 &Reference, bool RotateAroundReference = false);
	void Move(const vec3 &Movement);
	vec3 OnKeys(BYTE Keys, float FrameTime);
	void OnMouseMove(int dx, int dy);
	void OnMouseWheel(float zDelta);
	void SetViewMatrixPointer(float *ViewMatrix, float *ViewMatrixInverse = NULL);

private:
	void CalculateViewMatrix();
};

// ----------------------------------------------------------------------------------------------------------------------------



class COpenGLRenderer
{
protected:
	int Width, Height;
	mat4x4 ModelMatrix, ViewMatrix, ProjectionMatrix;

protected:
	CTexture Texture;
	CShaderProgram PenumbraShadowMapping;
	GLuint VBO, ShadowMapCascade3,ShadowMapCascade2,ShadowMapCascade1, FBO;
	vec3 LightPosition;
	mat4x4 LightViewMatricesCascade3[4],LightViewMatricesCascade2[9],LightViewMatricesCascade1[16], LightProjectionMatrix, LightTextureMatricesCascade3[4], LightTextureMatricesCascade2[9], LightTextureMatricesCascade1[16];

public:
	bool Pause;
	float Radius;

public:
	CString Text;

public:
	COpenGLRenderer();
	~COpenGLRenderer();

	bool Init();
	void RenderShadowMap();
	void Render(float FrameTime);
	void Resize(int Width, int Height);
	void Destroy();
};

// ----------------------------------------------------------------------------------------------------------------------------

class COpenGLView
{
protected:
	char *Title;
	int Width, Height, Samples;
	HWND hWnd;
	HGLRC hGLRC;

protected:
	int LastX, LastY;

public:
	COpenGLView();
	~COpenGLView();

	bool Init(HINSTANCE hInstance, char *Title, int Width, int Height, int Samples);
	void Show(bool Maximized = false);
	void MessageLoop();
	void Destroy();

	void OnKeyDown(UINT Key);
	void OnMouseMove(int X, int Y);
	void OnMouseWheel(short zDelta);
	void OnPaint();
	void OnRButtonDown(int X, int Y);
	void OnSize(int Width, int Height);
};

// ----------------------------------------------------------------------------------------------------------------------------

LRESULT CALLBACK WndProc(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam);

// ----------------------------------------------------------------------------------------------------------------------------

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR sCmdLine, int iShow);
