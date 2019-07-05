#include "tinyxml\tinystr.h"
#include "tinyxml\tinyxml.h"
#include <stdio.h>

#include <IL/il.h>


#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glew.h>
#include <GL/glut.h>
#endif


#define _USE_MATH_DEFINES
#include <math.h>

#include <iostream>
#include <vector>
#include <fstream>
#include <cstring>
#include <sstream>


using namespace std;


float alfa = 0.0f, beta = 0.0f, radius = 5.0f;
float camX, camY, camZ;
int timebase = 0, frame = 0;

GLuint texc;


float fps = 0;
int secs;


int loadTexture(std::string s);


void buildRotMatrix(float *x, float *y, float *z, float *m) {

	m[0] = x[0]; m[1] = x[1]; m[2] = x[2]; m[3] = 0;
	m[4] = y[0]; m[5] = y[1]; m[6] = y[2]; m[7] = 0;
	m[8] = z[0]; m[9] = z[1]; m[10] = z[2]; m[11] = 0;
	m[12] = 0; m[13] = 0; m[14] = 0; m[15] = 1;
}


void cross(float *a, float *b, float *res) {

	res[0] = a[1] * b[2] - a[2] * b[1];
	res[1] = a[2] * b[0] - a[0] * b[2];
	res[2] = a[0] * b[1] - a[1] * b[0];
}


void normalize(float *a) {

	float l = sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
	a[0] = a[0] / l;
	a[1] = a[1] / l;
	a[2] = a[2] / l;
}


float length(float *v) {

	float res = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	return res;

}

void multMatrixVector(float *m, float *v, float *res) {

	for (int j = 0; j < 4; ++j) {
		res[j] = 0;
		for (int k = 0; k < 4; ++k) {
			res[j] += v[k] * m[j * 4 + k];
		}
	}

}


void getCatmullRomPoint(float t, float *p0, float *p1, float *p2, float *p3, float *pos, float *deriv) {

	// catmull-rom matrix
	float m[4][4] = { { -0.5f,  1.5f, -1.5f,  0.5f },
	{ 1.0f, -2.5f,  2.0f, -0.5f },
	{ -0.5f,  0.0f,  0.5f,  0.0f },
	{ 0.0f,  1.0f,  0.0f,  0.0f } };

	// Compute A = M * P
	float a[4][4];

	float px[4] = { p0[0], p1[0], p2[0], p3[0] };
	float py[4] = { p0[1], p1[1], p2[1], p3[1] };
	float pz[4] = { p0[2], p1[2], p2[2], p3[2] };
	float pw[4] = { p0[3], p1[3], p2[3], p3[3] };

	multMatrixVector((float *)m, px, a[0]);
	multMatrixVector((float *)m, py, a[1]);
	multMatrixVector((float *)m, pz, a[2]);
	multMatrixVector((float *)m, pw, a[3]);

	float tv[4] = { powf(t, 3.0), powf(t, 2.0), t, 1.0 };
	float tdv[4] = { 3.0 * powf(t, 2.0), 2.0f * t, 1.0, 0.0 };

	// Compute pos = T * A
	multMatrixVector((float *)a, tv, pos);
	// Compute deriv = T' * A
	//multMatrixVector((float *)a, tdv, deriv);
}





