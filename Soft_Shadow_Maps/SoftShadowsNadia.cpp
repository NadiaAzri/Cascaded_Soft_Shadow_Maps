#include "SoftShadowsNadia.h"
#include "loaderobj.h"
#include <time.h>
#include <gl/glut.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
struct obj_model_t objfile1, objfile2,objfile3;
int obj1,obj2,obj3;


// -----------------------------------------------------------------------------------------------
vec3 cam;
CBuffer::CBuffer()
{
	SetDefaults();
}


CBuffer::~CBuffer()
{
	Empty();
}


void CBuffer::AddData(void *Data, int DataSize)
{
	int Remaining = BufferSize - Position;

	if(DataSize > Remaining)
	{
		BYTE *OldBuffer = Buffer;
		int OldBufferSize = BufferSize;

		int Needed = DataSize - Remaining;

		BufferSize += Needed > BUFFER_SIZE_INCREMENT ? Needed : BUFFER_SIZE_INCREMENT;

		Buffer = new BYTE[BufferSize];

		memcpy(Buffer, OldBuffer, OldBufferSize);

		delete [] OldBuffer;
	}

	memcpy(Buffer + Position, Data, DataSize);

	Position += DataSize;
}


void CBuffer::Empty()
{
	delete [] Buffer;

	SetDefaults();
}


void *CBuffer::GetData()
{
	return Buffer;
}


int CBuffer::GetDataSize()
{
	return Position;
}


void CBuffer::SetDefaults()
{
	Buffer = NULL;

	BufferSize = 0;
	Position = 0;
}


// ----------------------------------------------------------------------------------------------------------------------------

int gl_max_texture_size = 0, gl_max_texture_max_anisotropy_ext = 0;

// ----------------------------------------------------------------------------------------------------------------------------


CTexture::CTexture()
{
	Texture = 0;
}


CTexture::~CTexture()
{
}


CTexture::operator GLuint ()
{
	return Texture;
}


bool CTexture::LoadTexture2D(char *FileName)
{
	CString DirectoryFileName = ModuleDirectory + FileName;

	int Width, Height, BPP;

	FIBITMAP *dib = GetBitmap(DirectoryFileName, Width, Height, BPP);

	if(dib == NULL)
	{
		ErrorLog.Append("Error loading texture " + DirectoryFileName + "!\r\n");
		return false;
	}

	GLenum Format = 0;

	if(BPP == 32) Format = GL_BGRA;
	if(BPP == 24) Format = GL_BGR;

	if(Format == 0)
	{
		ErrorLog.Append("Unsupported texture format (%s)!\r\n", FileName);
		FreeImage_Unload(dib);
		return false;
	}

	Destroy();

	glGenTextures(1, &Texture);

	glBindTexture(GL_TEXTURE_2D, Texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if(GLEW_EXT_texture_filter_anisotropic)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, gl_max_texture_max_anisotropy_ext);
	}

	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Width, Height, 0, Format, GL_UNSIGNED_BYTE, FreeImage_GetBits(dib));

	glBindTexture(GL_TEXTURE_2D, 0);

	FreeImage_Unload(dib);

	return true;
}


void CTexture::Destroy()
{
	glDeleteTextures(1, &Texture);
	Texture = 0;
}


FIBITMAP *CTexture::GetBitmap(char *FileName, int &Width, int &Height, int &BPP)
{
	FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(FileName);

	if(fif == FIF_UNKNOWN)
	{
		fif = FreeImage_GetFIFFromFilename(FileName);
	}

	if(fif == FIF_UNKNOWN)
	{
		return NULL;
	}

	FIBITMAP *dib = NULL;

	if(FreeImage_FIFSupportsReading(fif))
	{
		dib = FreeImage_Load(fif, FileName);
	}

	if(dib != NULL)
	{
		int OriginalWidth = FreeImage_GetWidth(dib);
		int OriginalHeight = FreeImage_GetHeight(dib);

		Width = OriginalWidth;
		Height = OriginalHeight;

		if(Width == 0 || Height == 0)
		{
			FreeImage_Unload(dib);
			return NULL;
		}

		BPP = FreeImage_GetBPP(dib);

		if(Width > gl_max_texture_size) Width = gl_max_texture_size;
		if(Height > gl_max_texture_size) Height = gl_max_texture_size;

		if(!GLEW_ARB_texture_non_power_of_two)
		{
			Width = 1 << (int)floor((log((float)Width) / log(2.0f)) + 0.5f); 
			Height = 1 << (int)floor((log((float)Height) / log(2.0f)) + 0.5f);
		}

		if(Width != OriginalWidth || Height != OriginalHeight)
		{
			FIBITMAP *rdib = FreeImage_Rescale(dib, Width, Height, FILTER_BICUBIC);
			FreeImage_Unload(dib);
			dib = rdib;
		}
	}

	return dib;
}


