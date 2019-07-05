// generator.cpp : Generates files with coordenates to be rendered by the engine.

#define _USE_MATH_DEFINES
#include <math.h>
#include <fstream>
#include <iostream>
#include <string.h>
#include <sstream>
#include <algorithm>
using namespace std;

// Output File
fstream outputFile; 

// Plane
void plane(float x, float z){
    float xa = x / 2;
    float za = z / 2;
    float ya = 0;

    //Number of triangles to be drawn
    outputFile << "2" << endl;

    //Write on output  outputFile the 3 vectors for the 1st triangle to be drawn
    outputFile << xa << " " << ya << " " << za << endl;
    outputFile << xa << " " << ya << " " << -za << endl;
    outputFile << -xa << " " << ya << " " << za << endl;

    //Write on output  outputFile the 3 vectors for the 2nd triangle to be drawn
    outputFile << -xa << " " << ya << " " << za << endl;
    outputFile << xa << " " << ya << " " << -za << endl;
    outputFile << -xa << " " << ya << " " << -za << endl;
}

// Box
void box(float x, float z, float y, int divisions) {
    if ((divisions = abs(divisions)) == 0) divisions = 1;

    //Calculate number of triangles
    outputFile << 6*2*pow(divisions,2) << endl;

    float xaux = -x / 2;
    float zaux = z / 2;
    float yaux = y / 2;
    float za = z / 2;

    // Front Face
    for (int j = 0; j < divisions; j++) {
        for (int i = 0; i < divisions; i++) {

            //Triangle 1
            outputFile << xaux << " " << yaux << " " << za << endl;
            outputFile << xaux << " " << yaux - (y / divisions) << " " << za << endl;
            outputFile << xaux + (x / divisions) << " " << yaux - (y / divisions) << " " << za << endl;

            // Triangle 2
            outputFile << xaux << " " << yaux << " " << za << endl;
            outputFile << xaux + (x / divisions) << " " << yaux - (y / divisions) << " " << za << endl;
            outputFile << xaux + (x / divisions) << " " << yaux << " " << za << endl;

            xaux += (x / divisions);
        }
        xaux = -x / 2;
        yaux = yaux - (y / divisions);
    }

    yaux = y / 2;
    xaux = -x / 2;
    zaux = z / 2;

    // Back Face
    for (int j = 0; j < divisions; j++) {
        for (int i = 0; i < divisions; i++) {
            // Triangle
            outputFile << xaux + (x / divisions) << " " << yaux << " " << -za << endl;
            outputFile << xaux + (x / divisions) << " " << yaux - (y / divisions) << " " << -za << endl;
            outputFile << xaux << " " << yaux - (y / divisions) << " " << -za << endl;

            // Triangle
            outputFile << xaux << " " << yaux - (y / divisions) << " " << -za << endl;
            outputFile << xaux << " " << yaux << " " << -za << endl;
            outputFile << xaux + (x / divisions) << " " << yaux << " " << -za << endl;

            xaux += (x / divisions);
        }
        xaux = -x / 2;
        yaux = yaux - (y / divisions);
    }

    xaux = x / 2;
    zaux = z / 2;
    yaux = y / 2;

    // Right Side Face
    for (int j = 0; j < divisions; j++) {
        for (int i = 0; i < divisions; i++) {
            // Triangle
            outputFile << xaux << " " << yaux << " " << zaux << endl;
            outputFile << xaux << " " << yaux - (y / divisions) << " " << zaux << endl;
            outputFile << xaux << " " << yaux - (y / divisions) << " " << zaux - (z / divisions) << endl;

            // Triangle
            outputFile << xaux << " " << yaux << " " << zaux << endl;
            outputFile << xaux << " " << yaux - (y / divisions) << " " << zaux - (z / divisions) << endl;
            outputFile << xaux << " " << yaux << " " << zaux - (z / divisions) << endl;

            zaux -= (z / divisions);
        }

        zaux = z / 2;
        yaux = yaux - (y / divisions);
    }

    xaux = -x / 2;
    zaux = z / 2;
    yaux = y / 2;

    // Left Side Face
    for (int j = 0; j < divisions; j++) {
        for (int i = 0; i < divisions; i++) {
            // Triangle
            outputFile << xaux << " " << yaux << " " << zaux - (z / divisions) << endl;
            outputFile << xaux << " " << yaux - (y / divisions) << " " << zaux << endl;
            outputFile << xaux << " " << yaux << " " << zaux << endl;

            // Triangle
            outputFile << xaux << " " << yaux << " " << zaux - (z / divisions) << endl;
            outputFile << xaux << " " << yaux - (y / divisions) << " " << zaux - (z / divisions) << endl;
            outputFile << xaux << " " << yaux - (y / divisions) << " " << zaux << endl;

            zaux -= (z / divisions);
        }

        zaux = z / 2;
        yaux = yaux - (y / divisions);
    }

    xaux = -x / 2;
    zaux = -z / 2;
    yaux = y / 2;

    // Top Face
    for (int j = 0; j < divisions; j++) {
        for (int i = 0; i < divisions; i++) {
            // Triangle
            outputFile << xaux << " " << yaux << " " << zaux << endl;
            outputFile << xaux << " " << yaux << " " << zaux + (z / divisions) << endl;
            outputFile << xaux + (x / divisions) << " " << yaux << " " << zaux + (z / divisions) << endl;

            // Triangle
            outputFile << xaux << " " << yaux << " " << zaux << endl;
            outputFile << xaux + (x / divisions) << " " << yaux << " " << zaux + (z / divisions) << endl;
            outputFile << xaux + (x / divisions) << " " << yaux << " " << zaux << endl;

            xaux += (x / divisions);
        }

        xaux = -x / 2;
        zaux = zaux + (y / divisions);
    }

    xaux = -x / 2;
    zaux = -z / 2;
    yaux = -y / 2;

    // Bottom Face
    for (int j = 0; j < divisions; j++) {
        for (int i = 0; i < divisions; i++) {
            // Triangle
            outputFile << xaux << " " << yaux << " " << zaux << endl;
            outputFile << xaux + (x / divisions) << " " << yaux << " " << zaux + (z / divisions) << endl;
            outputFile << xaux << " " << yaux << " " << zaux + (z / divisions) << endl;

            // Triangle
            outputFile << xaux << " " << yaux << " " << zaux << endl;
            outputFile << xaux + (x / divisions) << " " << yaux << " " << zaux << endl;
            outputFile << xaux + (x / divisions) << " " << yaux << " " << zaux + (z / divisions) << endl;

            xaux += (x / divisions);
        }

        xaux = -x / 2;
        zaux = zaux + (y / divisions);
    }
}

