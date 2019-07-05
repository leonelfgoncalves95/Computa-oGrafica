// motor.cpp: Lê um ficheiro com instruções do número de triangulos a desenhar, bem como as cores e vertices de cada triangulo, e desenha-os.

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glew.h>
#include <GL/glut.h>
#endif

#define _USE_MATH_DEFINES
#include <math.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include "tinyxml/tinyxml.h"
#include "tinyxml/tinystr.h"
#include <vector>
#include <algorithm>
#include <map>
#include <IL/il.h>
using namespace std;

class Group{
    public:
        Group() {
            id = 0;
        }
        Group(int a) {
            id = a;
        }
        int getId() {
            return id;
        }
        virtual int apply() {
            glPushMatrix();
            return 0;
        }
    private:
        int id;
};

typedef struct node {
    Group* g;
    const char* label;
    vector<struct node*> sons;
} *Tree;

float camX, camY, camZ;
float alfa = 0.0f, beta = 0.0f, radius = 5.0f;
float runX = 0.0f, runY = 0.0f, runZ = 0.0f;
float rotX = 0.0f, rotY = 1.0f, rotZ = 0.0f;
int flag_drawing_mode = 1, idx = 0;
string fileName = "";
Tree t;
float fps = 0;
int timebase = 0, frame = 0;
map<string,vector<string> > modelStorage;
map<string,int> textureStorage;

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

