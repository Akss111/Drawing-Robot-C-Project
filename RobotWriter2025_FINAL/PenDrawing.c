#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "PenDrawing.h"
#include "serial.h"

// [F1]

int LoadSSFfile (const char* Filename, SSFfont FontCharacterList [128])
{
 FILE *file = fopen (Filename,"r");

 if (!file) 
 return -1; // this is so file can be checked if it has opened 

 char line [256];
 int NewMarker,ASCIIcode, NumberMoves;
 int count = 0;

 while (fgets (line, sizeof(line), file)) // the file is now read line by line
 {
    if (sscanf (line, "%d %d %d",&NewMarker, &ASCIIcode,&NumberMoves) == 3) // confirm the 999 marker (start of a new character)
    {
        if (NewMarker == 999 && ASCIIcode >= 0 && ASCIIcode < 128) // check if the ascii character is real
        {
            FontCharacterList [ASCIIcode] . character = (char)ASCIIcode;
            FontCharacterList [ASCIIcode]. NumberMoves = NumberMoves;
            FontCharacterList [ASCIIcode]. GRIDsize = 18;

            for (int i = 0; i<NumberMoves; i++) //loop counter to read movement lines of the specifc character
            {
                if (fgets (line, sizeof(line), file))
                {
                    float x;
                    float y;
                    int p;
                    sscanf (line, "%f %f %d", &x, &y, &p); // this is to parse the X coordinates, Y coordinates, and pen states
                    FontCharacterList [ASCIIcode] . Moves[i] . x = x;
                    FontCharacterList [ASCIIcode] . Moves[i] . y = y;
                    FontCharacterList [ASCIIcode] . Moves[i] . pen = p;
                }
            }
        }
        count ++;

    }
 }
 fclose (file); return 128;

}

// [F2]

float UserInputTextHeight (void)
{
    float height;
    // going to include code to manually put height
    // printf (" What is the desired text height");
    if(scanf ("%f", &height)  != 1)
    return -1;

    if (height<4.0 || height>10.0) //ensure the input is within the range
    {
        return -1;
    }

    return height;
}

// [F3]

float ScaleFactorCalculation (float Height)
{
    return Height/18; // The font is defined on an 18 unit grid. The users height / 18 = mutilplier for all coordinates
}

// [F4] the main loop

int ProcessTheTextFile (const char *TextFile, SSFfont FontCharacterList [] , float ScaleFactor)
{
    FILE *file = fopen (TextFile, "r");
    if (!file)
    return -1;

    // generate the buffers
    char Gcodebuf [256]; 
    MC MoveCommands [64];
    char Word [256];  // replaced TEXTbuf with Word from preliminary report 

    // X and Y setting. Line limits are defined 
    float Xcurrent = 0.0;
    float Ycurrent = 0.0;
    float MaxLineWidth = 100.0;
    float LineBreakSpace = 5.0;

    
    while (fscanf (file, "%99s",Word)==1 ) // The file is read one word at a time
    {
        float WordWidth = CalculateTheWordWidth (Word,FontCharacterList,ScaleFactor); // [F12] called to calculate the word width

        if (WordWidth  >  MaxLineWidth) // if the word width is wider than the page then it will not be printed
        {
            fclose(file); return -2;
        }

        if ((Xcurrent + WordWidth) >  MaxLineWidth) //  If adding this word will exceed the 100mm limit then move to a new line
        {
            LineBreakHandling (&Xcurrent,&Ycurrent,MaxLineWidth,LineBreakSpace); // [F8] is called to handle the line break

        }

        for (int i = 0; i < strlen(Word);i++) // every character in the word is looped 
        {
            int NumberMoves = CharacterMovementsNeeded (Word [i],FontCharacterList,MoveCommands ); // call [F5] to get the current letters movement data
            if (NumberMoves>0)
            {
                MovementToGcode (MoveCommands,NumberMoves,&Xcurrent,&Ycurrent,ScaleFactor,Gcodebuf,0 ); // call [F6] to convert the movements to G code to be sent to the robot
            }

            float CharWidth = 0; int idx = (int) Word[i]; // the width of the character that was just drawn is calculated to update the X position 
            for (int q=0; q < FontCharacterList[idx].NumberMoves; q++)
            {
                if (FontCharacterList[idx].Moves[q].x > CharWidth)
                CharWidth = FontCharacterList [idx]. Moves[q] .x;
            }

            float Xfinal = Xcurrent + (CharWidth*ScaleFactor) + (2*ScaleFactor); // new X position implementing my scale
            
            CharacterPositionUpdate (&Xcurrent,&Ycurrent,Xfinal,Ycurrent); // [F7] to update the coordinate pointers
        }
        float XfinalSpacing = Xcurrent+(5*ScaleFactor); // A space is added here after the word has been drawn
            CharacterPositionUpdate (&Xcurrent,&Ycurrent,XfinalSpacing,Ycurrent); // call [F7]
    }
    fclose(file); 
    return 0;
}


