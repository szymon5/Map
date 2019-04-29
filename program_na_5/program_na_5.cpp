#define BITMAP_ID 0x4D42
#define MAP_SCALE 20.0f
#define PI 3.141592
#include"Structures.cpp"
#include<GL/freeglut.h>
#include<GL/GL.h>
#include<GL/GLU.h>
#include<math.h>
#include<time.h>
#include<stdio.h>
#include<stdlib.h>
#include<omp.h>

Math math;
Water water;
Mouse mouse;
Camera camera;
Texture texture;
Data data;
ID id;

HDC globalContext;

int x, z;
bool keyPressed[256];
bool fullscreen = false;

float Terrain[Island::X][Island::Z][3];

unsigned char *LoadBMPFile(const char *filename, BITMAPINFOHEADER *info)
{
	FILE *file;
	BITMAPFILEHEADER fileHeader;
	unsigned char *imgData;
	unsigned char tempRGB;
	errno_t error;

	error = fopen_s(&file, filename, "rb");
	if (file == NULL)
		return NULL;

	fread(&fileHeader, sizeof(BITMAPFILEHEADER), 1, file);

	if (fileHeader.bfType != BITMAP_ID)
	{
		fclose(file);
		return NULL;
	}

	fread(info, sizeof(BITMAPINFOHEADER), 1, file);

	fseek(file, fileHeader.bfOffBits, SEEK_SET);

	imgData = (unsigned char*)malloc(info->biSizeImage);

	if (!imgData)
	{
		free(imgData);
		fclose(file);
		return NULL;
	}

	fread(imgData, 1, info->biSizeImage, file);

	if (imgData == NULL)
	{
		fclose(file);
		return NULL;
	}

#pragma omp parallel for private(x,tempRGB)
	for (x = 0; x < info->biSizeImage; x += 3)
	{
		tempRGB = imgData[x];
		imgData[x] = imgData[x + 2];
		imgData[x + 2] = tempRGB;
	}

	fclose(file);
	return imgData;
}

void DrawIslandPoints()
{
	for (z = 0; z < Island::Z ; z++)
	{
#pragma omp parallel for private(x)
		for (x = 0; x < Island::X; x++)
		{
			Terrain[x][z][0] = float(x)*MAP_SCALE;
			Terrain[x][z][1] = (float)data.imageData[(z*Island::Z+x)*3];
			Terrain[x][z][2] = -float(z)*MAP_SCALE;
		}
	}
}

void LoadTexture()
{
#pragma omp parallel num_threads(2)
	{
#pragma omp sections nowait
		{
#pragma omp section
			{
				data.island_data = LoadBMPFile("green.bmp", &texture.island_header);

				glGenTextures(1, &id.island_ID);
				glBindTexture(GL_TEXTURE_2D, id.island_ID);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, texture.island_header.biWidth, texture.island_header.biHeight, GL_RGB, GL_UNSIGNED_BYTE, data.island_data);
			}
#pragma omp section
			{
				data.water_data = LoadBMPFile("water.bmp", &texture.water_header);

				glGenTextures(1, &id.water_ID);
				glBindTexture(GL_TEXTURE_2D, id.water_ID);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, texture.water_header.biWidth, texture.water_header.biHeight, GL_RGB, GL_UNSIGNED_BYTE, data.water_data);
			}
		}
	}
}

void CleanUp()
{
#pragma omp parallel num_threads(3)
	{
#pragma omp sections nowait
		{
#pragma omp section
			{
				free(data.imageData);
			}
#pragma omp section
			{
				free(data.island_data);
			}
#pragma omp section
			{
				free(data.water_data);
			}
		}
	}
}

void InitializeOpenGLGraphic()
{
#pragma omp parallel num_threads(2)
	{
#pragma omp sections nowait
		{
#pragma omp section
			{
				glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

				glShadeModel(GL_SMOOTH);
				glEnable(GL_DEPTH_TEST);
				glEnable(GL_CULL_FACE);
				glFrontFace(GL_CCW);
			}
#pragma omp section
			{
				glEnable(GL_TEXTURE_2D);

				data.imageData = LoadBMPFile("Terrain2.bmp", &texture.header);

				DrawIslandPoints();
				LoadTexture();
			}
		}
	}
}

void SetCameraPosition()
{
#pragma omp parallel num_threads(2)
	{
#pragma omp sections nowait
		{
#pragma omp section
			{
				camera.cameraX = camera.lookX + sin(math.radians)*mouse.mouseY;
				camera.cameraZ = camera.lookZ + cos(math.radians)*mouse.mouseY;
				camera.cameraY = camera.lookY + mouse.mouseY;
			}
#pragma omp section
			{
				camera.lookX = (Island::X*MAP_SCALE) / 2.0f;
				camera.lookY = 100.0f;
				camera.lookZ = -(Island::Z*MAP_SCALE) / 2.0f;
			}
		}
	}
	
}