// Cone
void cone(float radius, float height, int slices, int stacks) {

    // Number of triangles to be drawn
    outputFile << (6*slices)+(6*slices*(stacks-1)) << endl;

    float angulo = (2 * M_PI) / slices;
    float baseaux = -height / 2;
    float raioaux = radius;
    float raioaux2 = radius - (radius / stacks);

    // Draw the base of the cone
    for (int i = 0; i < slices; i++) {
        outputFile << radius*sin(angulo*i) << " " << baseaux << " " << radius*cos(angulo*i) << endl;
        outputFile << 0.0f << " " << baseaux << " " << 0.0f << endl;
        outputFile << radius*sin(angulo*(i + 1)) << " " << baseaux << " " << radius*cos(angulo*(i + 1)) << endl;
    }

    // Draw the stacks of the cone
    for (int j = 0; j < stacks; j++) {
        
        for (int i = 0; i < slices; i++) {
            outputFile << raioaux*(sin(angulo*i)) << " " << baseaux << " " << raioaux*(cos(angulo*i))<< endl;
            outputFile << raioaux2*(sin(angulo*(i + 1))) << " " << baseaux + (height / stacks) << " " << raioaux2*(cos(angulo*(i + 1))) << endl;
            outputFile << raioaux2*(sin(angulo*i)) << " " << baseaux + (height / stacks) << " " << raioaux2*(cos(angulo*i)) << endl;
            outputFile << raioaux*(sin(angulo*i)) << " " << baseaux << " " << raioaux*(cos(angulo*i)) << endl;
            outputFile << raioaux*(sin(angulo*(i + 1))) << " " << baseaux << " " << raioaux*(cos(angulo*(i + 1))) << endl;
            outputFile << raioaux2*(sin(angulo*(i + 1))) << " " << baseaux + (height / stacks) << " " << raioaux2*(cos(angulo*(i + 1))) << endl;
        }
        baseaux += height / stacks;
        raioaux = raioaux2;
        raioaux2 -= (radius / stacks);
    }
}

