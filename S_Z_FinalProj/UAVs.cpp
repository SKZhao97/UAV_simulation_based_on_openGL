/*
Author: Sikai Zhao
Class: ECE 6122
Last date modified: 11/27/2019
Description:
 1. Implement the class of UAVs
 2. Implement the functions to calculate force, velocity, location and draw UAV.
 3. Implement the functions to generator random tangent force to let UAVs fly along sphere
*/

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <ctime>

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glut.h>
#endif

#include "UAVs.h"
#include "Field.h"

GLfloat mat_specular[] = {0.5, 0.5, 0.5, 1.0};     // Set mat specular as the value in Hmk6
GLfloat shininess[] = {50.0};                      // Set shiness as the value in Hmk6

/*
* Function name: UAVs
* Summary: Constructor, set the target of the UAV
* Parameters: N/A
* Return: N/A
*/
UAVs::UAVs()
{
    uavTarget.x = 0;
    uavTarget.y = 0;                        // Target is in the center
    uavTarget.z = 50;                       // The height of the target is 50m
}

/*
* Function name: displayColor
* Summary: Display color with oscillation
* Parameters:
    1. int timeSlice: the time slice, which is used to set the color oscillately
* Return: N/A
*/
void UAVs::displayColor(int timeSlice)
{
    int round = timeSlice/128;              // Calculate the round num of change from 1 to 128
    int display = 255;                      // Initiate display
    if(round%2 == 0)                        // In the even round
    {
        display = 255 - timeSlice%128;      // Change from 255 to 128
    }
    else if(round%2 == 1)                   // In the odd round
    {
        display = 128 + timeSlice%128;      // Change from 128 to 256
    }
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular); // Set specular
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shininess);   // Set shiness
    glClearColor(0.0, 0.0, 0, 1.0);         // Clear color
    glColor3ub(display, 0, 0);              // Set to red
}

/*
* Function name: drawUav
* Summary: Draw UAV with color oscillation
* Parameters:
    1. int timeSlice: the time slice, which is used to set the color oscillately
* Return: N/A
*/
void UAVs::drawUav(int timeSlice)
{
    glPushMatrix();                         // Push the current position
        displayColor(timeSlice);            // Call function to set color
        glTranslatef(uavLocation.x, uavLocation.y, uavLocation.z);  // Translate to the location
        glTranslatef(0, 0, 1.0);            // Translate upper
        glRotated(90, 1, 0, 0);             // Rotate the UAV along x-axis
        glRotated(90, 0, 1, 0);             // Rotate the UAV along y-axis
	glutSolidTeapot(1);                 // Draw UAV
    glPopMatrix();                          // Pop the current position
}

/*
* Function name: distanceToTarget
* Summary:  Calculate the distance to the target
* Parameters: N/A
* Return:
    1. double distance: the calculated distance
*/
double UAVs::distanceToTarget()
{
    double dx = uavTarget.x - uavLocation.x;        // Distance difference on x-axis
    double dy = uavTarget.y - uavLocation.y;        // Distance difference on y-axis
    double dz = uavTarget.z - uavLocation.z;        // Distance difference on z-axis
    double distance = sqrt(dx*dx + dy*dy + dz*dz);  // The distance
    return distance;                                // Return
}

/*
* Function name: randomNumGenerator
* Summary: Generate a random number in the range of [1.00, 3.99]
* Parameters: N/A
* Return: N/A
*/
double UAVs::randomNumGenerator()
{
    int min = 100;            // Lower bound(multuply 100)
    int max = 399;            // Upper bound(multuply 100)
    int k = rand();           // Generate random number
    double res = (k % (max - min) + min)/100;     //Get the random number
    return res;               // Return the generated random number
}

/*
* Function name: randomVectorGenerator
* Summary: Generate a random vector tangent to the sphere to give the UAV a force to fly around
* Parameters:
    1. dirVector normal: the vector of the UAV's direction to the target
* Return: N/A
*/
void UAVs::randomVectorGenerator(dirVector normal)
{
    srand((unsigned)time(NULL));        // Generate the seed
    double x = randomNumGenerator();    // Get a random number for x
    double y = randomNumGenerator();    // Get a random number for y
    double z = randomNumGenerator();    // Get a random number for z
    double choice = int(randomNumGenerator());// Get a random number of the direction to calculate
    
    /* There will be xx' + yy' + zz' = 0 if the two vectors are perpendicular */
    if (choice == 1)
    {
        z = -x * normal.dirX + -y * normal.dirY;  // Calculate z accordinhg to random number x,y
    }
    else if (choice == 2)
    {
        y = -x * normal.dirX + -z * normal.dirZ;  // Calculate y accordinhg to random number x,z
    }
    else if(choice == 3)
    {
        x = -y * normal.dirY + -z * normal.dirZ;  // Calculate x accordinhg to random number y,z
    }
    double magnitude = sqrt(pow(x,2)+pow(y,2)+pow(z,2));    // Calculate the magnitude
    randomVector.dirX = x/magnitude;        // Normalize
    randomVector.dirY = y/magnitude;        // Normalize
    randomVector.dirZ = z/magnitude;        // Normalize
}

