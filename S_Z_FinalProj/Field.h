/*
Author: Sikai Zhao
Class: ECE 6122
Last date modified: 11/25/2019
Description:
 1. Define the field class, providing the functions to draw football field with texture, UAVs;
 2. Provide functions to do tasks such as check collision, process data.
*/

#ifndef Field_h
#define Field_h

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glut.h>
#endif

#include <array>
#include "Bitmap.h"
#include "UAVs.h"

#define checkImageWidth 64              // CheckImageWidth set as 64
#define checkImageHeight 64             // CheckImageHeight set as 64

struct Image {                          // Define the struct of image to store image
    unsigned long sizeX;
    unsigned long sizeY;
    char *data;
};
typedef struct Image Image;

class Field                             // Define the class of Field
{
public:
    // The variables related to use imported .bmp file as texture
    GLubyte checkImage[checkImageWidth][checkImageHeight][3];  // Set checkImage as GLubyte to process the imported .bmp image;
    BMP inBitmap;           // BMP object imBitmap
    GLuint texture[2];      // GLuint texture to store the texture
    
    static constexpr double fieldLength = 100 * 0.9144;     // The length of the football field(m)
    static constexpr double fieldWidth = (53+1/3) * 0.9144; // The width of the football field(m)
    location origin;        // The origin of the field
    location target;        // The target to fly at first
    std::array<UAVs, 15> uavStatus; // An array to store 15 UAVs as objects of UAVs class
    
public:
    Field();                // Construtor to set the origin, target and initiate the UAVs
    void makeCheckImage();  // Function to check the image
    void textureInit();     // Initiate the texture
    
    void drawField();       // Function to draw the football field
    void drawSphere();      // Function to draw a sphere to visualize the sphere UAVs fly around
    void drawUavs(int timeSlice);   // Function to draw UAVs with color oscilation
    
    void getParametersMpi(double transferData[],int uavNum); // Get parameters of specific UAV to send buffer
    void setParametersMpi(double transferData[],int numElements);  // Store parameters of specific UAV according to receive buffer
    
    void collisionDealing();    // Check and deal with collisions
    double distanceBetweenUavs(const UAVs &uav1, const UAVs &uav2); // Calculate the distances
};

#endif 