// given  global t, returns the point in the curve
void getGlobalCatmullRomPoint(float gt, float *pos, float *deriv, vector<vector<float>> pc, int npc) {

	const int POINT_COUNT = npc;

	//float p[4][3] = { { -1,0,-1 },{ -1,0,1 },{ 1,0,1 },{ 1,0,-1 } };
	float** p = new float*[POINT_COUNT];
	for (int i = 0; i < POINT_COUNT; i++)
		p[i] = new float[3];


	for (int i = 0; i < npc; i++) {
		for (int j = 0; j < 3; j++) {
			p[i][j] = pc[i][j];
		}
	}


	// Points that make up the loop for catmull-rom interpolation

	float t = gt * POINT_COUNT; // this is the real global t
	int index = floor(t);  // which segment
	t = t - index; // where within  the segment

				   // indices store the points
	int indices[4];
	indices[0] = (index + POINT_COUNT - 1) % POINT_COUNT;
	indices[1] = (indices[0] + 1) % POINT_COUNT;
	indices[2] = (indices[1] + 1) % POINT_COUNT;
	indices[3] = (indices[2] + 1) % POINT_COUNT;

	getCatmullRomPoint(t, p[indices[0]], p[indices[1]], p[indices[2]], p[indices[3]], pos, deriv);

	//free
	for (int i = 0; i < POINT_COUNT; i++)
		delete[] p[i];
	delete[] p;

}



class Group
{
	public:
	Group::Group() {
		id = 0;
	}
	Group::Group(int a) {
		id = a;
	}
	int getId() {
		return id;
	}

	virtual int apply() {

		glPushMatrix();

		return 0;
	}

	virtual int prepare() {

		return 0;
	}

	private:
	int id;
};


class Catmull : public Group {

public:
	Catmull::Catmull(float t, vector<float> pontos) {

		flag = 1;
		time = t;
		rangle = 0;
		for (int i = 0; i < pontos.size(); i += 3) {

			vector<float> aux;

			aux.push_back(pontos[i]);
			aux.push_back(pontos[i + 1]);
			aux.push_back(pontos[i + 2]);

			pc.push_back(aux);
		}
	}

	Catmull::Catmull(float t, float xx, float yy, float zz) {

		rangle = 0;
		xa = xx;
		ya = yy;
		za = zz;

		time = t;
		flag = 0;
	}

	int apply() {


		static float t = 0;
		static float up[4] = { 0, 1, 0, 0 };

		float pos[4], deriv[4];
		float m[4 * 4];
		float y[4], z[4];


		if (flag) {
			getGlobalCatmullRomPoint(t, pos, deriv, pc, pc.size());

			cross(deriv, up, z);
			cross(z, deriv, y);

			normalize(deriv);
			normalize(y);
			normalize(z);

			glTranslatef(pos[0], pos[1], pos[2]);
		}

		else {

			if (fps) {

				float angaux = (360 / (fps*time));
				rangle += angaux;

				rangle = fmod(rangle, 360);

				glRotatef(rangle, xa, ya, za);

			}
		}


		//glutWireTeapot(0.5);


		up[0] = y[0]; up[1] = y[1]; up[2] = y[2]; up[3] = y[3];


		if (fps) {
			t += 1 / (fps*time);
		}



		return 2;
	}

private:
	float time;
	int flag;
	float xa, ya, za;
	vector<vector<float>> pc;
	float rangle;
};



class Scale : public Group {

public:
	Scale::Scale(float a, float b, float c) {
		x = a;
		y = b;
		z = c;
	}
	Scale::Scale() {
		x = 0;
		y = 0;
		z = 0;
	}

	int apply() {

		glScalef(x, y, z);
		return 2;
	}

private:
	float x, y, z;
};


class Translate : public Group {

public:
	Translate::Translate(float a, float b, float c) {
		x = a;
		y = b;
		z = c;
	}
	Translate::Translate() {
		x = 0;
		y = 0;
		z = 0;
	}

	int apply() {

		glTranslatef(x, y, z);
		return 3;
	}

private:
	float x, y, z;
};



class Rotate : public Group {

public:
	Rotate::Rotate(float l, float a, float b, float c) {
		angle = l;
		x = a;
		y = b;
		z = c;
	}
	Rotate::Rotate() {
		x = 0;
		y = 0;
		z = 0;
		angle = 0;
	}

	int apply() {

		glRotatef(angle, x, y, z);
		return 1;

	}


private:
	float x, y, z, angle;
};





class Model : public Group {

public:

