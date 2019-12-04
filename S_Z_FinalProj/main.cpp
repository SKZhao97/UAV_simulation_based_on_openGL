/*
Author: Sikai Zhao
Class: ECE 6122
Last date modified: 11/30/2019
Description:
 1. Initiate and implement the MPI oeprations, in the main process(rank = 0) the OpenGL callbacks are called to draw the football field and UAVs, in other processes the UAVs' parameters are calculate and updated;
 2. Define and impelment the OpenGL callback functions
 3. Main entry of functions
*/

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <chrono>
#include <thread>
#include <cstdlib>

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glut.h>
#endif

#include "iomanip"
#include "mpi.h"                // To use mpi
#include "Bitmap.h"             // To import .bmp file
#include "Field.h"              // To draw the field
#include "UAVs.h"               // To draw the UAVs

const int numElements = 6;      // Thre number of elements, x, y, z, vx, vy, vz
const int rcvSize = 16 * 6;     // (Main task + 15 UAVs) * numElements
double* rcvbuffer = new double[rcvSize];    // Receiver buffer for MPI_Allgather
double sendBuffer[numElements]; // Send buffer for MPI_Allgather    // From skeleton code

double xCamera = -50, yCamera = 0, zCamera = 100; // Camera position with origin at the center
double xCenter = 0, yCenter = 0, zCenter = 0;     // Look at
double dirCamera[3] = {0.0, 0.0, 1.0};            // Camera vector

int timeSlice = 0;               // Time slice to implement the color oscillation
Field field;                     // Initiate an field object

/*
* Function name: changeSize
* Summary: adjust the perspective according to the change of window
* Parameters:
* 1. int w: weigth of the window
* 2. int h: height of the window
* Return: N/A
*/
void changeSize(int w, int h)
{
    float ratio = ((float)w) / ((float)h);      // window aspect ratio
    glMatrixMode(GL_PROJECTION);                // projection matrix is active
    glLoadIdentity();                           // reset the projection
    gluPerspective(60.0, ratio, 0.1, 200.0);    // perspective transformation
    glMatrixMode(GL_MODELVIEW);                 // return to modelview mode
    glViewport(0, 0, w, h);                     // set viewport (drawing area) to entire window
}

/*
* Function name: update
* Summary: Redisplay the view
* Parameters: N/A
* Return: N/A
*/
void update(void)
{
    glutPostRedisplay();            // Redisplay the scene
}

/*
* Function name: keyboard
* Summary: keyboard command processing (which is not required by the project but useful to adjust)
* Parameters:
* 1. unsigned char key: the key inputed;
* 2&3. int xx & int yy: indicate the mouse location in window relative coordinates when the key was pressed(required by glutKeyboardFunc)
* Return: N/A
*/
void keyboard(unsigned char key, int xx, int yy) {
    switch (key)
    {
        case 'r':
        {
            double angle = atan2((yCamera-yCenter),(xCamera-xCenter))-(10.0/180)*3.14;
            //Calculate the angle of eye position in terms of the center of the chess board
            double r = sqrt((xCamera-xCenter)*(xCamera-xCenter) + (yCamera-yCenter)*(yCamera-yCenter));
            xCamera = r*cos(angle)+xCenter;
            yCamera = r*sin(angle)+yCenter;       //Calcule the new x, y position of the eye position
            break;
        }
        case 'd':
        {
            zCamera = zCamera - 1;          //Moves the eye location down z –axis 0.25 m each time
            break;
        }
        case 'D':
        {
            zCamera = zCamera - 1;          //Moves the eye location down z –axis 0.25 m each time
            break;
        }
        case 'u':
        {
            zCamera = zCamera + 1;          //Moves the eye location down z +axis 0.25 m each time
            break;
        }
        case 'U':
        {
            zCamera = zCamera + 1;          //Moves the eye location down z +axis 0.25 m each time
            break;
        }
        case 27:                            // “esc” on keyboard
            exit(0);
            break;
    }
}