// -------------------------------------------------------------------------------


CShaderProgram::CShaderProgram()
{
	SetDefaults();
}


CShaderProgram::~CShaderProgram()
{
}


CShaderProgram::operator GLuint ()
{
	return Program;
}


bool CShaderProgram::Load(char *VertexShaderFileName, char *FragmentShaderFileName)
{
	bool Error = false;

	Destroy();

	Error |= ((VertexShader = LoadShader(VertexShaderFileName, GL_VERTEX_SHADER)) == 0);
	Error |= ((FragmentShader = LoadShader(FragmentShaderFileName, GL_FRAGMENT_SHADER)) == 0);

	if(Error)
	{
		Destroy();
		return false;
	}

	Program = glCreateProgram();
	glAttachShader(Program, VertexShader);
	glAttachShader(Program, FragmentShader);
	glLinkProgram(Program);

	int LinkStatus;
	glGetProgramiv(Program, GL_LINK_STATUS, &LinkStatus);

	if(LinkStatus == GL_FALSE)
	{
		ErrorLog.Append("Error linking program (%s, %s)!\r\n", VertexShaderFileName, FragmentShaderFileName);

		int InfoLogLength = 0;
		glGetProgramiv(Program, GL_INFO_LOG_LENGTH, &InfoLogLength);
	
		if(InfoLogLength > 0)
		{
			char *InfoLog = new char[InfoLogLength];
			int CharsWritten  = 0;
			glGetProgramInfoLog(Program, InfoLogLength, &CharsWritten, InfoLog);
			ErrorLog.Append(InfoLog);
			delete [] InfoLog;
		}

		Destroy();

		return false;
	}

	return true;
}


void CShaderProgram::Destroy()
{
	glDetachShader(Program, VertexShader);
	glDetachShader(Program, FragmentShader);

	glDeleteShader(VertexShader);
	glDeleteShader(FragmentShader);

	glDeleteProgram(Program);

	delete [] UniformLocations;
	delete [] AttribLocations;

	SetDefaults();
}


GLuint CShaderProgram::LoadShader(char *FileName, GLenum Type)
{
	CString DirectoryFileName = ModuleDirectory + FileName;

	FILE *File;

	if(fopen_s(&File, DirectoryFileName, "rb") != 0)
	{
		ErrorLog.Append("Error loading file " + DirectoryFileName + "!\r\n");
		return 0;
	}

	fseek(File, 0, SEEK_END);
	long Size = ftell(File);
	fseek(File, 0, SEEK_SET);
	char *Source = new char[Size + 1];
	fread(Source, 1, Size, File);
	fclose(File);
	Source[Size] = 0;

	GLuint Shader = glCreateShader(Type);

	glShaderSource(Shader, 1, (const char**)&Source, NULL);
	delete [] Source;
	glCompileShader(Shader);

	int CompileStatus;
	glGetShaderiv(Shader, GL_COMPILE_STATUS, &CompileStatus);

	if(CompileStatus == GL_FALSE)
	{
		ErrorLog.Append("Error compiling shader %s!\r\n", FileName);

		int InfoLogLength = 0;
		glGetShaderiv(Shader, GL_INFO_LOG_LENGTH, &InfoLogLength);
	
		if(InfoLogLength > 0)
		{
			char *InfoLog = new char[InfoLogLength];
			int CharsWritten  = 0;
			glGetShaderInfoLog(Shader, InfoLogLength, &CharsWritten, InfoLog);
			ErrorLog.Append(InfoLog);
			delete [] InfoLog;
		}

		glDeleteShader(Shader);

		return 0;
	}

	return Shader;
}


void CShaderProgram::SetDefaults()
{
	VertexShader = 0;
	FragmentShader = 0;

	Program = 0;

	UniformLocations = NULL;
	AttribLocations = NULL;
}


// ----------------------------------------------------------------------------------------------------------------------------


