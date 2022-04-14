
#define GLM_FORCE_RADIANS
#define GLM_FORCE_SWIZZLE

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include "constants.h"
#include "lodepng.h"
#include "shaderprogram.h"
#include "myCube.h"
#include "myTeapot.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define nic 1
#define sciana 2
#define przepasc 3
#define most 4
#define podloga 5
#define schody 6

GLuint tex;
GLuint tex2;//tego i 1 nie uzywam
GLuint tex3; //do podlogi
GLuint tex4;

GLuint tex5; //do cegiel
GLuint tex6;
GLuint tex7;
GLuint tex3b;
GLuint tex4b;

float speed_x = 0;//[radiany/s]
float speed_y = 0;//[radiany/s]
float walk_speed_x = 0;
float walk_speed_z = 0;

bool firstMouse = true;

glm::vec3 pos = glm::vec3(3.5f, 3.5f, 3.0f);
glm::vec3 zmiana = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec4 swiatlo = glm::vec4(3.5f, 7.5f, 7.0f, 1.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);

ShaderProgram *sp;
ShaderProgram *sp2;

float lastX = 800, lastY = 450;

float* c1 = myCubeC1;
float* c2 = myCubeC2;
float* c3 = myCubeC3;

//tablica zrodel swiatla
int ile_swiatel = 10;
glm::vec4 swiatla[] = { 
	glm::vec4(26,3.5,5,1),		glm::vec4(24,3.5,5,1),		glm::vec4(14,3.5,12,1),	glm::vec4(2,3.5,19,1),		glm::vec4(36,3.5,21,1),				//dol
	glm::vec4(14,3.5,25,1),		glm::vec4(36,3.5,31,1),	glm::vec4(2,3.5,29,1),		glm::vec4(14,3.5,33,1),	glm::vec4(38,3.5,37,1)					//dol

};



glm::vec3 calcDir(float kat_x, float kat_y) {
	glm::vec4 dir = glm::vec4(0, 0, 1, 0);
	glm::mat4 M = glm::rotate(glm::mat4(1.0f), kat_y, glm::vec3(0, 1, 0));
	M = glm::rotate(M, kat_x, glm::vec3(1, 0, 0));
	dir = M * dir;
	return glm::vec3(dir);
}

int mapa[20][20][5] = {};

void wczytanie() {
	std::ifstream mapa0("mapa0.txt");
	std::ifstream mapa1("mapa1.txt");
	std::ifstream mapa2("mapa2.txt");
	std::ifstream mapa3("mapa3.txt");
	std::ifstream mapa4("mapa4.txt");
	for (int y = 0; y < 20; y++) {
		for (int x = 0; x < 20; x++) {
			mapa0 >> mapa[x][y][0];
			mapa1 >> mapa[x][y][1];
			mapa2 >> mapa[x][y][2];
			mapa3 >> mapa[x][y][3];
			mapa4 >> mapa[x][y][4];
		}
	}
	mapa0.close();
	mapa1.close();
	mapa2.close();
	mapa3.close();
	mapa4.close();
}

//Procedura obsługi błędów
void error_callback(int error, const char* description) {
	fputs(description, stderr);
}

float deltaTime = 0.0f;	
float lastFrame = 0.0f;

void key_callback(
	GLFWwindow* window,
	int key,
	int scancode,
	int action,
	int mod
) {
	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_A) walk_speed_x = 1.0f;
		if (key == GLFW_KEY_D) walk_speed_x = -1.0f;
		if (key == GLFW_KEY_W) walk_speed_z = 1.0f;
		if (key == GLFW_KEY_S) walk_speed_z = -1.0f;

	}
	if (action == GLFW_RELEASE) {
		if (key == GLFW_KEY_A) walk_speed_x = 0.0f;
		if (key == GLFW_KEY_D) walk_speed_x = 0.0f;
		if (key == GLFW_KEY_W) walk_speed_z = 0.0f;
		if (key == GLFW_KEY_S) walk_speed_z = 0.0f;
	}
}

