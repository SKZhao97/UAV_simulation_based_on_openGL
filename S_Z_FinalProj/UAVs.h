/*
Author: Sikai Zhao
Class: ECE 6122
Last date modified: 11/30/2019
Description:
 1. Define the class of UAVs, providing the functions to do kinematic calculations and update.
 2. Provide useful data structure for calculation
*/

#ifndef UAVs_h
#define UAVs_h

#include <stdio.h>

struct location             // Struct to store the location imformation
{
    double x;               // location on x-axis
    double y;               // location on y-axis
    double z;               // location on z-axis
};

struct dirVector            // Struct to store the direction imformation
{
    double dirX;            // direction on x-axis
    double dirY;            // direction on y-axis
    double dirZ;            // direction on z-axis
};

struct parameter            // Struct to store the kinectic parameters
{
    double xVel = 0;        // velocity on x-axis
    double yVel = 0;        // velocity on y-axis
    double zVel = 0;        // velocity on z-axis
    double xAcc = 0;        // acceleration on x-axis
    double yAcc = 0;        // acceleration on y-axis
    double zAcc = 0;        // acceleration on z-axis
    double xForce = 0;      // force on x-axis
    double yForce = 0;      // force on y-axis
    double zForce = 0;      // force on z-axis
};

class UAVs                  // Define the class of UAVs
{
public:
    const double timeSlice = 0.1;    // Define the time slice to 0.1 second (100 miliseconds)
    const double uavMass = 1;        // Define the mass of a UAV
    const double kConst = 2.5;         // Define the constant in the Hooke Law
    const double maxForce = 20;      // Define the maximum of the force
    const double gravity = -10;      // Define the gravity
    bool arriveSphere = false;       // Flag showing if the UAV arrives the sphere
    bool speedUp = false;        //
    location uavLocation;            // The location of a UAV
    location uavTarget;              // The target of a UAV
    parameter uavParameter;          // The parameter of a UAV
    dirVector randomVector;          // The random direction vector of a UAV
     
public:
    UAVs();                          // Constuctor to initiate a UAV
    void drawUav(int timeSlice);     // Draw UAV with color oscillation
    void displayColor(int timeSlice);// Display color with oscillation
    double distanceToTarget();       // Calculate the distance to the target
    void updateLocation();           // Update the location of the UAV
    void calculateForce();           // Calculate the suitable force
    void calculateAcceleration();    // Calculate the acceleration
    void addTangentForce(double x, double y, double z); // Add tangent force to the force
    double calculateVelocityVal();   // Calculate the magnitude of the speed
    
    double randomNumGenerator();     // Function to generate random number
    void randomVectorGenerator(dirVector normal);   // Generate a random direction vector according to its driection vector towards the target
};

#endif