CCamera::CCamera()
{
	ViewMatrix = NULL;
	ViewMatrixInverse = NULL;

	X = vec3(1.0f, 0.0f, 0.0f);
	Y = vec3(0.0f, 1.0f, 0.0f);
	Z = vec3(0.0f, 0.0f, 1.0f);

	Position = vec3(0.0f, 0.0f, 5.0f);
	Reference = vec3(0.0f, 0.0f, 0.0f);
}


CCamera::~CCamera()
{
}


void CCamera::Look(const vec3 &Position, const vec3 &Reference, bool RotateAroundReference)
{
	this->Position = Position;
	this->Reference = Reference;

	Z = normalize(Position - Reference);
	X = normalize(cross(vec3(0.0f, 1.0f, 0.0f), Z));
	Y = cross(Z, X);

	if(!RotateAroundReference)
	{
		this->Reference = this->Position;
		this->Position += Z * 0.05f;
	}

	CalculateViewMatrix();
}


void CCamera::Move(const vec3 &Movement)
{
	Position += Movement;
	Reference += Movement;

	CalculateViewMatrix();
}


vec3 CCamera::OnKeys(BYTE Keys, float FrameTime)
{
	float Speed = 5.0f;

	if(Keys & 0x40) Speed *= 2.0f;
	if(Keys & 0x80) Speed *= 0.5f;

	float Distance = Speed * FrameTime;

	vec3 Up(0.0f, 1.0f, 0.0f);
	vec3 Right = X;
	vec3 Forward = cross(Up, Right);

	Up *= Distance;
	Right *= Distance;
	//vec3 Forward;
		Forward*= Distance;

	vec3 Movement;

	if(Keys & 0x01) Movement += Forward;
	if(Keys & 0x02) Movement -= Forward;
	if(Keys & 0x04) Movement -= Right;
	if(Keys & 0x08) Movement += Right;
	if(Keys & 0x10) Movement += Up;
	if(Keys & 0x20) Movement -= Up;

	return Movement;
}


void CCamera::OnMouseMove(int dx, int dy)
{
	float Sensitivity = 0.25f;

	Position -= Reference;

	if(dx != 0)
	{
		float DeltaX = (float)dx * Sensitivity;

		X = rotate(X, DeltaX, vec3(0.0f, 1.0f, 0.0f));
		Y = rotate(Y, DeltaX, vec3(0.0f, 1.0f, 0.0f));
		Z = rotate(Z, DeltaX, vec3(0.0f, 1.0f, 0.0f));
	}

	if(dy != 0)
	{
		float DeltaY = (float)dy * Sensitivity;

		Y = rotate(Y, DeltaY, X);
		Z = rotate(Z, DeltaY, X);

		if(Y.y < 0.0f)
		{
			Z = vec3(0.0f, Z.y > 0.0f ? 1.0f : -1.0f, 0.0f);
			Y = cross(Z, X);
		}
	}

	Position = Reference + Z * length(Position);

	CalculateViewMatrix();
}


void CCamera::OnMouseWheel(float zDelta)
{
	Position -= Reference;

	if(zDelta < 0 && length(Position) < 500.0f)
	{
		Position += Position * 0.1f;
	}

	if(zDelta > 0 && length(Position) > 0.05f)
	{
		Position -= Position * 0.1f;
	}

	Position += Reference;

	CalculateViewMatrix();
}


void CCamera::SetViewMatrixPointer(float *ViewMatrix, float *ViewMatrixInverse)
{
	this->ViewMatrix = (mat4x4*)ViewMatrix;
	this->ViewMatrixInverse = (mat4x4*)ViewMatrixInverse;

	CalculateViewMatrix();cam=Position;
}


void CCamera::CalculateViewMatrix()
{
	if(ViewMatrix != NULL)
	{
		*ViewMatrix = mat4x4(X.x, Y.x, Z.x, 0.0f, X.y, Y.y, Z.y, 0.0f, X.z, Y.z, Z.z, 0.0f, -dot(X, Position), -dot(Y, Position), -dot(Z, Position), 1.0f);

		if(ViewMatrixInverse != NULL)
		{
			*ViewMatrixInverse = inverse(*ViewMatrix);
		}
	}}

// ------------------------------------------------------------------------------------------



CCamera Camera;

// ----------------------------------------------------------------------------------------------------------------------------


