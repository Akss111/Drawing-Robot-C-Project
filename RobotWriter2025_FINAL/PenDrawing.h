#ifndef PenDrawing_H
#define PenDrawing_H

// Struct MC
typedef struct 
{
    float x;
    float y;
    int pen;
}   MC;


typedef struct 
{
    char character;
    int GRIDsize;
    int NumberMoves;
    MC Moves [35];
}   SSFfont;

// Function Declarations 

// [F1]
int LoadSSFfile(const char* Filename, SSFfont FontCharacterList[128]);

// [F2]
float UserInputTextHeight (void);

// [F3]
float ScaleFactorCalculation (float Height);

// [F4]
int ProcessTheTextFile (const char *TextFile, SSFfont FontCharacterList [] , float ScaleFactor);

// [F5]
int CharacterMovementsNeeded (char CharCurrent, SSFfont FontCharacterList [], MC MoveCommands [] );

// [F6]
int MovementToGcode (MC MoveCommands [], int NumOfMovement, float *Xcurrent, float *Ycurrent, float ScaleFactor, char *Gcodebuf, int SerialPort);

// [F7]
int CharacterPositionUpdate (float *Xcurrent, float *Ycurrent, float Xfinal, float Yfinal );

// [F8]
int LineBreakHandling (float *Xcurrent, float *Ycurrent, float MaxLineWidth, float LineBreakSpacing);

// [F9]
int RobotStartUp (int SerialPort, int *PENstate, char Gcodebuf[] );

// [F10]
int GcodetoArduino (const char *Gcodebuf, int SerialPort,  char okBuf [] );

// [F11]
int ReturnPenToOrigin (int SerialPort, float *Xcurrent, float *Ycurrent, int *PENstate, char Gcodebuf [ ] );

// [F12]
float CalculateTheWordWidth (const char *WordBuf, SSFfont FontCharacterList [], float ScaleFactor);

#endif