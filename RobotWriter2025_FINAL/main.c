#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "rs232.h"
#include "serial.h"
#include "PenDrawing.h"
//#define bdrate 115200               /* 115200 baud */

//void SendCommands (char *buffer );

//Font Array
SSFfont FontCharacterList [128];

int main()
{

    
    char Gcodebuf[50]; int SerialPort=4;
    int PENstate=0;
    float Xcur=0.0f; float Ycur=0.0f;


    // If we cannot open the port then give up immediately
    if ( CanRS232PortBeOpened() == -1 )
    {
        printf ("\nUnable to open the COM port (specified in serial.h) \n ");
        exit (0);
    }

    // Time to wake up the robot
    printf ("\nAbout to wake up the robot\n");
    PrintBuffer ("\n");
    Sleep(100); 
    // This is a special case - we wait  until we see a dollar ($)
    WaitForDollar();

    printf ("\nThe robot is now ready to draw\n");

    // Call [F1] to load the Font

    if (LoadSSFfile ("SingleStrokeFont.txt", FontCharacterList)== -1)
    {
        printf("The SingleStrokeFont.txt file was not found\n");
        CloseRS232Port(); return -1;
    }
    else 
    {
        printf("The Font file loaded\n");
    }

    // Call [F2] to get desired text height from user
    
    printf("what is your desired text height\n");
    float Height = UserInputTextHeight();
    
    if (Height == -1)
    {
        printf ("This height is INVALID\n");
        CloseRS232Port(); return -1;
    }

    // Call [F3] to calculate the scale for this desired hight

    float SCALE = ScaleFactorCalculation (Height);
    printf ("The scale factor is: %.3f\n", SCALE);

    // Call [F9] to check robot starts up properly (send M3 and S0)

    if (RobotStartUp (SerialPort,&PENstate,Gcodebuf) == -1)
    {
        printf ("An error occured during STARTUP commands\n");
    }
    char okBuf [30];
    // GcodetoArduino ("G90\n",SerialPort,okBuf); // Safety check

    // Call [F4] to process the text file ( reads it, word width calculations, Gcode transmission)

    printf ("The text file is being processed\n");

    int Processing = ProcessTheTextFile ("test.txt", FontCharacterList,SCALE );

    if (Processing==0)
    {
        printf ("The drawing has completed\n");
    }
    else if (Processing == -1)
    {
        printf ("The text file was not found\n");
    }
    else if (Processing == -2)
    {
        printf ("There is a word in the file that is too long for the width of the page");
    }

    // Call [F8] to return the PEN to the origin point and finish

    ReturnPenToOrigin (SerialPort,&Xcur,&Ycur,&PENstate,Gcodebuf);
    CloseRS232Port();
    printf ("Finished\n");
    return 0;
}






