COpenGLRenderer::COpenGLRenderer()
{
	Pause = false;

	Radius = 0.7f;

	Camera.SetViewMatrixPointer(&ViewMatrix);
}


COpenGLRenderer::~COpenGLRenderer()
{
}


bool COpenGLRenderer::Init()
{
	// -------------------------------------------------------------------------------------------------------

	bool Error = false;

	// check OpenGL extensions -------------------------------------------------------------------------------

	if(!GLEW_ARB_depth_texture)
	{
		ErrorLog.Append("GL_ARB_depth_texture not supported!\r\n");
		Error = true;
	}

	if(!GLEW_EXT_texture_array)
	{
		ErrorLog.Append("GL_EXT_texture_array not supported!\r\n");
		Error = true;
	}

	if(!GLEW_EXT_framebuffer_object)
	{
		ErrorLog.Append("GL_EXT_framebuffer_object not supported!\r\n");
		Error = true;
	}

	// load textures and shaders ----------------------------------------------------------------------------------------------

	Error |= !Texture.LoadTexture2D("cloitre_Model_4_u2_v1.png");

	Error |= !PenumbraShadowMapping.Load("penumbra_shadow_mapping.vs", "penumbra_shadow_mapping.fs");
	
	// if an error occurred, return false -------------------------------------------------------------------------------------

	if(Error)
	{
		return false;
	}

	// get uniform locations --------------------------------------------------------------------------------------------------

	PenumbraShadowMapping.UniformLocations = new GLuint[5];
	PenumbraShadowMapping.UniformLocations[0] = glGetUniformLocation(PenumbraShadowMapping, "LightPosition");
	PenumbraShadowMapping.UniformLocations[1] = glGetUniformLocation(PenumbraShadowMapping, "CameraPosition");
	PenumbraShadowMapping.UniformLocations[2] = glGetUniformLocation(PenumbraShadowMapping, "LightTextureMatricesCascade3");
	PenumbraShadowMapping.UniformLocations[3] = glGetUniformLocation(PenumbraShadowMapping, "LightTextureMatricesCascade2");
	PenumbraShadowMapping.UniformLocations[4] = glGetUniformLocation(PenumbraShadowMapping, "LightTextureMatricesCascade1");
	// set constant uniforms --------------------------------------------------------------------------------------------------

	glUseProgram(PenumbraShadowMapping);
	glUniform1i(glGetUniformLocation(PenumbraShadowMapping, "Texture"), 0);
	glUniform1i(glGetUniformLocation(PenumbraShadowMapping, "ShadowMapCascade3"), 1);
	glUniform1i(glGetUniformLocation(PenumbraShadowMapping, "ShadowMapCascade2"), 2);
	glUniform1i(glGetUniformLocation(PenumbraShadowMapping, "ShadowMapCascade1"), 3);

	glUniform1i(glGetUniformLocation(PenumbraShadowMapping, "siezSM"), 4);
	glUseProgram(0);

	// init array buffers -----------------------------------------------------------------------------------------------------

	float Data[] = 
	{	// s, t, nx, ny, nz, x, y, z
		0.0f, 0.0f, 0.0f, 1.0f, 0.0f, -10.0f, -0.5f, 10.0f,
		20.0f, 0.0f, 0.0f, 1.0f, 0.0f, 10.0f, -0.5f, 10.0f,
		20.0f, 20.0f, 0.0f, 1.0f, 0.0f, 10.0f, -0.5f, -10.0f,
		0.0f, 20.0f, 0.0f, 1.0f, 0.0f, -10.0f, -0.5f, -10.0f

		
	};

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, 128, Data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// generate shadow map texture --------------------------------------------------------------------------------------------

	glGenTextures(1, &ShadowMapCascade1);
	glBindTexture(GL_TEXTURE_2D_ARRAY, ShadowMapCascade1);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT24, 1024, 1024, 36, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	
	// generate FBO -----------------------------------------------------------------------------------------------------------

	glGenFramebuffersEXT(1, &FBO);

	// set light position and projection matrix -------------------------------------------------------------------------------

	LightPosition = vec3(7.0f, 2.0f, 4.0f);

	// set camera -------------------------------------------------------------------------------------------------------------

	Camera.Look(vec3(0.0f, 3.0f, 14.0f), vec3(0.0f, 0.0f, 0.0f));
	
	return true;
}


