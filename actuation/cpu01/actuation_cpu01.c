// FILE:   actuation_cpu01.c

#include "F28x_Project.h"     // Device Header File and Examples Include File

// Function Prototypes
void ConfigureDAC(void);

// Variables
#define RESULTS_BUFFER_SIZE 300
Uint16 dacOutput;
Uint16 LoadTorque = 0xFFF;
Uint16 DutyCycle = 0x7FF;
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

    // Send Load Torque and Duty Cycle to Opal
    DacaRegs.DACVALS.all = LoadTorque;
    DacbRegs.DACVALS.all = DutyCycle;

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