// Sphere
void sphere(float radius, int slices, int stacks){
	int vertex_count = 6*(slices+(stacks-2)*slices);

    outputFile << vertex_count << endl;

    for(int j = 0; j<slices; j++){
        // Up Face
        int i = stacks-1;
        float alpha = 2*M_PI/slices*j;
        float beta = M_PI/stacks*i-M_PI/2;
        float alpha_aux = 2*M_PI/slices*(j+1);
        float beta_aux = M_PI/stacks*(i+1)-M_PI/2;
        float x = (float) i;
        float y = (float) j;

        outputFile << radius*cos(beta_aux)*cos(alpha) << " " << radius*sin(beta_aux) << " " << radius*cos(beta_aux)*sin(alpha) << " " << cos(beta_aux)*cos(alpha) << " " << sin(beta_aux) << " " << cos(beta_aux)*sin(alpha) << " " << y/slices << " " << 1.0 << endl;
        outputFile << radius*cos(beta)*cos(alpha_aux) << " " << radius*sin(beta) << " " << radius*cos(beta)*sin(alpha_aux) << " " << cos(beta)*cos(alpha_aux) << " " << sin(beta) << " " << cos(beta)*sin(alpha_aux) << " " << y/slices << " " << x/stacks << endl;
        outputFile << radius*cos(beta)*cos(alpha) << " " << radius*sin(beta) << " " << radius*cos(beta)*sin(alpha) << " " << cos(beta)*cos(alpha) << " " << sin(beta) << " " << cos(beta)*sin(alpha) << " " << y/slices << " " << x/stacks << endl;

        // Down Face
        i = 0;
        x = (float) i;
        alpha = 2*M_PI/slices*j;
        beta = M_PI/stacks*i-M_PI/2;
        alpha_aux = 2*M_PI/slices*(j+1);
        beta_aux = M_PI/stacks*(i+1)-M_PI/2;

        outputFile << radius*cos(beta_aux)*cos(alpha) << " " << radius*sin(beta_aux) << " " << radius*cos(beta_aux)*sin(alpha) << " " << cos(beta_aux)*cos(alpha) << " " << sin(beta_aux) << " " << cos(beta_aux)*sin(alpha) << " " << y/slices << " " << 1.0/stacks << endl;
        outputFile << radius*cos(beta_aux)*cos(alpha_aux) << " " << radius*sin(beta_aux) << " " << radius*cos(beta_aux)*sin(alpha_aux) << " " << cos(beta_aux)*cos(alpha_aux) << " " << sin(beta_aux) << " " << cos(beta_aux)*sin(alpha_aux) << " " << y/slices << " " << 1.0/stacks << endl;
        outputFile << radius*cos(beta)*cos(alpha_aux) << " " << radius*sin(beta) << " " << radius*cos(beta)*sin(alpha_aux) << " " << cos(beta)*cos(alpha_aux) << " " << sin(beta) << " " << cos(beta)*sin(alpha_aux) << " " << y/slices << " " << 0.0 << endl;
    }

    for(int i = 1; i<stacks-1; i++){
        for(int j = 0; j<slices; j++){
            float alpha = 2*M_PI/slices*j;
            float beta = M_PI/stacks*i-M_PI/2;

            float alpha_aux = 2*M_PI/slices*(j+1);
            float beta_aux = M_PI/stacks*(i+1)-M_PI/2;

            float x = (float) i;
            float y = (float) j;

            //caso base
            outputFile << radius*cos(beta_aux)*cos(alpha) << " " << radius*sin(beta_aux) << " " << radius*cos(beta_aux)*sin(alpha) << " " << cos(beta_aux)*cos(alpha) << " " << sin(beta_aux) << " " << cos(beta_aux)*sin(alpha) << " " << y/slices << " " << (x+1.0)/stacks << endl;
            outputFile << radius*cos(beta_aux)*cos(alpha_aux) << " " << radius*sin(beta_aux) << " " << radius*cos(beta_aux)*sin(alpha_aux) << " " << cos(beta_aux)*cos(alpha_aux) << " " << sin(beta_aux) << " " << cos(beta_aux)*sin(alpha_aux) << " " << (y+1.0)/slices << " " << (x+1.0)/stacks << endl;
            outputFile << radius*cos(beta)*cos(alpha_aux) << " " << radius*sin(beta) << " " << radius*cos(beta)*sin(alpha_aux) << " " << cos(beta)*cos(alpha_aux) << " " << sin(beta) << " " << cos(beta)*sin(alpha_aux) << " " << (y+1.0)/slices << " " << x/stacks << endl;
            outputFile << radius*cos(beta_aux)*cos(alpha) << " " << radius*sin(beta_aux) << " " << radius*cos(beta_aux)*sin(alpha) << " " << cos(beta_aux)*cos(alpha) << " " << sin(beta_aux) << " " << cos(beta_aux)*sin(alpha) << " " << y/slices << " " << (x+1.0)/stacks << endl;
            outputFile << radius*cos(beta)*cos(alpha_aux) << " " << radius*sin(beta) << " " << radius*cos(beta)*sin(alpha_aux) << " " << cos(beta)*cos(alpha_aux) << " " << sin(beta) << " " << cos(beta)*sin(alpha_aux) << " " << (y+1.0)/slices << " " << x/stacks << endl;
            outputFile << radius*cos(beta)*cos(alpha) << " " << radius*sin(beta) << " " << radius*cos(beta)*sin(alpha) << " " << cos(beta)*cos(alpha) << " " << sin(beta) << " " << cos(beta)*sin(alpha) << " " << y/slices << " " << x/slices << endl;
        }
    }
}