float SHADOW_MAP_SIZE;
float min1=20.0f,min2=30.0f,max1=0.0,max2=0.0;


void COpenGLRenderer::RenderShadowMap()
{
		LightProjectionMatrix =ortho(-8,8,-8,8,-10.0,50);
	
	float pas=0.1;
	int k=0;

	vec3 lp2 = vec3(0.0f);
	lp2.x = LightPosition.x;
	lp2.y = LightPosition.y;
	lp2.z = LightPosition.z;
	k=0;
	for(int i = 0; i < 6; i++)
	{for(int j = 0; j < 6; j++)
	{
		LightViewMatricesCascade1[k] = look(lp2, vec3(0.0f), vec3(0.0f, 1.0f, 0.0f));
		LightTextureMatricesCascade1[k] = BiasMatrix * LightProjectionMatrix * LightViewMatricesCascade1[k];
	lp2.x =lp2.x+pas;
	k++;

	}
	lp2.y =lp2.y-pas;
	lp2.z =lp2.z-pas;
	lp2.x = LightPosition.x;

	}
	// render shadow map ------------------------------------------------------------------------------------------------------

	glViewport(0, 0, 1024, 1024);

	for(int i = 0; i < 36; i++)
	{
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, FBO);
		glDrawBuffers(0, NULL); glReadBuffer(GL_NONE);
		glFramebufferTextureLayerEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, ShadowMapCascade2, 0, i);
		glClear(GL_DEPTH_BUFFER_BIT);
		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(&LightProjectionMatrix);
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(&LightViewMatricesCascade2[i]);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		//glTranslatef(-5,0,0);
		glPushMatrix();
		glTranslatef(0,-0.24,0);
	        glScalef(0.5,0.5,0.5);
	        // RenderOBJModel (&objfile2);
		glutSolidCube(1);	
	        glPopMatrix();
	        glPushMatrix();
	        glTranslatef(0,-0.24,-5);
		glScalef(0.5,0.5,0.5);
	       //RenderOBJModel (&objfile2);
		glutSolidCube(1);
		glPopMatrix();
		glPushMatrix();
		glTranslatef(0,-0.24,6);
		glScalef(0.5,0.5,0.5);
	        //RenderOBJModel (&objfile2);
		glutSolidCube(1);
		glPopMatrix();
		glCullFace(GL_BACK);
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	}


}

int a=0,b=0;	double TimeTaken;
void COpenGLRenderer::Render(float FrameTime)
{
	// render shadow map ------------------------------------------------------------------------------
	clock_t t;
	if(a==0)
	t=clock();// render shadow map ---------------------------------------------------------------------

	// render shadow map -------------------------------------------------------------------------------

	if(!Pause)
	{
		RenderShadowMap();
	}

	// render scene -------------------------------------------------------------------------------------

	glViewport(0, 0, Width, Height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(&ProjectionMatrix);
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(&ViewMatrix);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, Texture);
	glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D_ARRAY, ShadowMapCascade1);
	glUseProgram(PenumbraShadowMapping);
	glUniform3fv(PenumbraShadowMapping.UniformLocations[0], 1, &LightPosition);
	glUniform3fv(PenumbraShadowMapping.UniformLocations[1], 1, &Camera.Position);
	glUniformMatrix4fv(PenumbraShadowMapping.UniformLocations[4], 36, GL_FALSE, (GLfloat*)LightTextureMatricesCascade1);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 32, (void*)0);
	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 32, (void*)8);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 32, (void*)20);
	glDrawArrays(GL_QUADS, 0, 52);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glTranslatef(-5,0,0);
	glPushMatrix();
	glTranslatef(-0.009,-0.25,0.02);
	glScalef(0.5,0.5,0.5);
	//RenderOBJModel (&objfile2);
	glutSolidCube(1);
	glPopMatrix();
	glPushMatrix();
	glTranslatef(-0.009,-0.25,-4.98);
	glScalef(0.5,0.5,0.5);
	//RenderOBJModel (&objfile2);
	glutSolidCube(1);
	glPopMatrix();
	glPushMatrix();
	glTranslatef(-0,-0.25,6.02);
	glScalef(0.5,0.5,0.5);
	//RenderOBJModel (&objfile2);
	glutSolidCube(1);
	glPopMatrix(); 
	glUseProgram(0);
	glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	
	
	
	// ------------------------MOVE LIGHT----------------------------------------------------

	/*if(!Pause)
	{
		LightPosition = rotate(LightPosition, 2.8125f * FrameTime, vec3(0.0f, 1.0f, 0.0f));
	}*/

	// -------------------------------TIME TAKEN--------------------------------------------------

	if(a==0){
	t=clock()-t;
	 TimeTaken=((double)t)/CLOCKS_PER_SEC;
	}
	a=1;
	Text.Set("Time Taken : %f",TimeTaken);
}


