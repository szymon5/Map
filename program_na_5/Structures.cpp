#include<Windows.h>
#include<cstdlib>

typedef struct _math
{
	float angle = 0.0f;
	float radians = 0.0f;
}Math;

typedef struct _water
{
	float waterHeight = 155.0f;
	bool waterDirection = true;//true->w gore...false->w dol
}Water;

typedef struct _mouse
{
	float mouseX, mouseY;
	float oldMouseX, oldMouseY;
}Mouse;

typedef struct _camera
{
	float cameraX, cameraY, cameraZ;//pozycja kamery
	float lookX, lookY, lookZ;//punkt na ktory kamera sie patrzy
}Camera;

typedef struct _texture
{
	BITMAPINFOHEADER header;//pomocniczy naglowek obrazu
	BITMAPINFOHEADER island_header;//naglowek obrazu wyspy
	BITMAPINFOHEADER water_header;//naglowek obrazu wody
}Texture;

typedef struct _data
{
	unsigned char *imageData;//dane rozmieszczenia wierzcholkow na podstawie tekstury
	unsigned char *island_data;//dane wyspy
	unsigned char *water_data;//dane wody
}Data;

typedef struct _island
{
	static const int X = 32;
	static const int Z = 32;
}Island;

typedef struct _id
{
	unsigned int island_ID;
	unsigned int water_ID;
}ID;