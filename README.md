# Drawing Robot Project (C)

An embedded control system developed in C to translate user-inputted text into G-code motion commands for a 2-axis robotic gantry.

## Key Project Documents
**To understand the complete system architecture and logic flow, please refer to the following:**
*   **[Full System Manual (PDF)](./docs/SystemManualAS_20576162.pdf)** - Explanation of this project and detailed breakdown of functions, data structures and testing protocols.
*   **[System Flowchart (PDF)](./docs/SystemFlowchartAS_20576162.pdf)** - Visual mapping of the softwares descision making process (Funcitions F1-F12)

## Project Structure
*   `/RobotWriter2025_FINAL`: Source code including `main.c` and `PenDrawing.c`.
*   `/docs`: Technical documentation and planning files.

## Tech Stack
*   **Language:** C 
*   **Protocol:** RS232 Serial Communication
*   **Hardware:** Arduino, 2-Axis Drawing Gantry
*   **Documentation:** Flowchart design and technical manual writing

## Acknowledgements
*   Core application logic and G-Code generation: Akshmen Sutharshan.
*   Serial communication library: Provided by University of Nottingham.