// [F5]

int CharacterMovementsNeeded (char CharCurrent, SSFfont FontCharacterList [], MC MoveCommands [] )
{
    int idx = (int) CharCurrent; // check ASCII is within the bounds 
    if (idx<0 || idx>127)
    return -1;

    int MoveCount = FontCharacterList[idx]. NumberMoves; // This is for spaces or "empty" characters 
    if (MoveCount == 0)
    return 0;

    for (int i=0; i<MoveCount; i++) // data is copied from the large struct to the array
    {
        MoveCommands [i] = FontCharacterList[idx]. Moves[i];
    }

    return MoveCount;
}

// [F6]
/*
during robot testing, the pen was repaetedly moving up and down withought drawing because S0 
and S1000 commands was being sent with each G code string. This caused the pen to jitter instead 
of drawing smooth letters. 
When implementing the Pen state tracker to fix the robot issue, the G code simulator began to show 
squiggly lines instead of the corrected text output.
However, when using the original code before pen tracker modifications, the G code simulator
correctly displayed the text within the text.file 

This created a conflict because the Simulator required each S0/S1000 with every command and
that would cause the Pen on the robot to jitter if the same G code commands were parsed 

Therefore i have implemented a solution, switchable modes using a block comment
*/

int MovementToGcode (MC MoveCommands [], int NumOfMovement, float *Xcurrent, float *Ycurrent, float ScaleFactor, char *Gcodebuf, int SerialPort)
{
    char okBuf[16];

    // ========= ROBOT TESTING MODE - uncomment */ below ===========
/*

    static int LastPenState = -1;  // setting to -1 is an impossible state so the code will check if (-1 != 0) is true. This forces the robot to update and send the correct S command for the first movement.
           //                         This ensures the pen state is in the correct position before it begins to move                        
    
    for (int i=0; i < NumOfMovement; i++)
    {
        float Xtarget = *Xcurrent + (MoveCommands[i].x * ScaleFactor); // applying the scale factor to the current coordinates position
        float Ytarget = *Ycurrent + (MoveCommands[i].y * ScaleFactor);

        // This is the pen state checker 
        if (LastPenState != MoveCommands [i]. pen)
        {
            if (MoveCommands[i]. pen ==0)
            {
                sprintf(Gcodebuf,"S0\nG0 X%.2f Y%.2f\n",Xtarget,Ytarget);
            }
            else
            {
                 sprintf(Gcodebuf,"S1000\nG1 X%.2f Y%.2f\n",Xtarget,Ytarget);
            }

            LastPenState = MoveCommands [i]. pen;
        }

        else

        {
            if (MoveCommands[i]. pen ==0) // if pen state does not chnage then continue
            {
                sprintf(Gcodebuf,"G0 X%.2f Y%.2f\n",Xtarget,Ytarget);
            }
            else
            {
                 sprintf(Gcodebuf,"G1 X%.2f Y%.2f\n",Xtarget,Ytarget);
            }
        }

         if (GcodetoArduino (Gcodebuf, SerialPort, okBuf) == -1)
         return -1;
         
    }

*/
    // ========END ROBOT VERSION - use */ above============== 

 //   ============== SIMULATOR TESTING MODE START - uncomment /* below ================
 ///*

    for (int i=0; i < NumOfMovement; i++)
    {
        float Xtarget = *Xcurrent + (MoveCommands [i]. x * ScaleFactor); 
        float Ytarget = *Ycurrent + (MoveCommands [i]. y * ScaleFactor);

        if (MoveCommands[i]. pen == 0) // in this section, "S" commands are sent with each G code 
        {
            sprintf(Gcodebuf,"S0\nG0 X%.2f Y%.2f\n",Xtarget,Ytarget);
        }
        else
        {
            sprintf(Gcodebuf,"S1000\nG1 X%.2f Y%.2f\n",Xtarget,Ytarget);
        }
        if (GcodetoArduino (Gcodebuf, SerialPort, okBuf) == -1)
        return -1;
    }

//*/
//    ============= END SIMULATOR TESTING MODE =======================


    return 0;
}

