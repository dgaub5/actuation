// FILE:   actuation_cpu01.c

#include "F28x_Project.h"     // Device Header File and Examples Include File

// Function Prototypes
void ConfigureDAC(void);

// Variables
#define RESULTS_BUFFER_SIZE 300
Uint16 dacOutput;
float LoadTorque = 0.1; 	// {-0.2, 0.2} [Nm]
float DutyCycle = 100; 		// {0, 100} [%]
extern int QuadratureTable[40];


void main(void)
{
	// Initialize System Control
    InitSysCtrl();
    EALLOW;
    ClkCfgRegs.PERCLKDIVSEL.bit.EPWMCLKDIV = 1;
    EDIS;

    // Configure DACs
    ConfigureDAC();

    DutyCycle = (DutyCycle/100)*4095;		// convert floating point number to number of bits between 0 and 4095
    LoadTorque = (10237.5)*(LoadTorque)+2047.5; // convert floating point number to number of bits between 0 and 4095

    do {
    // Send Load Torque and Duty Cycle to Opal
    DacaRegs.DACVALS.all = LoadTorque;
    DacbRegs.DACVALS.all = DutyCycle;
    } while(1);

}


void ConfigureDAC(void)
{
    EALLOW;
    DacbRegs.DACCTL.bit.DACREFSEL = 1;		// Use ADC references
    DacbRegs.DACCTL.bit.LOADMODE = 0;		// Load on next SYSCLK
    DacbRegs.DACOUTEN.bit.DACOUTEN = 1;		// Enable DAC
    DacaRegs.DACCTL.bit.DACREFSEL = 1;          // Use ADC references
    DacaRegs.DACCTL.bit.LOADMODE = 0;           // Load on next SYSCLK
    DacaRegs.DACOUTEN.bit.DACOUTEN = 1;         // Enable DAC
    EDIS;
}

 // end of file