/*
* Function name: renderScene
* Summary: Render the color, lighting and draw UAVs and football field
* Parameters: N/A
* Return: N/A
*/
void renderScene(void)
{
    glClearColor(0.7, 0.9, 1.0, 1.0);            // Clear color with light blue
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);                     // Enable depth test
    glShadeModel(GL_SMOOTH);                     // Enable smooth
    glEnable(GL_COLOR_MATERIAL);                 // Enable material color
    glEnable(GL_NORMALIZE);                      // Enable normalize
    glEnable(GL_LIGHTING);                       // Enable lighting
    glEnable(GL_COLOR_MATERIAL);                 // Enable color material
    glEnable(GL_LIGHT0);                         // Enable light0
    
    glLoadIdentity();		                     // Reset transformations

    if(timeSlice>50 && timeSlice <250)           // While the UAVs have not all arrived
    {
        if (timeSlice%50 == 0)                   // Change the view every 5 seconds
        {
            zCenter += 10;
            zCamera -= 5;
        }
    }
    
    gluLookAt(                                   // Set the perspetive
        xCamera, yCamera, zCamera,
        xCenter, yCenter, zCenter,
        dirCamera[0], dirCamera[1], dirCamera[2]
    );
    
    glMatrixMode(GL_MODELVIEW);

    field.drawField();                // Draw football field
    field.drawSphere();               // Draw the sphere
    field.drawUavs(timeSlice);        // Draw the UAVs with color according to the time
    timeSlice++;                      // Increase the time slices;
    glutSwapBuffers();                // Make it all visible
    
    if (timeSlice == 600)             // When 60s has passed, stop to render scene
    {
        // MPI_Finalize();
        exit(0);
    }
    MPI_Allgather(sendBuffer, numElements, MPI_DOUBLE, rcvbuffer, numElements, MPI_DOUBLE, MPI_COMM_WORLD);      // Gather and boardcast the locations and velosties of the UAVs
    field.setParametersMpi(rcvbuffer, numElements);
}

/*
* Function name: timerFunction
* Summary: Set up a timer for 100 miliseconds
* Parameters: int id
* Return: N/A
*/
void timerFunction(int id)
{
    glutPostRedisplay();
    glutTimerFunc(100, timerFunction, 0);
}

/*
* Function name: mainOpenGL
* Summary: standard GLUT initializations and callbacks for OpenGL to work
* Parameters:  int argc, char** argv : command line input parameters
* Return: N/A
*/
void mainOpenGL(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(400, 400);        // Set window size
    glutInitWindowPosition(320, 120);    // Set window position on the screen
    glutCreateWindow("GaTech Buzzy Bowl");  // Create window
    //glutIdleFunc(update);
    glutReshapeFunc(changeSize);         // Deal with window size changes
    glutDisplayFunc(renderScene);        // Render sence to draw field, sphere and UAVs
    glutKeyboardFunc(keyboard);          // Deal with keyboard command
    glutTimerFunc(100, timerFunction, 0);// Set up timer
    glutMainLoop();                      // Main loop for OpenGl
}

/*
* Function name: Main
* Summary: Main entry point determines rank of the process and follows the correct program path
* Parameters: int argc, char** argv: command line input parameters
* Return: N/A
*/
int main(int argc, char** argv)
{
    int numTasks, rank;                 // Number of tasks and rank
    int rc = MPI_Init(&argc, &argv);    // Initiate MPI
    if (rc != MPI_SUCCESS)
    {
        printf("Error starting MPI program. Terminating.\n");       // If not successed
        MPI_Abort(MPI_COMM_WORLD, rc);
    }

    MPI_Comm_size(MPI_COMM_WORLD, &numTasks);   // Get number of tasks
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);       // Get rank
    int gsize = 0;
    MPI_Comm_size(MPI_COMM_WORLD, &gsize);

    if (rank == 0)                              // In the main process
    {
        mainOpenGL(argc, argv);                 // Call OpenGL main to render scene and draw
    }
    else                                        // In other process for UAVs
    {
        unsigned int uavNum = rank-1;           // Get current UAV number according to the rank
        std::this_thread::sleep_for(std::chrono::seconds(5));   // Sleep for 5 seconds
        for (int ii = 0; ii < 600; ii++)        // Start the 60s flight(600 frame)
        {
            field.collisionDealing();           // Check and deal with collision between UAVs
            field.uavStatus[uavNum].updateLocation();   // Calculate the new locations of UAVs
            field.getParametersMpi(sendBuffer, uavNum);    // Put the parameters in the sendBuffer
        
            MPI_Allgather(sendBuffer, numElements, MPI_DOUBLE, rcvbuffer, numElements, MPI_DOUBLE, MPI_COMM_WORLD);    // Gather and boardcast the parameters of UAVs
            
            field.setParametersMpi(rcvbuffer, numElements); // Set the parameters to the UAVs
            
        }
    }
    MPI_Finalize();                // Finalize the MPI
    return 0;
}
