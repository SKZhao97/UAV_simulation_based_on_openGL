/*
Author: Sikai Zhao
Class: ECE 6122
Last date modified: 11/30/2019
Description:
 1. Implement the class of Field
 2. Implement the functions to draw field, sphere and UAVs, and functions related to MPI data
 3. Store the location information of UAVs and check collisions
*/

#include <iostream>
#include <stdio.h>
#include <cmath>
#include <stdlib.h>
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glut.h>
#endif

#include "Field.h"
#include "Bitmap.h"
#include "UAVs.h"


/*
 * Function name: Field()
 * Summary: Constructor of Field class, setting the origin of the field and the initial the locations of the UAVs.
 * Input: N/A
 * Output: N/A
 */
Field::Field()                              // Constructor
{
    origin.x = 0;
    origin.y = 0;
    origin.z = 0;                           // Set the origin
    target.x = origin.x;
    target.y = origin.y;
    target.z = 50;                          // Set the target
    
    int row = 0;
    int column = 0;                         // The colomn of UAVs
    for(auto &uav : uavStatus)              // Loop the 15 UAVs to initiate the loacations
    {
        uav.uavLocation.y = 25 * column * 0.9144 - fieldLength/2; // Initial the location of UAVs
        uav.uavLocation.x = row * fieldWidth/2 - fieldWidth/2;    // Initial the
        row++;
        if (row == 3)                       // If row reach 3, then
        {
            row = 0;
            column++;                       // Transfer to next column
        }
    }
}

/*
* Function name: makeCheckImage
* Summary: Generate the check image and store it
* Input: N/A
* Output: N/A
*/
void Field::makeCheckImage() {
    int i, j, c;
    for (i = 0; i < checkImageWidth; i++) {
        for (j = 0; j < checkImageHeight; j++) {
            c = ((((i & 0x8) == 0) ^ ((j & 0x8) == 0))) * 255;
            checkImage[i][j][0] = (GLubyte)c;
            checkImage[i][j][1] = (GLubyte)c;
            checkImage[i][j][2] = (GLubyte)c;       // Store the information in checkImage
        }
    }
}

/*
* Function name: textureInit
* Summary: Initiate the texture to draw the football field
* Input: N/A
* Output: N/A
*/
void Field::textureInit()
{
    glClearColor(0.5, 0.5, 0.5, 0.0);        // Set clear color
    glEnable(GL_DEPTH_TEST);                 // Enable deep test
    glDepthFunc(GL_LESS);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    inBitmap.read("ff.bmp");                 // Import ff.bmp
    makeCheckImage();                        // Make changefile
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);   // Create Textures
    glGenTextures(2, texture);               // Setup first texture
    glBindTexture(GL_TEXTURE_2D, texture[0]);// Bind texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //scale linearly when image bigger than texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //scale linearly when image smalled than texture
    glTexImage2D(GL_TEXTURE_2D, 0, 3, inBitmap.bmp_info_header.width, inBitmap.bmp_info_header.height, 0,
        GL_BGR_EXT, GL_UNSIGNED_BYTE, &inBitmap.data[0]);   // Initiate
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    glEnable(GL_TEXTURE_2D);                // Enable texture
}

/*
* Function name: drawField
* Summary: Draw the football field with .bmp texture
* Input: N/A
* Output: N/A
*/
void Field::drawField()
{
    glPushMatrix();             // Push the position
    textureInit();              // Initialte th texture
    glEnable(GL_TEXTURE_2D);    // Enable texture
    glColor3ub(0, 255, 0);      // Set color
    glBindTexture(GL_TEXTURE_2D, texture[0]);   //  Bind texture
    glBegin(GL_QUADS);          // Start drawing quads
        glTexCoord2f(1, 1);
        glVertex3f(fieldWidth/2, fieldLength/2 + 10*0.9144, 0.0f); // Add 10 yards to each side
        //glVertex3f(fieldWidth, (fieldLength + 10 - 50)*0.9144, 0.0f);
        glTexCoord2f(0, 1);
        glVertex3f(-fieldWidth/2, fieldLength/2 + 10*0.9144, 0.0f);
        //glVertex3f(0, fieldLength + 10 - 50)*0.9144, 0.0f);
        glTexCoord2f(0, 0);
        //glVertex3f(0, -10*0.9144, 0.0f);
        glVertex3f(-fieldWidth/2, -10*0.9144-fieldLength/2, 0.0f);
        glTexCoord2f(1, 0);
        //glVertex3f(fieldWidth, -10*0.9144, 0.0f);
        glVertex3f(fieldWidth/2, -10*0.9144-fieldLength/2, 0.0f);  // Set the four vertex
    glEnd();                    // End drawing
    glPopMatrix();              // Pop the position
    glDisable(GL_TEXTURE_2D);   // Disable Texture
}