	Model::Model(string s, string t) {
		modelo = s;
		texture = t;
	}

	Model::Model(string s) {
		modelo = s;
	}

	string getModelo() {
		return modelo;
	}


	int prepare() {

		ifstream file(modelo);
		string str;

		getline(file, str);

		int numvertices;
		istringstream ss(str);

		ss >> numvertices;

		nvertices = numvertices;



		vector<float> position, textures, normals;

		position.clear();
		normals.clear();
		textures.clear();

		int i = 2;

		while (getline(file, str)) {



			float v1, v2, v3;

			istringstream ss(str);

			ss >> v1;
			ss >> v2;
			ss >> v3;


			if (i < numvertices + 2) {

				position.push_back(v1);
				position.push_back(v2);
				position.push_back(v3);
			}
			else {

				if ((i >= numvertices + 2) && (i < 2 * numvertices + 2)) {
					normals.push_back(v1);
					normals.push_back(v2);
					normals.push_back(v3);
				}

				else {
					textures.push_back(v1);
					textures.push_back(v2);
				}


			}



			i++;
		}




		glGenBuffers(1, &buffer);
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		glBufferData(GL_ARRAY_BUFFER, position.size() * sizeof(float), &(position[0]), GL_STATIC_DRAW);

		glGenBuffers(1, &normalsgl);
		glBindBuffer(GL_ARRAY_BUFFER, normalsgl);
		glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), &(normals[0]), GL_STATIC_DRAW);

		glGenBuffers(1, &texCoord);
		glBindBuffer(GL_ARRAY_BUFFER, texCoord);
		glBufferData(GL_ARRAY_BUFFER, textures.size() * sizeof(float), &(textures[0]), GL_STATIC_DRAW);


		obj_text = loadTexture(texture);

		return 0;
	}



	int apply() {

		if (!strcmp(modelo.c_str(), "sphere.3d") == 0) {

			glDisable(GL_LIGHTING);
			glDisable(GL_LIGHT0);
		}

		glBindTexture(GL_TEXTURE_2D, obj_text);


		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		glVertexPointer(3, GL_FLOAT, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, normalsgl);
		glNormalPointer(GL_FLOAT, 0, 0);


		glBindBuffer(GL_ARRAY_BUFFER, texCoord);
		glTexCoordPointer(2, GL_FLOAT, 0, 0);

		glDrawArrays(GL_TRIANGLES, 0, nvertices * 3);

		glBindTexture(GL_TEXTURE_2D, 0);


		if (!strcmp(modelo.c_str(), "sphere.3d") == 0) {

			glEnable(GL_LIGHTING);
			glEnable(GL_LIGHT0);
		}


		return 4;
	}

public:
	string modelo;
	string texture;
	int nvertices;
	GLuint buffer, texCoord, normalsgl;
	GLuint obj_text;
};




typedef struct node {

	Group* g;
	char* label;
	vector<struct node*> sons;

} *Arvore;




Arvore cg;
int idx = 0;