float yaw = 90.0f;
float pitch = 0.0f;

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 direction;
	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.y = sin(glm::radians(pitch));
	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(direction);
}

GLuint readTexture(const char* filename) {
	GLuint tex;
	glActiveTexture(GL_TEXTURE0);

	//Wczytanie do pamięci komputera
	std::vector<unsigned char> image;   //Alokuj wektor do wczytania obrazka
	unsigned width, height;   //Zmienne do których wczytamy wymiary obrazka
	//Wczytaj obrazek
	unsigned error = lodepng::decode(image, width, height, filename);

	//Import do pamięci karty graficznej
	glGenTextures(1, &tex); //Zainicjuj jeden uchwyt
	glBindTexture(GL_TEXTURE_2D, tex); //Uaktywnij uchwyt
	//Wczytaj obrazek do pamięci KG skojarzonej z uchwytem
	glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, (unsigned char*)image.data());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


	return tex;
}

void loadModel(std::string plik) {
	using namespace std;
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(plik, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals);
	cout << importer.GetErrorString() << endl;

	aiMesh* mesh = scene->mMeshes[0];
	for (int i = 0; i < mesh->mNumVertices; i++) {
		aiVector3D vertex = mesh->mVertices[i];
		cout << vertex.x << " " << vertex.y << " " << vertex.z << endl;

		aiVector3D normal = mesh->mNormals[i];
		cout << normal.x << " " << normal.y << " " << normal.z << endl;

		unsigned int liczba_zest = mesh->GetNumUVChannels();
		unsigned int liczba_wsp_tex = mesh->mNumUVComponents[0];
		aiVector3D texCoord = mesh->mTextureCoords[0][i];
		cout << texCoord.x << " " << texCoord.y << " " << texCoord.z << endl;

	}
}

//Procedura inicjująca
void initOpenGLProgram(GLFWwindow* window) {

	glClearColor(0, 0, 0, 1); //Ustaw kolor czyszczenia bufora kolorów
	glEnable(GL_DEPTH_TEST); //Włącz test głębokości na pikselach
	glfwSetKeyCallback(window, key_callback);

	tex3 = readTexture("Stone_Path_006_basecolor.png");		//to jest ok na podloge
	tex4 = readTexture("Stone_Path_006_height (1).png");

	tex3b = readTexture("Stone_Path_006_basecolor.png");		//to jest ok na podloge
	tex4b = readTexture("Stone_Path_006_height (1).png");


	sp=new ShaderProgram("v_simplest.glsl",NULL,"f_simplest.glsl");
	sp2 = new ShaderProgram("v_podloga.glsl", NULL, "f_podloga.glsl");
}


//Zwolnienie zasobów zajętych przez program
void freeOpenGLProgram(GLFWwindow* window) {
    //************Tutaj umieszczaj kod, który należy wykonać po zakończeniu pętli głównej************

	glDeleteTextures(1, &tex);
	glDeleteTextures(1, &tex2);
    delete sp;
    delete sp2;
}