void SaveImage(){

	// Make the BYTE array, factor of 3 because it's RBG.
	BYTE* pixels = new BYTE[3 * 800 * 600];

	glReadPixels(0, 0, 800, 600, GL_RGB, GL_UNSIGNED_BYTE, pixels);

	// Convert to FreeImage format & save to file
	FIBITMAP* image = FreeImage_ConvertFromRawBits(pixels, 800, 600, 3 * 800, 24, 0x0000FF, 0xFF0000, 0x00FF00, false);
	FreeImage_Save(FIF_JPEG, image, "testNaTexture2color.jpg", 0);

	// Free resources
	FreeImage_Unload(image);
	delete [] pixels;

}


void COpenGLRenderer::Resize(int Width, int Height)
{
	this->Width = Width;
	this->Height = Height;

	ProjectionMatrix = perspective(45.0f, (float)Width / (float)Height, 0.125f, 512.0f);
}


void COpenGLRenderer::Destroy()
{
	Texture.Destroy();

	PenumbraShadowMapping.Destroy();

	glDeleteBuffers(1, &VBO);

	glDeleteTextures(1, &ShadowMapCascade1);
	
	if(GLEW_EXT_framebuffer_object)
	{
		glDeleteFramebuffersEXT(1, &FBO);
	}
}


// ----------------------------------------------------------------------------------------------------------------------------

COpenGLRenderer OpenGLRenderer;

// ----------------------------------------------------------------------------------------------------------------------------


CString ModuleDirectory, ErrorLog;

// ----------------------------------------------------------------------------------------------------------------------------


void GetModuleDirectory()
{
	char *moduledirectory = new char[256];
	GetModuleFileName(GetModuleHandle(NULL), moduledirectory, 256);
	*(strrchr(moduledirectory, '\\') + 1) = 0;
	ModuleDirectory = moduledirectory;
	delete [] moduledirectory;
}


// ----------------------------------------------------------------------------------------------------------------------------

COpenGLView::COpenGLView()
{
}


COpenGLView::~COpenGLView()
{
}