int xml_parser(char* fxml) {


	string fich_xml = (string)fxml;

	int num = 0;
	int cap = 0;

	//Objeto para ler XML
	TiXmlDocument doc;

	if (!doc.LoadFile(fich_xml.c_str())) {

		//nome do ficheiro de configuracao (path) nao enontrado a partir da diretoria corrente
		printf("Erro ao carregar o ficheiro de configuracao XML.\n");
		return 1;
	}


	TiXmlNode* base = doc.FirstChild();

	if (strcmp(base->Value(), "scene") != 0) {

		printf("XML nao comeca com o elemento \" <scene> \"\n");
		return 1;
	}



	TiXmlElement* elementos = base->FirstChildElement("group");
	TiXmlElement* modelos;


	if (elementos == NULL) {

		printf("base-> nenhum elemento encontradoo");
		return 0; // <group> nao encontrado
	}



	cg = new struct node;

	Arvore cgaux = new struct node;

	cgaux = cg;
	cgaux->g = new Group(idx++);
	cgaux->label = "group";



	// queue de apontadores de elementos para percorrer a arvore de hierarquias
	vector<TiXmlElement*> stackgroups;

	//queue que guarda os n�s para adicionar os seus possiveis filhos posteriormente
	vector<struct node *>stack_nodes_group;


	// come�ar a percorrer a hierarquia
	elementos = elementos->FirstChildElement();

	while (elementos != NULL) {

		if (strcmp(elementos->Value(), "group") == 0) {

			stackgroups.push_back(elementos->FirstChildElement());

			Arvore aux = new struct node;
			aux->g = new Group(idx++);
			aux->label = "group";
			aux->sons.clear();

			cgaux->sons.push_back(aux);

			stack_nodes_group.push_back(aux);

			cap++;
		}


		if (strcmp(elementos->Value(), "scale") == 0 || strcmp(elementos->Value(), "translate") == 0) {

			float x, y, z;
			x = y = z = 0.0;


			if (elementos->Attribute("X")) {
				x = atof(elementos->Attribute("X"));
			}

			if (elementos->Attribute("Y")) {
				y = atof(elementos->Attribute("Y"));
			}

			if (elementos->Attribute("Z")) {
				z = atof(elementos->Attribute("Z"));
			}


			if (strcmp(elementos->Value(), "scale") == 0) {


				Arvore aux = new struct node;
				aux->g = new Scale(x, y, z);
				aux->label = "scale";
				aux->sons.clear();

				cgaux->sons.push_back(aux);
			}
			else {

				float ttime;

				if (elementos->Attribute("time")) {
					ttime = atof(elementos->Attribute("time"));

					TiXmlElement *pontos;

					pontos = elementos;
					pontos = pontos->FirstChildElement("point");


					vector<float> pcontrolo;

					while (pontos != NULL) {

						if (pontos->Attribute("X")) {
							x = atof(pontos->Attribute("X"));
						}

						if (pontos->Attribute("Y")) {
							y = atof(pontos->Attribute("Y"));
						}

						if (pontos->Attribute("Z")) {
							z = atof(pontos->Attribute("Z"));
						}

						pcontrolo.push_back(x);
						pcontrolo.push_back(y);
						pcontrolo.push_back(z);

						pontos = pontos->NextSiblingElement();

					}


					Arvore aux = new struct node;
					aux->g = new Catmull(ttime, pcontrolo);
					aux->label = "catmull";
					aux->sons.clear();

					cgaux->sons.push_back(aux);
				}

				else {

					Arvore aux = new struct node;
					aux->g = new Translate(x, y, z);
					aux->label = "translate";
					aux->sons.clear();

					cgaux->sons.push_back(aux);
				}



			}



		}

		if (strcmp(elementos->Value(), "rotate") == 0) {

			float x, y, z, angle;
			x = y = z = angle = 0.0;

			float ttime;


			if (elementos->Attribute("axisX")) {
				x = atof(elementos->Attribute("axisX"));
			}

			if (elementos->Attribute("axisY")) {
				y = atof(elementos->Attribute("axisY"));
			}

			if (elementos->Attribute("axisZ")) {
				z = atof(elementos->Attribute("axisZ"));
			}

			if (elementos->Attribute("angle")) {
				angle = atof(elementos->Attribute("angle"));
			}

			if (elementos->Attribute("time")) {
				ttime = atof(elementos->Attribute("time"));

				Arvore aux = new struct node;
				aux->g = new Catmull(ttime, x, y, z);
				aux->label = "catmull";
				aux->sons.clear();
				cgaux->sons.push_back(aux);

			}
			else {

				Arvore aux = new struct node;
				aux->g = new Rotate(angle, x, y, z);
				aux->label = "rotate";
				aux->sons.clear();

				cgaux->sons.push_back(aux);
			}


		}



		if (strcmp(elementos->Value(), "models") == 0) {

			modelos = elementos;
			modelos = modelos->FirstChildElement("model");

			while (modelos != NULL) {

				const char *nome = modelos->Attribute("file");
				const char *text;


				FILE *test;
				if ((fopen_s(&test, nome, "r")) == 0) {

					cout << "Ficheiro \'" << nome << "\' importado com sucesso." << endl;

				}

				Arvore aux = new struct node;

				if (modelos->Attribute("texture")) {

					aux->g = new Model((char*)nome, (char*)modelos->Attribute("texture"));
				}
				else

					aux->g = new Model((char*)nome);


				aux->label = "model";
				aux->sons.clear();

				cgaux->sons.push_back(aux);

				modelos = modelos->NextSiblingElement();
			}

		}



		elementos = elementos->NextSiblingElement();

		if (elementos == NULL) {

			if (cap != 0) {

				elementos = stackgroups[num];

				cgaux = stack_nodes_group[num];

				num++;
				cap--;

			}

		}

	}

	return 0;
}