void multMatrixVector(float *m, float *v, float *res) {
	for (int j = 0; j < 4; ++j) {
		res[j] = 0;
		for (int k = 0; k < 4; ++k)
			res[j] += v[k] * m[j * 4 + k];
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
void getGlobalCatmullRomPoint(float gt, float *pos, float *deriv, vector<vector<float> > pc, int npc) {
	const int POINT_COUNT = npc;

	//float p[4][3] = { { -1,0,-1 },{ -1,0,1 },{ 1,0,1 },{ 1,0,-1 } };
	float** p = new float*[POINT_COUNT];
	for (int i = 0; i < POINT_COUNT; i++)
		p[i] = new float[3];

	for (int i = 0; i < npc; i++)
		for (int j = 0; j < 3; j++)
			p[i][j] = pc[i][j];

	// Points that make up the loop for catmull-rom interpolation
	float t = gt * POINT_COUNT; // this is the real global t
	int index = floor(t);  // which segment
	t = t - index; // where within  the segment

	int indices[4]; // indices store the points
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

int loadTexture(string s) {
	unsigned int t, width, height;
	unsigned char *texData;
	unsigned int texID;
    if (textureStorage.find(s) == textureStorage.end()){
        ilInit();
        ilEnable(IL_ORIGIN_SET);
        ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
        ilGenImages(1, &t);
        ilBindImage(t);
        ilLoadImage((ILstring)s.c_str());
        width = ilGetInteger(IL_IMAGE_WIDTH);
        height = ilGetInteger(IL_IMAGE_HEIGHT);
        ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
        texData = ilGetData();

        glGenTextures(1, &texID);

        glBindTexture(GL_TEXTURE_2D, texID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);
        glGenerateMipmap(GL_TEXTURE_2D);

        glBindTexture(GL_TEXTURE_2D, 0);
        textureStorage[s] = texID;
    }
	return textureStorage[s];

}

class Scale : public Group {
    public:
        Scale(float a, float b, float c) {
            x = a;
            y = b;
            z = c;
        }
        Scale() {
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
        Translate(float a, float b, float c) {
            x = a;
            y = b;
            z = c;
        }
        Translate() {
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
        Rotate(float l, float a, float b, float c) {
            angle = l;
            x = a;
            y = b;
            z = c;
        }
        Rotate() {
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

class Color : public Group {
    public:
        Color(float x, float y, float z) {
            r = x;
            g = y;
            b = z;
        }
        Color() {
            r = 0;
            g = 0;
            b = 0;
        }
        int apply() {
            glColor3f(r, g, b);
            return 4;
        }
    private:
        float r, g, b;
};

class Model : public Group {
    public:
        Model(string s, string t) {
		modelo = s;
		texture = t;
	    }
        Model(string s) {
            modelo = s;
        }
        string getModelo() {
            return modelo;
        }
        
        //Motor VBO
        int apply() {
            if (modelStorage.find(modelo) == modelStorage.end()){
                ifstream ficheiro(modelo);
                string s;
                vector<string> lines;
                while(getline(ficheiro,s))
                    lines.push_back(s);
                modelStorage[modelo] = lines;
                ficheiro.close();
            }
            vector<string> currentFile = modelStorage.find(modelo)->second;
            int pp = 0, pn = 0, pt = 0;
            float *position, *textures, *normals;
            int vertexCount;
            bool vertexCountNotRead = true;
            
            // Fill Buffer
            for(auto s: currentFile){
                if (vertexCountNotRead){
                    vertexCount = atoi(s.c_str());
                    vertexCountNotRead = false;
                    position = (float*) calloc(vertexCount*3, sizeof(float));
                    normals = (float*) calloc(vertexCount*3, sizeof(float));
                    textures = (float*) calloc(vertexCount*2, sizeof(float));
                }
                else{
                    istringstream lineSplit(s);
                    float x, y, z, nx, ny, nz, tx, ty;

                    lineSplit >> x;
                    lineSplit >> y;
                    lineSplit >> z;

                    lineSplit >> nx;
                    lineSplit >> ny;
                    lineSplit >> nz;

                    lineSplit >> tx;
                    lineSplit >> ty;

                    position[pp++] = x;
                    position[pp++] = y;
                    position[pp++] = z;
                    
                    normals[pn++] = nx;
                    normals[pn++] = ny;
                    normals[pn++] = nz;

                    textures[pt++] = tx;
                    textures[pt++] = ty;
                }
            }

            // VBO Render
            glGenBuffers(1, &bufferPos);
            glBindBuffer(GL_ARRAY_BUFFER, bufferPos);
            glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertexCount * 3, position, GL_STATIC_DRAW);
            
            glGenBuffers(1, &bufferNor);
            glBindBuffer(GL_ARRAY_BUFFER, bufferNor);
            glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertexCount * 3, normals, GL_STATIC_DRAW);
            
            glGenBuffers(1, &bufferTex);
		    glBindBuffer(GL_ARRAY_BUFFER, bufferTex);
		    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertexCount * 2, textures, GL_STATIC_DRAW);

            tex = loadTexture(texture);
            
            glBindBuffer(GL_ARRAY_BUFFER, bufferPos);
            glVertexPointer(3, GL_FLOAT, 0, 0);
            
            glBindBuffer(GL_ARRAY_BUFFER, bufferNor);
            glNormalPointer(GL_FLOAT, 0, 0);

            glBindTexture(GL_TEXTURE_2D, tex);
            glBindBuffer(GL_ARRAY_BUFFER, bufferTex);
            glTexCoordPointer(2, GL_FLOAT, 0, 0);
            
            glDrawArrays(GL_TRIANGLES, 0, vertexCount);

            glBindTexture(GL_TEXTURE_2D, 0);

            glDeleteBuffers(1,&bufferPos);
            glDeleteBuffers(1,&bufferNor);
            glDeleteBuffers(1,&bufferTex);

            free(position);
            free(normals);
            free(textures);


            return 4;
        }

    public:
        string modelo;
        string texture;
        int nvertices;
        GLuint bufferPos, bufferTex, bufferNor;
        GLuint tex;
};

class Catmull : public Group {
    public:
        Catmull(float t, vector<float> pontos) {
            flag = 1;
            time = t;
            for (int i = 0; i < pontos.size(); i+=3) {
                vector<float> aux;
                aux.push_back(pontos[i]);
                aux.push_back(pontos[i+1]);
                aux.push_back(pontos[i+2]);
                pc.push_back(aux);
            }
            rangle = 0;
        }

        Catmull(float t, float xx, float yy, float zz) {
            xa = xx;
            ya = yy;
            za = zz;
            time = t;
            flag = 0;
            rangle = 0;
        }

        int apply() {
            static float t = 0;
            static float up[4] = { 0, 1, 0, 0 };
            float pos[4], deriv[4];
            float m[4 * 4];
            float y[4], z[4];

            if (flag) { // Time
                getGlobalCatmullRomPoint(t, pos, deriv, pc, pc.size());
                cross(deriv, up, z);
                cross(z, deriv, y);
                normalize(deriv);
                normalize(y);
                normalize(z);
                glTranslatef(pos[0], pos[1], pos[2]);
            }
            
            else // Rotate
                if (fps) {   
                    float angaux = (360 / (fps*time));
                    rangle += angaux;
                    rangle=fmod(rangle, 360);
                    glRotatef(rangle, xa, ya, za);
                }

            up[0] = y[0]; up[1] = y[1]; up[2] = y[2]; up[3] = y[3];

            if (fps)
                t += 1 / (fps*time);

            return 2;
    }

    private:
        float time;
        int flag;
        float xa, ya, za;
        vector<vector<float> > pc;
        float rangle;
};

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

    // Set the projection matrix as current
    glMatrixMode(GL_PROJECTION);
    // Load Identity Matrix
    glLoadIdentity();

    // Set the viewport to be the entire window
    glViewport(0, 0, w, h);

    // Set perspective
    gluPerspective(45.0f, ratio, 1.0f, 1000.0f);

    // return to the model view matrix mode
    glMatrixMode(GL_MODELVIEW);
}

void processKeys(unsigned char key, int xx, int yy) {
    switch (key) {
        //Translate:
        case 'D' | 'd':
            runX += 0.1;
            break;
        case 'A' | 'a':
            runX -= 0.1;
            break;
        case 'W' | 'w':
            runY += 0.1;
            break;
        case 'S' |'s':
            runY -= 0.1;
            break;
        case 'Q' | 'q':
            runZ += 0.1;
            break;
        case 'E' | 'e':
            runZ -= 0.1;
            break;
        
        //Rotate:
        case 'U' | 'u':
            rotX += 0.1;
            break;
        case 'J' | 'j':
            rotX -= 0.1;
            break;
        case 'H' | 'h':
            rotY += 0.1;
            break;
        case 'K' | 'k':
            rotY -= 0.1;
            break;
        case 'O' | 'o':
            rotZ += 0.1;
            break;
        case 'L' | 'l':
            rotZ -= 0.1;
            break;

        //Zoom:
        case '+':
            radius -= 0.1f;
            if (radius < 0.1f)
                radius = 0.1f;
            break;
        case '-': radius += 0.1f;
                  break;

        // Camara Inicial:
        case 'R' | 'r':
            runX = 0.0f, runY = 0.0f, runZ = 0.0f;
            rotX = 0.0f, rotY = 1.0f, rotZ = 0.0f;
            alfa = 0.0f, beta = 0.0f, radius = 5.0f;
            camX = radius * cos(beta) * sin(alfa);
            camY = radius * sin(beta);
            camZ = radius * cos(beta) * cos(alfa);
            break;

        //alteraçao de cor completa, traços e pontos:
        case '1': flag_drawing_mode = 0;
                  break;
        case '2': flag_drawing_mode = 1;
                  break;
        case '3': flag_drawing_mode = 2;
                  break;
    }
    spherical2Cartesian();
    glutPostRedisplay();
}

void processSpecialKeys(int key, int xx, int yy) {
    switch (key) {
        case GLUT_KEY_RIGHT:
            alfa -= 0.1; break;

        case GLUT_KEY_LEFT:
            alfa += 0.1; break;

        case GLUT_KEY_UP:
            beta -= 0.1f;
            if (beta > 1.5)
                beta = 1.5f;
            break;

        case GLUT_KEY_DOWN:
            beta += 0.1f;
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

void processMenuEvents(int option) {
    switch (option) {
        case 0 :
            flag_drawing_mode = 0;
            break;
        case 1 :
            flag_drawing_mode = 1;
            break;
        case 2 :
            flag_drawing_mode = 2;
            break;
        default:
            break;
    }
    glutPostRedisplay();
}

void createGLUTMenus() {
    int menu;
    menu = glutCreateMenu(processMenuEvents);

    glutAddMenuEntry("Fill",0);
    glutAddMenuEntry("Line",1);
    glutAddMenuEntry("Point",2);

    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

int parserXML(char* ficheiro) {
    string fich_xml = (string)ficheiro;
    int num = 0;
    int cap = 0;

    //Ler XML
    TiXmlDocument doc;

    if (!doc.LoadFile(fich_xml.c_str())) {
        cout << "Error loading XML file." << endl;
        return 1;
    }

    TiXmlNode* base = doc.FirstChild();

    if (strcmp(base->Value(), "scene") != 0) {
        cout << "Wrong XML scene format." << endl;
        return 1;
    }

    TiXmlElement* elementos = base->FirstChildElement("group");
    TiXmlElement* modelos;

    if (elementos == NULL) {
        cout << "base -> XML file has no elements!" << endl;
        return 0;
    }

    t = new struct node;
    Tree taux = new struct node;
    taux = t;
    taux->g = new Group(idx++);
    taux->label = "group";
    vector<TiXmlElement*> stackgroups; // Apontadores de elementos para percorrer a arvore de hierarquias
    vector<struct node *>stack_nodes_group; // Nodes para adicionar as leafs

    elementos = elementos->FirstChildElement();
    while (elementos != NULL) {
        if (strcmp(elementos->Value(), "group") == 0) {
            stackgroups.push_back(elementos->FirstChildElement());
            Tree aux = new struct node;
            aux->g = new Group(idx++);
            aux->label = "group";
            aux->sons.clear();
            taux->sons.push_back(aux);
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
                Tree aux = new struct node;
                aux->g = new Scale(x,y,z);
                aux->label = "scale";
                aux->sons.clear();
                taux->sons.push_back(aux);
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

					Tree aux = new struct node;
					aux->g = new Catmull(ttime, pcontrolo);
					aux->label = "catmull";
					aux->sons.clear();
					taux->sons.push_back(aux);
				}
				else {
					Tree aux = new struct node;
					aux->g = new Translate(x, y, z);
					aux->label = "translate";
					aux->sons.clear();
					taux->sons.push_back(aux);
				}
            }
        }

        if (strcmp(elementos->Value(), "rotate") == 0) {
            float x, y, z, angle, ttime;
            x = y = z = angle = 0.0;

            if (elementos->Attribute("X")) {
                x = atof(elementos->Attribute("X"));
            }
            if (elementos->Attribute("Y")) {
                y = atof(elementos->Attribute("Y"));
            }
            if (elementos->Attribute("Z")) {
                z = atof(elementos->Attribute("Z"));
            }
            if (elementos->Attribute("angle")) {
                angle = atof(elementos->Attribute("angle"));
            }

            if (elementos->Attribute("time")) {
				ttime = atof(elementos->Attribute("time"));
				Tree aux = new struct node;
				aux->g = new Catmull(ttime, x,y,z);
				aux->label = "catmull";
				aux->sons.clear();
				taux->sons.push_back(aux);
			}
			else {
				Tree aux = new struct node;
				aux->g = new Rotate(angle, x, y, z);
				aux->label = "rotate";
				aux->sons.clear();
				taux->sons.push_back(aux);
			}
        }

        if (strcmp(elementos->Value(), "color") == 0) {
            float x, y, z;
            x = y = z = 0.0;

            if (elementos->Attribute("R")) {
                x = atof(elementos->Attribute("R"));
            }
            if (elementos->Attribute("G")) {
                y = atof(elementos->Attribute("G"));
            }
            if (elementos->Attribute("B")) {
                z = atof(elementos->Attribute("B"));
            }

            Tree aux = new struct node;
            aux->g = new Color(x, y, z);
            aux->label = "color";
            aux->sons.clear();
            taux->sons.push_back(aux);
        }

        if (strcmp(elementos->Value(), "models") == 0) {
            modelos = elementos;
            modelos = modelos->FirstChildElement("model");

            while (modelos != NULL) {
                const char *nome = modelos->Attribute("file");
                FILE *test;
                if ((test = fopen(nome, "r")) != 0) {
                    cout << "Model \'" << nome << "\' loaded." << endl;
                }

                Tree aux = new struct node;
                if (modelos->Attribute("texture"))
                    aux->g = new Model((char*)nome, (char*)modelos->Attribute("texture"));
				else
					aux->g = new Model((char*)nome);
                aux->label = "model";
                aux->sons.clear();
                taux->sons.push_back(aux);
                modelos = modelos->NextSiblingElement();
            }
        }

        elementos = elementos->NextSiblingElement();
        if (elementos == NULL) {
            if (cap!=0 ) {
                elementos = stackgroups[num];
                taux = stack_nodes_group[num];
                num++;
                cap--;
            }
        }
    }
    return 0;
}

void dfs(struct node *root, struct node* leafs) {
    if (root != NULL) {
        if (strcmp(root->label, "group") == 0 && leafs==NULL) {
            glPopMatrix();
        }
    }
    if (leafs != NULL) {
        (leafs->g)->apply();
        int leafSize = leafs->sons.size();
        for (int i = 0; i <= leafSize; i++) {
            if (i<leafSize)
                dfs(leafs, leafs->sons[i]);
            else
                dfs(leafs, NULL);
        }
    }
}

void draw_models() {
    //Calls DFS to travel trough the tree and draw the models
    dfs(NULL, t);
    glPopMatrix();
}

void renderScene() {
    // clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // set the camera
    glLoadIdentity();
    gluLookAt(camX, camY, camZ,
              runX, runY, runZ,
              rotX, rotY, rotZ);

    // render mode
    if(flag_drawing_mode == 0){
        glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
    }else if(flag_drawing_mode == 1){
        glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
    }else if(flag_drawing_mode == 2){
        glPolygonMode(GL_FRONT_AND_BACK,GL_POINT);
    }

    //Time Rotation
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

    // Load Engine
    draw_models();

    // So the solar system is dynamic
    glutPostRedisplay();

    // End of frame
    glutSwapBuffers();
}

int main(int argc, char **argv) {
    if (argc < 2) {
        cout << "Main: Lack of arguments" << endl;
        exit(0);
    }
    parserXML(argv[1]);
    cout << endl << "CAMERA INFO:" << endl << endl;
    cout << "Turn up, down, left or right: Arrow Keys" << endl;
    cout << "Move on the X axis: A | D  -  Move on the Y axis: W | S  -  Move on the Z axis: Q | E" << endl;
    cout << "Rotation: U - J - H - K - O - L" << endl ;
    cout << "Zoom out: - | Zoom in: +" << endl;
    cout << "Reset camera to initial position: R" << endl;
    cout << "NOTE: Command keys are not case-sensitive." << endl;

    // init GLUT and the window
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(800, 800);
    glutCreateWindow("MOTOR");

    // Required callback registry
    glutDisplayFunc(renderScene);
    glutReshapeFunc(changeSize);

    // Callback registration for keyboard processing
    glutKeyboardFunc(processKeys);
    glutSpecialFunc(processSpecialKeys);

    // Menus
    glutDetachMenu(GLUT_RIGHT_BUTTON);
    createGLUTMenus();

    // OpenGL settings
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);

    glutPostRedisplay();
    spherical2Cartesian();

    // VBO, Normal, Tex
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);



	glColor3f(1.0f, 1.0f, 1.0f);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    // For Linux
    #ifndef __APPLE__
        glewInit();
    #endif
    
    // enter GLUT's main cycle
    glutMainLoop();

    return 0;
}