/*
* Function name: drawSphere
* Summary: Draw the sphere with center of (0,0,50)
* Input: N/A
* Output: N/A
*/
void Field::drawSphere()
{
    glPushMatrix();
        glColor3f(1.0,0.8,0.0);
        glTranslatef(target.x, target.y, target.z);
        glutWireSphere(10, 10, 20);
    glPopMatrix();
}

/*
* Function name: drawUAVs
* Summary: Draw the UAVs at specific position
* Input: int timeSlice: current time slice
* Output: N/A
*/
void Field::drawUavs(int timeSlice)
{
    for(auto &uav : uavStatus)     // Loop the 15 UAVs to draw them at the initiated location
    {
        uav.drawUav(timeSlice);    // Call drawUav to draw UAV with color based on the timeSlice
    }
}

/*
* Function name: getParameters
* Summary: Store the data needed in the send buffer for MPI_Allgather()
* Input:
    1.transferData[]:the array to store sending data
    2.int uavNum:the number of data
* Output: N/A
*/
void Field::getParametersMpi(double transferData[],int uavNum)
{
    transferData[0] = uavStatus[uavNum].uavLocation.x;
    transferData[1] = uavStatus[uavNum].uavLocation.y;
    transferData[2] = uavStatus[uavNum].uavLocation.z;          // Store location
    transferData[3] = uavStatus[uavNum].uavParameter.xVel;
    transferData[4] = uavStatus[uavNum].uavParameter.yVel;
    transferData[5] = uavStatus[uavNum].uavParameter.zVel;      // Store velosity
}

/*
* Function name: setParameters
* Summary: Get the needed parameters from receive buffer and set them to the UAV object
* Input:
    1.transferData[]:the array to store sending data
    2.int uavNum:the number of data
* Output: N/A
*/
void Field::setParametersMpi(double transferData[], int numElements)
{
    int uavNum = 1;
    for(auto &uav : uavStatus)
    {
        uav.uavLocation.x = transferData[uavNum * numElements+0];
        uav.uavLocation.y = transferData[uavNum * numElements+1];
        uav.uavLocation.z = transferData[uavNum * numElements+2];
        uav.uavParameter.xVel = transferData[uavNum * numElements+3];
        uav.uavParameter.yVel = transferData[uavNum * numElements+4];
        uav.uavParameter.zVel = transferData[uavNum * numElements+5];
        uavNum++;
    }
}

/*
* Function name: distanceBetweenUavs
* Summary: Calculate distance between two UAVs
* Input:
* 1. const UAVs &uav1: the first UAV to check
* 2. const UAVs &uav2: the second UAV to check
* Output: double distance: the distance between two checked UAVs
*/
double Field::distanceBetweenUavs(const UAVs &uav1, const UAVs &uav2)
{
    double distance = sqrt(pow(uav2.uavLocation.x - uav1.uavLocation.x, 2) + pow(uav2.uavLocation.y - uav1.uavLocation.y, 2) + pow(uav2.uavLocation.z - uav1.uavLocation.z, 2));    // Calculate distance
    return distance;
}

/*
* Function name: collisionDealing
* Summary: Check collisions between UAVs and deal with the collisions
* Input: N/A
* Output: N/A
*/
void Field::collisionDealing()
{
    for(int i = 0; i<uavStatus.size(); i++)
    {
        for(int j=i; j < uavStatus.size(); j++)
        {
            // If another UAV comes within .01 of it then swap velocity vectors
            if(distanceBetweenUavs(uavStatus[i], uavStatus[j]) < 1.01)
            {
                double temp = uavStatus[i].uavParameter.xVel;
                uavStatus[i].uavParameter.xVel = uavStatus[j].uavParameter.xVel;
                uavStatus[j].uavParameter.xVel = temp;          // Exchange the velocity
                temp = uavStatus[i].uavParameter.yVel;
                uavStatus[i].uavParameter.yVel = uavStatus[j].uavParameter.yVel;
                uavStatus[j].uavParameter.yVel = temp;          // Exchange the velocity
                temp = uavStatus[i].uavParameter.zVel;
                uavStatus[i].uavParameter.zVel = uavStatus[j].uavParameter.zVel;
                uavStatus[j].uavParameter.zVel = temp;          // Exchange the velocity
            }
        }
    }
}