void depth_first(struct node *senpai, struct node* xx) {


	if (senpai != NULL) {
		if (strcmp(senpai->label, "group") == 0 && xx == NULL) {

			glPopMatrix();

		}
	}


	if (xx != NULL) {


		(xx->g)->apply(); //executar a respetiva transforma�ao/draw da classe


		int ssize = xx->sons.size();

		for (int i = 0; i <= ssize; i++) {

			if (i<ssize)
				depth_first(xx, xx->sons[i]);
			else
				depth_first(xx, NULL);

		}
	}


}




void prepare_depth_first(struct node *senpai, struct node* xx) {

	if (xx != NULL) {

		(xx->g)->prepare(); //executar a respetiva transforma�ao/prepare da classe

		int ssize = xx->sons.size();

		for (int i = 0; i <= ssize; i++) {

			if (i<ssize)
				prepare_depth_first(xx, xx->sons[i]);
			else
				prepare_depth_first(xx, NULL);

		}
	}


}



void draw_models() {

	//perccorer a arvore de classes
	depth_first(NULL, cg);

	glPopMatrix();

}



void spherical2Cartesian() {

	camX = radius * cos(beta) * sin(alfa);
	camY = radius * sin(beta);
	camZ = radius * cos(beta) * cos(alfa);
}


void changeSize(int w, int h) {

	// Prevent a divide by zero, when window is too short
	// (you cant make a window with zero width).
	if (h == 0)
		h = 1;

	// compute window's aspect ratio 
	float ratio = w * 1.0 / h;

	// Reset the coordinate system before modifying
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// Set the viewport to be the entire window
	glViewport(0, 0, w, h);

	// Set the correct perspective
	gluPerspective(45, ratio, 1, 1000);

	// return to the model view matrix mode
	glMatrixMode(GL_MODELVIEW);
}




void renderTeapot() {
	static float t = 0;
	static float up[4] = { 0, 1, 0, 0 };

	float pos[4], deriv[4];
	float m[4 * 4];
	float y[4], z[4];


	float p[4][3] = { { -1,0,-1 },{ -1,0,1 },{ 1,0,1 },{ 1,0,-1 } };

	vector<vector<float>> asd;

	vector<float> aux;
	for (int i = 0; i < 4; i++) {

		for (int j = 0; j < 3; j++)
			aux.push_back(p[i][j]);

		asd.push_back(aux);
	}


	getGlobalCatmullRomPoint(t, pos, deriv, asd, 4);

	cross(deriv, up, z);
	cross(z, deriv, y);

	normalize(deriv);
	normalize(y);
	normalize(z);

	buildRotMatrix(deriv, y, z, m);
	glTranslatef(pos[0], pos[1], pos[2]);
	glMultMatrixf(m);

	glutWireTeapot(1);

	up[0] = y[0]; up[1] = y[1]; up[2] = y[2]; up[3] = y[3];


	if (fps)
		t += 1 / (fps*secs);

}