void ClearBuffers()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	gluLookAt(camera.cameraX, camera.cameraY, camera.cameraZ, camera.lookX, camera.lookY, camera.lookZ, 0.0, 1.0, 0.0);
}

void DrawIsland()
{
	glBindTexture(GL_TEXTURE_2D, id.island_ID);

	for (z = 0; z < Island::Z - 1; z++)
	{
		glBegin(GL_TRIANGLE_STRIP);
		for (x = 0; x < Island::X - 1; x++)
		{
			glColor3f(Terrain[x][z][1] / 255.0f, Terrain[x][z][1] / 255.0f, Terrain[x][z][1] / 255.0f);
			glTexCoord2f(0.0f, 0.0f);
			glVertex3f(Terrain[x][z][0], Terrain[x][z][1], Terrain[x][z][2]);

			glTexCoord2f(1.0f, 0.0f);
			glColor3f(Terrain[x + 1][z][1] / 255.0f, Terrain[x + 1][z][1] / 255.0f, Terrain[x + 1][z][1] / 255.0f);
			glVertex3f(Terrain[x + 1][z][0], Terrain[x + 1][z][1], Terrain[x + 1][z][2]);

			glTexCoord2f(0.0f, 1.0f);
			glColor3f(Terrain[x][z + 1][1] / 255.0f, Terrain[x][z + 1][1] / 255.0f, Terrain[x][z + 1][1] / 255.0f);
			glVertex3f(Terrain[x][z + 1][0], Terrain[x][z + 1][1], Terrain[x][z + 1][2]);

			glColor3f(Terrain[x + 1][z + 1][1] / 255.0f, Terrain[x + 1][z + 1][1] / 255.0f, Terrain[x + 1][z + 1][1] / 255.0f);
			glTexCoord2f(1.0f, 1.0f);
			glVertex3f(Terrain[x + 1][z + 1][0], Terrain[x + 1][z + 1][1], Terrain[x + 1][z + 1][2]);
		}
		glEnd();
	}
}

void SetBuffers()
{
#pragma omp parallel num_threads(3)
	{
#pragma omp sections nowait
		{
#pragma omp section
			{
				glEnable(GL_BLEND);
			}
#pragma omp section
			{
				glDepthMask(GL_FALSE);
			}
#pragma omp section
			{
				glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			}
		}
	}
}

void DrawWater()
{
	glColor4f(0.5f, 0.5f, 1.0f, 0.7f);
	glBindTexture(GL_TEXTURE_2D, id.water_ID);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(Terrain[0][0][0], water.waterHeight, Terrain[0][0][2]);

	glTexCoord2f(10.0f, 0.0f);
	glVertex3f(Terrain[Island::X - 1][0][0], water.waterHeight, Terrain[Island::X - 1][0][2]);

	glTexCoord2f(10.0f, 10.0f);
	glVertex3f(Terrain[Island::X - 1][Island::Z - 1][0], water.waterHeight, Terrain[Island::X - 1][Island::Z - 1][2]);

	glTexCoord2f(0.0f, 10.0f);
	glVertex3f(Terrain[0][Island::Z - 1][0], water.waterHeight, Terrain[0][Island::Z - 1][2]);
	glEnd();
}

void RestoreBuffers()
{
#pragma omp parallel num_threads(2)
	{
#pragma omp sections nowait
		{
#pragma omp section
			{
				glDepthMask(GL_TRUE);
			}
#pragma omp section
			{
				glDisable(GL_BLEND);
			}
		}
	}
}

void WaterAnimation(bool active)
{
	if (active)
	{
		if (water.waterHeight > 155.0f)water.waterDirection = false;
		else if (water.waterHeight < 154.0f)water.waterDirection = true;

		if (water.waterDirection)water.waterHeight += 0.01f;
		else water.waterHeight -= 0.01f;
	}

}

void Render()
{
	math.radians = float(PI*(math.angle / 180.0f));
	SetCameraPosition();
	ClearBuffers();
	DrawIsland();
	SetBuffers();
	DrawWater();
	RestoreBuffers();
	WaterAnimation(false);
	glFlush();
	SwapBuffers(globalContext);
}