// Generator
void generator(char* function, char** argv) {
        //string path = "../"; // For use in case we're using CLion
        string path = ""; // For use in case we're compiling through the bash

        if (strcmp(function, "plane") == 0) {
            outputFile.open(path + argv[4], std::fstream::out);
            plane(atof(argv[2]), atof(argv[3]));
        }

        if (strcmp(function, "box") == 0) {
            outputFile.open(path + argv[6], std::fstream::out);
            box(atof(argv[2]), atof(argv[3]), atof(argv[4]), atoi(argv[5]));
        }

        if (strcmp(function, "cone") == 0) {
            outputFile.open(path + argv[6], std::fstream::out);
            cone(atof(argv[2]), atof(argv[3]), atof(argv[4]), atoi(argv[5]));
        }

        if (strcmp(function, "sphere") == 0) {
            outputFile.open(path + argv[5], std::fstream::out);
            sphere(atof(argv[2]), atof(argv[3]), atof(argv[4]));
        }

        outputFile.close();
}

// Main
int main(int argc, char** argv)  {

    if (argc < 2) {
        cout << "Main: Lack of arguments" << endl;
        exit(0);
    }

    else if (strcmp(argv[1], "plane") == 0) {
        if (argc != 5){
            cout << "[Shape **" << argv[1] << "**]: Invalid Arguments!" << endl;
            exit(0);
        }
        else
            generator(argv[1], argv);
    }

    else if (strcmp(argv[1], "box") == 0) {
        if (argc != 7){
            cout << "[Shape **" << argv[1] << "**]: Invalid Arguments!" << endl;
            exit(0);
        }
        else
            generator(argv[1], argv);
    }

    else if (strcmp(argv[1], "cone") == 0) {
        if (argc != 7){
            cout << "[Shape **" << argv[1] << "**]: Invalid Arguments!" << endl;
            exit(0);
        }
        else
            generator(argv[1], argv);
    }

    else if (strcmp(argv[1], "sphere") == 0) {
        if (argc != 6){
            cout << "[Shape **" << argv[1] << "**]: Invalid Arguments!" << endl;
            exit(0);
        }
        else
            generator(argv[1], argv);
    }

    else {
        cout << "Shape not found!" << endl;
        return -1;
    }

    return 0;
}