bool COpenGLView::Init(HINSTANCE hInstance, char *Title, int Width, int Height, int Samples)
{
	this->Title = Title;
	this->Width = Width;
	this->Height = Height;

	WNDCLASSEX WndClassEx;

	memset(&WndClassEx, 0, sizeof(WNDCLASSEX));

	WndClassEx.cbSize = sizeof(WNDCLASSEX);
	WndClassEx.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	WndClassEx.lpfnWndProc = WndProc;
	WndClassEx.hInstance = hInstance;
	WndClassEx.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClassEx.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	WndClassEx.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClassEx.lpszClassName = "Win32OpenGLWindowClass";

	if(RegisterClassEx(&WndClassEx) == 0)
	{
		ErrorLog.Set("RegisterClassEx failed!");
		return false;
	}

	DWORD Style = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	hWnd = CreateWindowEx(WS_EX_APPWINDOW, WndClassEx.lpszClassName, Title, Style, 0, 0, Width, Height, NULL, NULL, hInstance, NULL);

	if(hWnd == NULL)
	{
		ErrorLog.Set("CreateWindowEx failed!");
		return false;
	}

	HDC hDC = GetDC(hWnd);

	if(hDC == NULL)
	{
		ErrorLog.Set("GetDC failed!");
		return false;
	}

	PIXELFORMATDESCRIPTOR pfd;

	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));

	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 24;
	pfd.iLayerType = PFD_MAIN_PLANE;

	int PixelFormat = ChoosePixelFormat(hDC, &pfd);

	if(PixelFormat == 0)
	{
		ErrorLog.Set("ChoosePixelFormat failed!");
		return false;
	}

	static int MSAAPixelFormat = 0;

	if(SetPixelFormat(hDC, MSAAPixelFormat == 0 ? PixelFormat : MSAAPixelFormat, &pfd) == FALSE)
	{
		ErrorLog.Set("SetPixelFormat failed!");
		return false;
	}

	hGLRC = wglCreateContext(hDC);

	if(hGLRC == NULL)
	{
		ErrorLog.Set("wglCreateContext failed!");
		return false;
	}

	if(wglMakeCurrent(hDC, hGLRC) == FALSE)
	{
		ErrorLog.Set("wglMakeCurrent failed!");
		return false;
	}

	if(glewInit() != GLEW_OK)
	{
		ErrorLog.Set("glewInit failed!");
		return false;
	}

	if(!GLEW_VERSION_2_1)
	{
		ErrorLog.Set("OpenGL 2.1 not supported!");
		return false;
	}

	if(MSAAPixelFormat == 0 && Samples > 0)
	{
		if(GLEW_ARB_multisample && WGLEW_ARB_pixel_format)
		{
			while(Samples > 0)
			{
				UINT NumFormats = 0;

				int PFAttribs[] =
				{
					WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
					WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
					WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
					WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
					WGL_COLOR_BITS_ARB, 32,
					WGL_DEPTH_BITS_ARB, 24,
					WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
					WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
					WGL_SAMPLES_ARB, Samples,
					0
				};

				if(wglChoosePixelFormatARB(hDC, PFAttribs, NULL, 1, &MSAAPixelFormat, &NumFormats) == TRUE && NumFormats > 0) break;

				Samples--;
			}

			wglDeleteContext(hGLRC);
			DestroyWindow(hWnd);
			UnregisterClass(WndClassEx.lpszClassName, hInstance);

			return Init(hInstance, Title, Width, Height, Samples);
		}
		else
		{
			Samples = 0;
		}
	}

	this->Samples = Samples;

	GetModuleDirectory();

	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &gl_max_texture_size);

	if(GLEW_EXT_texture_filter_anisotropic)
	{
		glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &gl_max_texture_max_anisotropy_ext);
	}

	if(WGLEW_EXT_swap_control)
	{
		wglSwapIntervalEXT(0);
	}

	return OpenGLRenderer.Init();
}


void COpenGLView::Show(bool Maximized)
{
	RECT dRect, wRect, cRect;

	GetWindowRect(GetDesktopWindow(), &dRect);
	GetWindowRect(hWnd, &wRect);
	GetClientRect(hWnd, &cRect);

	wRect.right += Width - cRect.right;
	wRect.bottom += Height - cRect.bottom;
	wRect.right -= wRect.left;
	wRect.bottom -= wRect.top;
	wRect.left = dRect.right / 2 - wRect.right / 2;
	wRect.top = dRect.bottom / 2 - wRect.bottom / 2;

	MoveWindow(hWnd, wRect.left, wRect.top, wRect.right, wRect.bottom, FALSE);

	ShowWindow(hWnd, Maximized ? SW_SHOWMAXIMIZED : SW_SHOWNORMAL);
}