void PixelFormat(HDC hDC)
{
	int _pixelFormat;

	static PIXELFORMATDESCRIPTOR format = {
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW |
		PFD_SUPPORT_OPENGL |
		PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		32,
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		16,
		0,
		0,
		PFD_MAIN_PLANE,
		0,
		0, 0, 0 
	};

	_pixelFormat = ChoosePixelFormat(hDC, &format);

	SetPixelFormat(hDC, _pixelFormat, &format);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HGLRC graphicsContext;
	static HDC hDC;
	int width, height;
	int oldMouseX, oldMouseY;

	switch (message)
	{
	case WM_CREATE:

		hDC = GetDC(hwnd);
		globalContext = hDC;
		PixelFormat(hDC);

		graphicsContext = wglCreateContext(hDC);
		wglMakeCurrent(hDC, graphicsContext);

		return 0;
		break;

	case WM_CLOSE:

		wglMakeCurrent(hDC, NULL);
		wglDeleteContext(graphicsContext);

		PostQuitMessage(0);

		return 0;
		break;

	case WM_SIZE:
		height = HIWORD(lParam);
		width = LOWORD(lParam);

		if (height == 0)
		{
			height = 1;
		}

		glViewport(0, 0, width, height);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		gluPerspective(54.0f, (GLfloat)width / (GLfloat)height, 1.0f, 1000.0f);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		return 0;
		break;

	case WM_KEYDOWN:
		keyPressed[wParam] = true;
		return 0;
		break;

	case WM_KEYUP:
		keyPressed[wParam] = false;
		return 0;
		break;

	case WM_MOUSEMOVE:
		mouse.oldMouseX = mouse.mouseX;
		mouse.oldMouseY = mouse.mouseY;

		mouse.mouseX = LOWORD(lParam);
		mouse.mouseY = HIWORD(lParam);

		if (mouse.mouseY < 200)mouse.mouseY = 200;
		if (mouse.mouseY > 450)mouse.mouseY = 450;

		if ((mouse.mouseX - mouse.oldMouseX) > 0)math.angle += 3.0f;//mysz w prawo
		else if ((mouse.mouseX - mouse.oldMouseX) < 0)math.angle -= 3.0f;//mysz w lewo

		return 0;
		break;
	default:
		break;
	}

	return (DefWindowProc(hwnd, message, wParam, lParam));
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int showCmd)
{
	WNDCLASSEX window;
	HWND	   hwnd;
	MSG		   msg;
	bool	   isDone;
	DWORD	   exStyle;
	DWORD	   Style;
	RECT	   windowRect;

	int width = 800;
	int height = 600;
	int bits = 32;

	//fullScreen = TRUE;

	windowRect.left = (long)0;
	windowRect.right = (long)width;
	windowRect.top = (long)0;
	windowRect.bottom = (long)height;

	window.cbSize = sizeof(WNDCLASSEX);
	window.style = CS_HREDRAW | CS_VREDRAW;
	window.lpfnWndProc = WndProc;
	window.cbClsExtra = 0;
	window.cbWndExtra = 0;
	window.hInstance = instance;
	window.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	window.hCursor = LoadCursor(NULL, IDC_ARROW);
	window.hbrBackground = NULL;
	window.lpszMenuName = NULL;
	window.lpszClassName = "program_na_5";
	window.hIconSm = LoadIcon(NULL, IDI_WINLOGO);

	if (!RegisterClassEx(&window))
		return 0;

	if (fullscreen)
	{
		DEVMODE deviceMode;
		memset(&deviceMode, 0, sizeof(deviceMode));
		deviceMode.dmSize = sizeof(deviceMode);
		deviceMode.dmPelsWidth = width;
		deviceMode.dmPelsHeight = height;
		deviceMode.dmBitsPerPel = bits;
		deviceMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		if (ChangeDisplaySettings(&deviceMode, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
		{
			MessageBox(NULL, "failed!", NULL, MB_OK);
			fullscreen = FALSE;
		}

	}

	if (fullscreen)
	{
		exStyle = WS_EX_APPWINDOW;
		Style = WS_POPUP;
		ShowCursor(FALSE);
	}
	else
	{
		exStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		Style = WS_OVERLAPPEDWINDOW;
	}

	AdjustWindowRectEx(&windowRect, Style, FALSE, exStyle);

	hwnd = CreateWindowEx(NULL,
		"program_na_5",
		"App",
		Style | WS_CLIPCHILDREN |
		WS_CLIPSIBLINGS,
		0, 0,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL,
		NULL,
		instance,
		NULL);

	if (!hwnd)
		return 0;

	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);

	isDone = false;
	InitializeOpenGLGraphic();

	while (!isDone)
	{
		PeekMessage(&msg, hwnd, NULL, NULL, PM_REMOVE);

		if (msg.message == WM_QUIT)
		{
			isDone = true;
		}
		else
		{
			if (keyPressed[VK_ESCAPE])
				isDone = true;
			else
			{
				Render();

				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}

	CleanUp();

	if (fullscreen)
	{
		ChangeDisplaySettings(NULL, 0);
		ShowCursor(TRUE);
	}
	return msg.wParam;
}