//Procedura rysująca zawartość sceny
void drawScene(GLFWwindow* window,float kat_x,float kat_y) {

	//************Tutaj umieszczaj kod rysujący obraz******************l
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Wyczyść bufor koloru i bufor głębokości

	float speed = 0.15f;

	//--kolizje--
	int a, b;
	a = (int)(floor(pos.x + (cameraFront.x * walk_speed_z)));
	b = (int)(floor(pos.z + (cameraFront.z * walk_speed_z)));
	if (a % 2 == 0)
		a = a + 1;
	if (b % 2 == 0)
		b = b + 1;

	//std::cout << a << ", " << b << ", " << mapa[(a-1)/2][(b-1)/2][1] << std::endl;
	if (mapa[(a - 1) / 2][(b - 1) / 2][1] == sciana) speed = 0.0f;

	//if (mapa[(int)round((pos.x + cameraFront.x) / 2.0)][(int)round((pos.z + cameraFront.z) / 2.0)][1] == sciana) speed = 0.0f;

	pos += glm::vec3((cameraFront.x*speed*deltaTime*walk_speed_z), 0.0f, (cameraFront.z*speed*deltaTime*walk_speed_z));
	glm::mat4 V = glm::lookAt(pos ,pos + cameraFront, glm::vec3(0.0f, 1.0f, 0.0f));//Wylicz macierz widoku

	std::cout << pos.x << "," << pos.y << "," << pos.z << std::endl;

	glm::mat4 P = glm::perspective(glm::radians(50.0f), 1.0f, 0.1f, 50.0f); //Wylicz macierz rzutowania
	//Zamiast poniższych linijek należy wstawić kod dotyczący rysowania własnych obiektów (glDrawArrays/glDrawElements i wszystko dookoła)
	//----------
	//Tablica współrzędnych teksturowania

	float texCoords[] = {
					  1.0f, 0.0f,	//A
					  0.0f, 1.0f,    //B
					  0.0f, 0.0f,    //C

					  1.0f, 0.0f,    //A
					  1.0f, 1.0f,    //D
					  0.0f, 1.0f,    //B

					  1.0f, 0.0f,	//A
					  0.0f, 1.0f,    //B
					  0.0f, 0.0f,    //C

					  1.0f, 0.0f,    //A
					  1.0f, 1.0f,    //D
					  0.0f, 1.0f,    //B

					  1.0f, 0.0f,	//A
					  0.0f, 1.0f,    //B
					  0.0f, 0.0f,    //C

					  1.0f, 0.0f,    //A
					  1.0f, 1.0f,    //D
					  0.0f, 1.0f,    //B

					  1.0f, 0.0f,	//A
					  0.0f, 1.0f,    //B
					  0.0f, 0.0f,    //C

					  1.0f, 0.0f,    //A
					  1.0f, 1.0f,    //D
					  0.0f, 1.0f,    //B

					  1.0f, 0.0f,	//A
					  0.0f, 1.0f,    //B
					  0.0f, 0.0f,    //C

					  1.0f, 0.0f,    //A
					  1.0f, 1.0f,    //D
					  0.0f, 1.0f,    //B

					  1.0f, 0.0f,	//A
					  0.0f, 1.0f,    //B
					  0.0f, 0.0f,    //C

					  1.0f, 0.0f,    //A
					  1.0f, 1.0f,    //D
					  0.0f, 1.0f,    //B
	};

	float normals[] = {
	  0.0f,0.0f,-1.0f,0.0f, //A
	  0.0f,0.0f,-1.0f,0.0f, //B
	  0.0f,0.0f,-1.0f,0.0f, //C

	  0.0f,0.0f,-1.0f,0.0f, //A
	  0.0f,0.0f,-1.0f,0.0f, //D
	  0.0f,0.0f,-1.0f,0.0f, //B

	  0.0f,0.0f,1.0f,0.0f, //A
	  0.0f,0.0f,1.0f,0.0f, //B
	  0.0f,0.0f,1.0f,0.0f, //C

	  0.0f,0.0f,1.0f,0.0f, //A
	  0.0f,0.0f,1.0f,0.0f, //D
	  0.0f,0.0f,1.0f,0.0f, //B

	  0.0f,-1.0f,0.0f,0.0f, //A
	  0.0f,-1.0f,0.0f,0.0f, //B
	  0.0f,-1.0f,0.0f,0.0f, //C

	  0.0f,-1.0f,0.0f,0.0f, //A
	  0.0f,-1.0f,0.0f,0.0f, //D
	  0.0f,-1.0f,0.0f,0.0f, //B

	  0.0f,1.0f,0.0f,0.0f, //A
	  0.0f,1.0f,0.0f,0.0f, //B
	  0.0f,1.0f,0.0f,0.0f, //C

	  0.0f,1.0f,0.0f,0.0f, //A
	  0.0f,1.0f,0.0f,0.0f, //D
	  0.0f,1.0f,0.0f,0.0f, //B

	  -1.0f,0.0f,0.0f,0.0f, //A
	  -1.0f,0.0f,0.0f,0.0f, //B
	  -1.0f,0.0f,0.0f,0.0f, //C

	  -1.0f,0.0f,0.0f,0.0f, //A
	  -1.0f,0.0f,0.0f,0.0f, //D
	  -1.0f,0.0f,0.0f,0.0f, //B

	  1.0f,0.0f,0.0f,0.0f, //A
	  1.0f,0.0f,0.0f,0.0f, //B
	  1.0f,0.0f,0.0f,0.0f, //C

	  1.0f,0.0f,0.0f,0.0f, //A
	  1.0f,0.0f,0.0f,0.0f, //D
	  1.0f,0.0f,0.0f,0.0f, //B
	};

	//Liczba wierzchołków w tablicy
	int vertexCount = 6; 
	int z=-5;


	
	for (int z = -5; z < 0; z++) {
		for (int y = 0; y < 20; y++) {
			for (int x = 0; x < 20; x++) {
				glm::mat4 M = glm::mat4(1.0f); //Zainicjuj macierz modelu macierzą jednostkową
				M = glm::translate(M, glm::vec3((float)(x + 1) * 2 - 1, (float)(z + 1) * 2 - 1, (float)(y + 1) * 2 - 1));

				switch (mapa[x][y][0])
				{
				case sciana:
					sp2->use();
					//M = glm::translate(M, glm::vec3(0, -3, 0));
					//M = glm::scale(M, glm::vec3(1, 4, 1));

					glUniformMatrix4fv(sp2->u("P"), 1, false, glm::value_ptr(P));
					glUniformMatrix4fv(sp2->u("V"), 1, false, glm::value_ptr(V));

					glUniformMatrix4fv(sp2->u("M"), 1, false, glm::value_ptr(M));

					glEnableVertexAttribArray(sp2->a("vertex"));
					glVertexAttribPointer(sp2->a("vertex"), 4, GL_FLOAT, false, 0, myCubeVertices);
					glEnableVertexAttribArray(sp2->a("swiatla"));
					glVertexAttribPointer(sp2->a("swiatla"), 4, GL_FLOAT , false, 0, swiatla);
					glEnableVertexAttribArray(sp2->a("normal"));
					glVertexAttribPointer(sp2->a("normal"), 4, GL_FLOAT, false, 0, normals);
					glEnableVertexAttribArray(sp2->a("texCoord0"));
					glVertexAttribPointer(sp2->a("texCoord0"), 2, GL_FLOAT, false, 0, texCoords);

					glUniform1i(sp2->u("textureMap0"), 0);
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, tex3b);

					glUniform1i(sp2->u("textureMap1"), 1);
					glActiveTexture(GL_TEXTURE1);
					glBindTexture(GL_TEXTURE_2D, tex4b);


					glDrawArrays(GL_TRIANGLES, 0, myCubeVertexCount);
					glDisableVertexAttribArray(sp2->a("vertex"));
					glDisableVertexAttribArray(sp2->a("normal"));
					glDisableVertexAttribArray(sp2->a("texCoord0"));
					glDisableVertexAttribArray(sp2->a("swiatla"));


					break;

				case podloga:
					sp2->use();
					//M = glm::translate(M, glm::vec3(0,-3,0));
					//M = glm::scale(M, glm::vec3(1, 4, 1));

					glUniformMatrix4fv(sp2->u("P"), 1, false, glm::value_ptr(P));
					glUniformMatrix4fv(sp2->u("V"), 1, false, glm::value_ptr(V));

					glUniformMatrix4fv(sp2->u("M"), 1, false, glm::value_ptr(M));

					glEnableVertexAttribArray(sp2->a("vertex"));
					glVertexAttribPointer(sp2->a("vertex"), 4, GL_FLOAT, false, 0, myCubeVertices);
					glEnableVertexAttribArray(sp2->a("normal"));
					glVertexAttribPointer(sp2->a("normal"), 4, GL_FLOAT, false, 0, normals);
					glEnableVertexAttribArray(sp2->a("texCoord0"));
					glVertexAttribPointer(sp2->a("texCoord0"), 2, GL_FLOAT, false, 0, texCoords);
					glEnableVertexAttribArray(sp2->a("swiatla"));
					glVertexAttribPointer(sp2->a("swiatla"), 4, GL_FLOAT, false, 0, swiatla);

					glUniform1i(sp2->u("textureMap0"), 0);
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, tex3);

					glUniform1i(sp2->u("textureMap1"), 1);
					glActiveTexture(GL_TEXTURE1);
					glBindTexture(GL_TEXTURE_2D, tex4);


					glDrawArrays(GL_TRIANGLES, 0, myCubeVertexCount);
					glDisableVertexAttribArray(sp2->a("vertex"));
					glDisableVertexAttribArray(sp2->a("normal"));
					glDisableVertexAttribArray(sp2->a("texCoord0"));
					glDisableVertexAttribArray(sp2->a("swiatla"));
					break;

				default:
					break;
				}


				//-----------
			}
		}
	}

	for (int z = 5; z < 10; z++) {
		for (int y = 0; y < 20; y++) {
			for (int x = 0; x < 20; x++) {
				glm::mat4 M = glm::mat4(1.0f); //Zainicjuj macierz modelu macierzą jednostkową
				M = glm::translate(M, glm::vec3((float)(x + 1) * 2 - 1, (float)(z + 1) * 2 - 1, (float)(y + 1) * 2 - 1));

				switch (mapa[x][y][0])
				{
				case sciana:
					sp2->use();
					//M = glm::translate(M, glm::vec3(0, -3, 0));
					//M = glm::scale(M, glm::vec3(1, 4, 1));

					glUniformMatrix4fv(sp2->u("P"), 1, false, glm::value_ptr(P));
					glUniformMatrix4fv(sp2->u("V"), 1, false, glm::value_ptr(V));

					glUniformMatrix4fv(sp2->u("M"), 1, false, glm::value_ptr(M));

					glEnableVertexAttribArray(sp2->a("vertex"));
					glVertexAttribPointer(sp2->a("vertex"), 4, GL_FLOAT, false, 0, myCubeVertices);
					glEnableVertexAttribArray(sp2->a("normal"));
					glVertexAttribPointer(sp2->a("normal"), 4, GL_FLOAT, false, 0, normals);
					glEnableVertexAttribArray(sp2->a("texCoord0"));
					glVertexAttribPointer(sp2->a("texCoord0"), 2, GL_FLOAT, false, 0, texCoords);
					glEnableVertexAttribArray(sp2->a("swiatla"));
					glVertexAttribPointer(sp2->a("swiatla"), 4, GL_FLOAT, false, 0, swiatla);

					glUniform1i(sp2->u("textureMap0"), 0);
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, tex3b);

					glUniform1i(sp2->u("textureMap1"), 1);
					glActiveTexture(GL_TEXTURE1);
					glBindTexture(GL_TEXTURE_2D, tex4b);


					glDrawArrays(GL_TRIANGLES, 0, myCubeVertexCount);
					glDisableVertexAttribArray(sp2->a("vertex"));
					glDisableVertexAttribArray(sp2->a("normal"));
					glDisableVertexAttribArray(sp2->a("texCoord0"));
					glDisableVertexAttribArray(sp2->a("swiatla"));


					break;

				case podloga:
					sp2->use();
					//M = glm::translate(M, glm::vec3(0,-3,0));
					//M = glm::scale(M, glm::vec3(1, 4, 1));

					glUniformMatrix4fv(sp2->u("P"), 1, false, glm::value_ptr(P));
					glUniformMatrix4fv(sp2->u("V"), 1, false, glm::value_ptr(V));

					glUniformMatrix4fv(sp2->u("M"), 1, false, glm::value_ptr(M));

					glEnableVertexAttribArray(sp2->a("vertex"));
					glVertexAttribPointer(sp2->a("vertex"), 4, GL_FLOAT, false, 0, myCubeVertices);
					glEnableVertexAttribArray(sp2->a("normal"));
					glVertexAttribPointer(sp2->a("normal"), 4, GL_FLOAT, false, 0, normals);
					glEnableVertexAttribArray(sp2->a("texCoord0"));
					glVertexAttribPointer(sp2->a("texCoord0"), 2, GL_FLOAT, false, 0, texCoords);
					glEnableVertexAttribArray(sp2->a("swiatla"));
					glVertexAttribPointer(sp2->a("swiatla"), 4, GL_FLOAT, false, 0, swiatla);

					glUniform1i(sp2->u("textureMap0"), 0);
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, tex3);

					glUniform1i(sp2->u("textureMap1"), 1);
					glActiveTexture(GL_TEXTURE1);
					glBindTexture(GL_TEXTURE_2D, tex4);


					glDrawArrays(GL_TRIANGLES, 0, myCubeVertexCount);
					glDisableVertexAttribArray(sp2->a("vertex"));
					glDisableVertexAttribArray(sp2->a("normal"));
					glDisableVertexAttribArray(sp2->a("texCoord0"));
					glDisableVertexAttribArray(sp2->a("swiatla"));
					break;

				default:
					break;
				}


				//-----------
			}
		}
	}

	for (int z = 0; z < 5; z++) {
		for (int y = 0; y < 20; y++) {
			for (int x = 0; x < 20; x++) {
				glm::mat4 M = glm::mat4(1.0f); //Zainicjuj macierz modelu macierzą jednostkową
				M = glm::translate(M, glm::vec3((float)(x + 1) * 2 - 1, (float)(z + 1) * 2 - 1, (float)(y + 1) * 2 - 1));

				switch (mapa[x][y][z])
				{
				case sciana:
					sp2->use();
					//M = glm::translate(M, glm::vec3(0, -3, 0));
					//M = glm::scale(M, glm::vec3(1, 4, 1));

					glUniformMatrix4fv(sp2->u("P"), 1, false, glm::value_ptr(P));
					glUniformMatrix4fv(sp2->u("V"), 1, false, glm::value_ptr(V));

					glUniformMatrix4fv(sp2->u("M"), 1, false, glm::value_ptr(M));

					glEnableVertexAttribArray(sp2->a("vertex"));
					glVertexAttribPointer(sp2->a("vertex"), 4, GL_FLOAT, false, 0, myCubeVertices);
					glEnableVertexAttribArray(sp2->a("normal"));
					glVertexAttribPointer(sp2->a("normal"), 4, GL_FLOAT, false, 0, normals);
					glEnableVertexAttribArray(sp2->a("texCoord0"));
					glVertexAttribPointer(sp2->a("texCoord0"), 2, GL_FLOAT, false, 0, texCoords);
					glEnableVertexAttribArray(sp2->a("swiatla"));
					glVertexAttribPointer(sp2->a("swiatla"), 4, GL_FLOAT, false, 0, swiatla);

					glUniform1i(sp2->u("textureMap0"), 0);
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, tex3b);

					glUniform1i(sp2->u("textureMap1"), 1);
					glActiveTexture(GL_TEXTURE1);
					glBindTexture(GL_TEXTURE_2D, tex4b);


					glDrawArrays(GL_TRIANGLES, 0, myCubeVertexCount);
					glDisableVertexAttribArray(sp2->a("vertex"));
					glDisableVertexAttribArray(sp2->a("normal"));
					glDisableVertexAttribArray(sp2->a("texCoord0"));
					glDisableVertexAttribArray(sp2->a("swiatla"));
				

					break;

				case podloga:
					sp2->use();
					//M = glm::translate(M, glm::vec3(0,-3,0));
					//M = glm::scale(M, glm::vec3(1, 4, 1));

					glUniformMatrix4fv(sp2->u("P"), 1, false, glm::value_ptr(P));
					glUniformMatrix4fv(sp2->u("V"), 1, false, glm::value_ptr(V));

					glUniformMatrix4fv(sp2->u("M"), 1, false, glm::value_ptr(M));

					glEnableVertexAttribArray(sp2->a("vertex"));
					glVertexAttribPointer(sp2->a("vertex"), 4, GL_FLOAT, false, 0, myCubeVertices);
					glEnableVertexAttribArray(sp2->a("normal"));
					glVertexAttribPointer(sp2->a("normal"), 4, GL_FLOAT, false, 0, normals);
					glEnableVertexAttribArray(sp2->a("texCoord0"));
					glVertexAttribPointer(sp2->a("texCoord0"), 2, GL_FLOAT, false, 0, texCoords);
					glEnableVertexAttribArray(sp2->a("swiatla"));
					glVertexAttribPointer(sp2->a("swiatla"), 4, GL_FLOAT, false, 0, swiatla);

					glUniform1i(sp2->u("textureMap0"), 0);
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, tex3);

					glUniform1i(sp2->u("textureMap1"), 1);
					glActiveTexture(GL_TEXTURE1);
					glBindTexture(GL_TEXTURE_2D, tex4);
					

					glDrawArrays(GL_TRIANGLES, 0, myCubeVertexCount);
					glDisableVertexAttribArray(sp2->a("vertex"));
					glDisableVertexAttribArray(sp2->a("normal"));
					glDisableVertexAttribArray(sp2->a("texCoord0"));
					glDisableVertexAttribArray(sp2->a("swiatla"));
					break;

				default:
					break;
				}


				//-----------
			}
		}
	}

	glfwSwapBuffers(window); //Skopiuj bufor tylny do bufora przedniego
}