// [F7]

int CharacterPositionUpdate (float *Xcurrent, float *Ycurrent, float Xfinal, float Yfinal )
{
    *Xcurrent=Xfinal; // These pointers update the X and Y variables to the updated end coordinates
    *Ycurrent=Yfinal;

    return 0;
}

// [F8]

int LineBreakHandling (float *Xcurrent, float *Ycurrent, float MaxLineWidth, float LineBreakSpacing)
{
    *Xcurrent=0.0; // reset X to 0 to start a new line
    *Ycurrent -= LineBreakSpacing; // Move Y down by 5mm

    char Gcodebuf [50];
    char okBuf [16];
    
    sprintf(Gcodebuf,"S0\nG0 X0.00 Y%.2f\n", *Ycurrent); // G code generated to move the robot to the start of the new line 
    GcodetoArduino (Gcodebuf,0,okBuf); // This will immediately send the move command to the robot 

    return 0;
}

// [F9]

int RobotStartUp (int SerialPort, int *PENstate, char Gcodebuf[] )
{
    char okBuf [16];
    sprintf (Gcodebuf, "F1000\n"); // Feed rate is 1000 mm/min
    if (GcodetoArduino (Gcodebuf,SerialPort,okBuf) == -1)
    return -1;
    sprintf (Gcodebuf, "M3\n");  // This will turn on the spindle
    if (GcodetoArduino (Gcodebuf,SerialPort,okBuf) == -1)
    return -1;
    sprintf (Gcodebuf,"S0\n"); // This raises the pen before G code commands are sent
    if (GcodetoArduino (Gcodebuf,SerialPort,okBuf) == -1)
    return -1;

    *PENstate = 0; // set the pen tracker state to 0, pen is up
    return 0;
}

// [F10]
int GcodetoArduino (const char *Gcodebuf, int SerialPort,  char okBuf [] )
{
    PrintBuffer ((char*) Gcodebuf); // This send the G code string through the RS232 
    WaitForReply(); // Wait for the robot to reply with "ok" before executing command
    return 0;
}

// [F11]

int  ReturnPenToOrigin (int SerialPort, float *Xcurrent, float *Ycurrent, int *PENstate, char Gcodebuf [ ] )
{
    char okBuf [16];
    sprintf (Gcodebuf, "S0\n"); // This will make sure the pen is up before the robot moves back to the origin
    GcodetoArduino (Gcodebuf,SerialPort,okBuf);

    *PENstate = 0;

    sprintf (Gcodebuf,"G0 X0 Y0\n"); // This will rapidly move the pen back to the origin
    GcodetoArduino (Gcodebuf,SerialPort,okBuf);
    *Xcurrent = 0; // this ensures the global coordinates are matched with the robots physical position
    *Ycurrent = 0;

    return 0;

}

// [F12] this is a new function implemented after feedback from preliminary report 

float CalculateTheWordWidth (const char *WordBufer, SSFfont FontCharacterList [], float ScaleFactor)
{
    float Width = 0.0;
    int len= (int) strlen(WordBufer);
    for (int i=0;i<len;i++) // This will loop through all the letters in the word
    {
        int idx = (int) WordBufer[i];
        if (idx >= 0 && idx < 128) // confirm it is an ASCII character
        {
            float Xmax = 0;
            for (int z=0; z<FontCharacterList [idx]. NumberMoves; z++) // this will find the X point that is furthest in the current characters geometry
            {
                if (FontCharacterList [idx]. Moves [z]. x > Xmax)
                Xmax = FontCharacterList [idx]. Moves [z]. x;
            }

            Width += (Xmax*ScaleFactor) + (2.0*ScaleFactor); // here i totaled the scaled character width added with the scaled spacing (2 units)
        }
    }

    return Width;
}




    