void renderScene(void) {


	// clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// set the camera
	glLoadIdentity();
	gluLookAt(camX, camY, camZ,
		0.0, 0.0, 0.0,
		0.0f, 1.0f, 0.0f);


	int time;
	char s[64];

	frame++;
	time = glutGet(GLUT_ELAPSED_TIME);

	if (time - timebase > 1000) {
		fps = frame * 1000.0 / (time - timebase);
		timebase = time;
		frame = 0;
		sprintf(s, "FPS: %f6.2", fps);
		glutSetWindowTitle(s);

	}


	glColor3f(0.5f, 0.5f, 0.5f);


	draw_models();



	// End of frame
	glutSwapBuffers();


}


void processKeys(unsigned char c, int xx, int yy) {

	// put code to process regular keys in here

}


void processSpecialKeys(int key, int xx, int yy) {

	switch (key) {

	case GLUT_KEY_RIGHT:
		alfa -= 0.1; break;

	case GLUT_KEY_LEFT:
		alfa += 0.1; break;

	case GLUT_KEY_UP:
		beta += 0.1f;
		if (beta > 1.5f)
			beta = 1.5f;
		break;

	case GLUT_KEY_DOWN:
		beta -= 0.1f;
		if (beta < -1.5f)
			beta = -1.5f;
		break;

	case GLUT_KEY_F2: radius -= 0.1f;
		if (radius < 0.1f)
			radius = 0.1f;
		break;

	case GLUT_KEY_F1: radius += 0.1f; break;
	}
	spherical2Cartesian();
	glutPostRedisplay();

}


void printInfo() {

	printf("Vendor: %s\n", glGetString(GL_VENDOR));
	printf("Renderer: %s\n", glGetString(GL_RENDERER));
	printf("Version: %s\n", glGetString(GL_VERSION));

	printf("\nUse Arrows to move the camera up/down and left/right\n");
	printf("F1 and F2 control the distance from the camera to the origin\n\n");

}



void initGL() {

	// alguns settings para OpenGL
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);


	glClearColor(0, 0, 0, 0);

	glColor3f(1.0f, 1.0f, 1.0f);


	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glEnable(GL_TEXTURE_2D);


}



int loadTexture(std::string s) {

	unsigned int t, tw, th;
	unsigned char *texData;
	unsigned int texID;

	ilInit();
	ilEnable(IL_ORIGIN_SET);
	ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
	ilGenImages(1, &t);
	ilBindImage(t);
	ilLoadImage((ILstring)s.c_str());
	tw = ilGetInteger(IL_IMAGE_WIDTH);
	th = ilGetInteger(IL_IMAGE_HEIGHT);
	ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
	texData = ilGetData();

	glGenTextures(1, &texID);

	glBindTexture(GL_TEXTURE_2D, texID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tw, th, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);

	return texID;

}



int main(int argc, char **argv) {

	// init GLUT and the window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(800, 800);
	glutCreateWindow("MOTOR 3D");



	// Required callback registry 
	glutDisplayFunc(renderScene);
	glutIdleFunc(renderScene);
	glutReshapeFunc(changeSize);



	// Callback registration for keyboard processing
	glutKeyboardFunc(processKeys);
	glutSpecialFunc(processSpecialKeys);



	spherical2Cartesian();

	printInfo();

	if (argc < 2) {
		printf("[Loading files] Ficheiro de configuracao nao encontrado!\n");

#ifndef __APPLE__
		glewInit();
#endif
		xml_parser("config1.xml");
		glutMainLoop();
		return 0;
	}


#ifndef __APPLE__	
	// init GLEW
	glewInit();
#endif	


	initGL();
	//texc = loadTexture("earth.jpg");



	xml_parser(argv[1]);

	prepare_depth_first(NULL, cg);

	// enter GLUT's main cycle
	glutMainLoop();

	return 1;
}