int main(void)
{
	wczytanie();

	GLFWwindow* window; //Wskaźnik na obiekt reprezentujący okno

	glfwSetErrorCallback(error_callback);//Zarejestruj procedurę obsługi błędów

	

	if (!glfwInit()) { //Zainicjuj bibliotekę GLFW
		fprintf(stderr, "Nie można zainicjować GLFW.\n");
		exit(EXIT_FAILURE);
	}

	window = glfwCreateWindow(1600, 900, "OpenGL", NULL, NULL);  //Utwórz okno 500x500 o tytule "OpenGL" i kontekst OpenGL.
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (!window) //Jeżeli okna nie udało się utworzyć, to zamknij program
	{
		fprintf(stderr, "Nie można utworzyć okna.\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window); //Od tego momentu kontekst okna staje się aktywny i polecenia OpenGL będą dotyczyć właśnie jego.
	glfwSwapInterval(1); //Czekaj na 1 powrót plamki przed pokazaniem ukrytego bufora

	if (glewInit() != GLEW_OK) { //Zainicjuj bibliotekę GLEW
		fprintf(stderr, "Nie można zainicjować GLEW.\n");
		exit(EXIT_FAILURE);
	}

	initOpenGLProgram(window); //Operacje inicjujące

	//Główna pętla
	float angle = 0;
	float kat_x = 0; //zadeklaruj zmienną przechowującą aktualny kąt obrotu
	float kat_y = 0; //zadeklaruj zmienną przechowującą aktualny kąt obrotu

	float currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;

	glfwSetTime(0); //Wyzeruj licznik czasu
	while (!glfwWindowShouldClose(window)) //Tak długo jak okno nie powinno zostać zamknięte
	{
		kat_x += speed_x * glfwGetTime();
		kat_y += speed_y * glfwGetTime();

		//pos += (float)(walk_speed * glfwGetTime());
		glfwSetTime(0); //Wyzeruj licznik czasu
		drawScene(window, kat_x, kat_y); //Wykonaj procedurę rysującą
		glfwPollEvents(); //Wykonaj procedury callback w zalezności od zdarzeń jakie zaszły.
	}

	freeOpenGLProgram(window);

	glfwDestroyWindow(window); //Usuń kontekst OpenGL i okno
	glfwTerminate(); //Zwolnij zasoby zajęte przez GLFW
	exit(EXIT_SUCCESS);
}