/*
* Function name: calculateForce
* Summary: Calculate the suitable force to let the UAVs fly to the target and then fly around the sphere. When the UAV has not arrived the sphere, with a velocity less than 2m/s and when it arrives the sphere, give force accodring to Hooke's Law; When the speed of UAV increases, decrease the force in case of losing control and flying away; Give random tangent force when the UAVs arrives the sphere.
* Parameters: N/A
* Return: N/A
*/
void UAVs::calculateForce()
{
    double distance = distanceToTarget();       // Calculate the distance to the target
    double dx = uavTarget.x - uavLocation.x;    // Distance difference on x-axis
    double dy = uavTarget.y - uavLocation.y;    // Distance difference on y-axis
    double dz = uavTarget.z - uavLocation.z;    // Distance difference on z-axis
    double dirX = dx/distance;                  // Normalize
    double dirY = dy/distance;                  // Normalize
    double dirZ = dz/distance;                  // Normalize
    
    double force = 0;                       // Initiate force
    double vel = calculateVelocityVal();    // Calculate the speed
    if (!arriveSphere)                      // If the UAV has not arrived the sphere
    {
        if(vel < 2)                         // If the speed has not arrived the maximum
        {
            force =  -kConst*(10 - distanceToTarget());  // Use the Hooke Law to get a force
        }
        else
        {
            force = 0;                      // Else stop implementing the force
        }
        if(distance <= 10)                  // If the UAV arrived the sphere
        {
            arriveSphere = true;            // Set the flag to true
        }
    }
    else
    {
        force =  -kConst*(10 - distanceToTarget());  // If the UAV arrives the sphere, give it force
        if(speedUp)
        {
            force *= 0.75;  // If the Speed of UAV is increasing, then decrease the force to avoid UAV flying away so quickly that Hooke's Law cannot catch it back
        }
    }
    
    if(force >= maxForce + gravity)    // If the force is larger than the maximum and gravity
        force = maxForce + gravity;    // Set it to it
    else if (force <= -(maxForce + gravity))    // The counter direction
        force = -(maxForce + gravity);          // The same
    
    uavParameter.xForce = dirX * force;         // Calculate the force on the x-axis
    uavParameter.yForce = dirY * force;         // Calculate the force on the y-axis
    uavParameter.zForce = dirZ * force;         // Calculate the force on the z-axis
    
    if(calculateVelocityVal() > 10)                       // If the speed exceed the limitation
    {
        vel = calculateVelocityVal();                     // Calculate the speed
        uavParameter.xForce = -10 * uavParameter.xVel/vel;
        uavParameter.yForce = -10 * uavParameter.yVel/vel;
        uavParameter.xForce = -10 * uavParameter.zVel/vel;// Give a force to decrease the velosity
    }
    
    if(arriveSphere)                    // If the UAV arrives the sphere
    {
        addTangentForce(dirX, dirY, dirZ);
    }
    double antiGravity = - gravity;                 // The force to offset the gravity
    uavParameter.zForce += (antiGravity + gravity); // The total force on z direction
}

/*
* Function name: addTangentForce
* Summary: Calcute the force in tangent direction that drives the UAVs fly around the sphere and add it to the force
* Parameters:
* 1. double x:  x of the vector towards target
* 2. double y:  y of the vector towards target
* 3. double z:  z of the vector towards target
* Return: N/A
*/
void UAVs::addTangentForce(double x, double y, double z)
{
    dirVector normal;
    normal.dirX = x;
    normal.dirY = y;
    normal.dirZ = z;             // Calculate the direction vector towards the center
    double tangentForce = 0.4;      // The tangent force to drive them fly around
    randomVectorGenerator(normal);  // If the force has not been initiate, then generate
    uavParameter.xForce += tangentForce * randomVector.dirX;    // Add this force to the force
    uavParameter.yForce += tangentForce * randomVector.dirY;    // Add this force to the force
    uavParameter.zForce += tangentForce * randomVector.dirZ;    // Add this force to the force
}

/*
* Function name: calculateVelocityVal
* Summary: Calculate the value of the velocity
* Parameters: N/A
* Return: N/A
*/
double UAVs::calculateVelocityVal()
{
    double veloVal = sqrt(pow(uavParameter.xVel, 2) + pow(uavParameter.yVel, 2) + pow(uavParameter.zVel,2));
    return veloVal;         // Return the value
}

/*
* Function name: calculateAcceleration
* Summary: Calculate the acceleration according to the force
* Parameters: N/A
* Return: N/A
*/
void UAVs::calculateAcceleration()
{
    uavParameter.xAcc = uavParameter.xForce/uavMass;
    uavParameter.yAcc = uavParameter.yForce/uavMass;
    uavParameter.zAcc = uavParameter.zForce/uavMass;    // Calculate and update the values
}
/*
* Function name: updateLocation
* Summary: update the location of the UAVs
* Parameters: N/A
* Return: N/A
*/
void UAVs::updateLocation()
{
    calculateForce();               // Call function to calculate the suitable force
    calculateAcceleration();        // Call function to calculate the accelerations
    double oldVelosity = calculateVelocityVal();
    // Calculate and update the new position
    uavLocation.x = uavLocation.x + uavParameter.xVel * timeSlice + 0.5*uavParameter.xAcc * pow(timeSlice,2);
    uavLocation.y = uavLocation.y + uavParameter.yVel * timeSlice + 0.5*uavParameter.yAcc * pow(timeSlice,2);
    uavLocation.z = uavLocation.z + uavParameter.zVel * timeSlice + 0.5*uavParameter.zAcc * pow(timeSlice,2);
    // Calculate and update the new velocities
    uavParameter.xVel = uavParameter.xVel + uavParameter.xAcc * timeSlice;
    uavParameter.yVel = uavParameter.yVel + uavParameter.yAcc * timeSlice;
    uavParameter.zVel = uavParameter.zVel + uavParameter.zAcc * timeSlice;
    double newVelosity = calculateVelocityVal();
    speedUp = newVelosity > oldVelosity;    // Set speedUp flag as 1 if the speed increased
}