void COpenGLView::MessageLoop()
{
	MSG Msg;

	while(GetMessage(&Msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
}


void COpenGLView::Destroy()
{
	if(GLEW_VERSION_2_1)
	{
		OpenGLRenderer.Destroy();
	}

	wglDeleteContext(hGLRC);
	DestroyWindow(hWnd);
}


void COpenGLView::OnKeyDown(UINT Key)
{
	switch(Key)
	{
		case VK_SPACE:
			OpenGLRenderer.Pause = !OpenGLRenderer.Pause;
			break;

		case VK_ADD:
			OpenGLRenderer.Radius += 0.001f;
			if(OpenGLRenderer.Pause) OpenGLRenderer.RenderShadowMap();
			break;

		case VK_SUBTRACT:
			OpenGLRenderer.Radius -= 0.001f;
			if(OpenGLRenderer.Radius < 0.0f) OpenGLRenderer.Radius = 0.0f;
			if(OpenGLRenderer.Pause) OpenGLRenderer.RenderShadowMap();
			break;
	}
}


void COpenGLView::OnMouseMove(int X, int Y)
{
	if(GetKeyState(VK_RBUTTON) & 0x80)
	{
		Camera.OnMouseMove(LastX - X, LastY - Y);

		LastX = X;
		LastY = Y;
	}
}


void COpenGLView::OnMouseWheel(short zDelta)
{
	Camera.OnMouseWheel(zDelta);
}

void COpenGLView::OnPaint()
{
	static DWORD LastFPSTime = GetTickCount(), LastFrameTime = LastFPSTime, FPS = 0;

	PAINTSTRUCT ps;

	HDC hDC = BeginPaint(hWnd, &ps);

	DWORD Time = GetTickCount();

	float FrameTime = (Time - LastFrameTime) * 0.001f;

	LastFrameTime = Time;

	if(Time - LastFPSTime > 1000)
	{
		CString Text = Title;

		if(OpenGLRenderer.Text[0] != 0)
		{
			Text.Append(" - " + OpenGLRenderer.Text);
		}

		Text.Append(" - %dx%d", Width, Height);
		Text.Append(", ATF %dx", gl_max_texture_max_anisotropy_ext);
		Text.Append(", MSAA %dx", Samples);
		Text.Append(", FPS: %d", FPS);
		Text.Append(" - %s", glGetString(GL_RENDERER));
		
		SetWindowText(hWnd, Text);

		LastFPSTime = Time;
		FPS = 0;
	}
	else
	{
		FPS++;
	}

	BYTE Keys = 0x00;

	
	if(GetKeyState('W') & 0x80){Keys |= 0x01;}
	if(GetKeyState('S') & 0x80){Keys |= 0x02;}
	if(GetKeyState('A') & 0x80) Keys |= 0x04;
	if(GetKeyState('D') & 0x80) Keys |= 0x08;
	if(GetKeyState('R') & 0x80) Keys |= 0x10;
	if(GetKeyState('F') & 0x80) Keys |= 0x20;
	if(GetKeyState('X') & 0x80) SaveImage();

	if(GetKeyState(VK_SHIFT) & 0x80) Keys |= 0x40;
	if(GetKeyState(VK_CONTROL) & 0x80) Keys |= 0x80;

	if(Keys & 0x3F)
	{
		Camera.Move(Camera.OnKeys(Keys, FrameTime));
	}

	OpenGLRenderer.Render(FrameTime);

	SwapBuffers(hDC);

	EndPaint(hWnd, &ps);

	InvalidateRect(hWnd, NULL, FALSE);
}


void COpenGLView::OnRButtonDown(int X, int Y)
{
	LastX = X;
	LastY = Y;
}


void COpenGLView::OnSize(int Width, int Height)
{
	this->Width = Width;
	this->Height = Height;

	OpenGLRenderer.Resize(Width, Height);
}


// ----------------------------------------------------------------------------------------------------------------------------

COpenGLView OpenGLView;

// ----------------------------------------------------------------------------------------------------------------------------


LRESULT CALLBACK WndProc(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uiMsg)
	{
		case WM_CLOSE:
			PostQuitMessage(0);
			break;

		case WM_MOUSEMOVE:
			OpenGLView.OnMouseMove(LOWORD(lParam), HIWORD(lParam));
			break;

		case 0x020A: // WM_MOUSWHEEL
			OpenGLView.OnMouseWheel(HIWORD(wParam));
			break;

		case WM_KEYDOWN:
			OpenGLView.OnKeyDown((UINT)wParam);
			break;

		case WM_PAINT:
			OpenGLView.OnPaint();
			break;

		case WM_RBUTTONDOWN:
			OpenGLView.OnRButtonDown(LOWORD(lParam), HIWORD(lParam));
			break;

		case WM_SIZE:
			OpenGLView.OnSize(LOWORD(lParam), HIWORD(lParam));
			break;

		default:
			return DefWindowProc(hWnd, uiMsg, wParam, lParam);
	}

	return 0;
}

// ----------------------------------------------------------------------------------------------------------------------------


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR sCmdLine, int iShow)
{ 
	//obj1=ReadOBJModel ("model\\LD_HorseRtime02.obj",&objfile1);
	//obj2=ReadOBJModel ("ModelNadia\\please.obj",&objfile2);
	//obj3=ReadOBJModel ("model\\statue.obj",&objfile3);
	char *AppName = "Cascaded Soft Shadow Maps ";

	if(OpenGLView.Init(hInstance, AppName, 800, 600, 4))
	{
		OpenGLView.Show();
		OpenGLView.MessageLoop();
	}
	else
	{
		MessageBox(NULL, ErrorLog, AppName, MB_OK | MB_ICONERROR);
	}

	OpenGLView.Destroy();

	return 0;
}
