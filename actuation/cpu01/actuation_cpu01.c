// Beginning of File

// File: actuation_cpu01.c

// File Description:
// This file contains the code for HIL communication with the OPAL-RT
// to the TI-Microcontoller (This Board). Here this code demonstrates
// two inputs and two outputs.

// Last Edit: 10/30/2021


#include "F28x_Project.h"     // Device Header File and Examples Include File

// Function Prototypes
void ConfigureADC(void);
void ConfigureEPWM(void);
void ConfigureDAC(void);
void SetupADCEpwm(void);
interrupt void adca1_isr(void);

// Variables for output
#define RESULTS_BUFFER_SIZE 300
Uint16 AdcaResults[RESULTS_BUFFER_SIZE];
Uint16 resultsIndex;
Uint16 ToggleCount = 0;
Uint16 mmSpeed = 0x000;         // {-600, 600} [rad/s] - Motor Mechanical Speed rad/s | {0.0 V, 3.3 V}
Uint16 maCurrent = 0x000;       // {0, 100} [A] - Motor Armature Current in A | {0.0 V, 3.3 V}

//Variables for input
Uint16 dacOutput;
Uint16 LoadTorque = 0xFFF;      // {-0.2, 0.2} [Nm] - Load Torque in Nm | {0.0 V, 3.0 V}
Uint16 DutyCycle = 0x7FF;       // {0, 100} [%]  - Load Torque in % | {0.0 V, 3.0 V}

void main(void)
{
	// Initialize System Control
    InitSysCtrl();
    EALLOW;
    ClkCfgRegs.PERCLKDIVSEL.bit.EPWMCLKDIV = 1;
    EDIS;

    // Initialize GPIO
    InitGpio(); 							// Configure default GPIO
    EALLOW;
    GpioCtrlRegs.GPADIR.bit.GPIO30 = 1;		// Used as input to ADC
    GpioCtrlRegs.GPADIR.bit.GPIO31 = 1;		// Drives LED LD2 on controlCARD
    EDIS;
    GpioDataRegs.GPADAT.bit.GPIO30 = 0; 	// Force GPIO18 output LOW
    GpioDataRegs.GPADAT.bit.GPIO31 = 1;		// Turn off LED

    // Clear all interrupts and initialize PIE vector table
    DINT;
    InitPieCtrl();
    IER = 0x0000;
    IFR = 0x0000;
    InitPieVectTable();

    // Map ISR functions
    EALLOW;
    PieVectTable.ADCA1_INT = &adca1_isr; 	// Function for ADCA interrupt 1
    EDIS;

    // Configure the ADC and power it up
    ConfigureADC();

    // Configure the ePWM
    ConfigureEPWM();

    // Configure DACs
    ConfigureDAC();

    // Setup the ADC for ePWM triggered conversions on channel 0
    SetupADCEpwm();



    // Enable PIE interrupt
    PieCtrlRegs.PIEIER1.bit.INTx1 = 1;

    // Enable global interrupts and higher priority real-time debug events
    IER |= M_INT1; 			// Enable group 1 interrupts
    EINT;  					// Enable Global interrupt INTM
    ERTM;  					// Enable Global real-time interrupt DBGM

    // Sync ePWM
    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    EDIS;

    // Start ePWM
    EPwm2Regs.TBCTL.bit.CTRMODE = 0; 			// Un-freeze and enter up-count mode

    do {

      	GpioDataRegs.GPADAT.bit.GPIO31 = 0;		// Turn on LED
      	DELAY_US(1000 * 250);					// ON delay
      	GpioDataRegs.GPADAT.bit.GPIO31 = 1;   	// Turn off LED
      	DELAY_US(1000 * 250);					// OFF delay

    } while(1);
}


// Write ADC configurations and power up the ADC for both ADC A and ADC B
void ConfigureADC(void)
{
	EALLOW;
	AdcaRegs.ADCCTL2.bit.PRESCALE = 6; 			// Set ADCCLK divider to /4
	AdcaRegs.ADCCTL2.bit.RESOLUTION = 0; 		// 12-bit resolution
	AdcaRegs.ADCCTL2.bit.SIGNALMODE = 0; 		// Single-ended channel conversions (12-bit mode only)
	AdcaRegs.ADCCTL1.bit.INTPULSEPOS = 1;		// Set pulse positions to late
	AdcaRegs.ADCCTL1.bit.ADCPWDNZ = 1;			// Power up the ADC
	DELAY_US(1000);								// Delay for 1ms to allow ADC time to power up
	EDIS;
}


void ConfigureEPWM(void)
{
	EALLOW;
	// Assumes ePWM clock is already enabled
	EPwm2Regs.TBCTL.bit.CTRMODE = 3;            // Freeze counter
	EPwm2Regs.TBCTL.bit.HSPCLKDIV = 0;			// TBCLK pre-scaler = /1
	EPwm2Regs.TBPRD = 0x07D0;			        // Set period to 2000 counts (50kHz)
	EPwm2Regs.ETSEL.bit.SOCAEN	= 0;	        // Disable SOC on A group
	EPwm2Regs.ETSEL.bit.SOCASEL	= 2;	        // Select SOCA on period match
    EPwm2Regs.ETSEL.bit.SOCAEN = 1; 			// Enable SOCA
	EPwm2Regs.ETPS.bit.SOCAPRD = 1;		        // Generate pulse on 1st event
	EDIS;
}


void ConfigureDAC(void)
{
    EALLOW;
    DacbRegs.DACCTL.bit.DACREFSEL = 1;			// Use ADC references
    DacbRegs.DACCTL.bit.LOADMODE = 0;			// Load on next SYSCLK
    DacbRegs.DACVALS.all = 0x0800;				// Set mid-range
    DacbRegs.DACOUTEN.bit.DACOUTEN = 1;			// Enable DAC
    EDIS;
}


void SetupADCEpwm(void)
{
	EALLOW;
	AdcaRegs.ADCSOC0CTL.bit.CHSEL = 0;  		// SOC0 will convert pin A0
	AdcaRegs.ADCSOC0CTL.bit.ACQPS = 14; 		// Sample window is 100 SYSCLK cycles
	AdcaRegs.ADCSOC0CTL.bit.TRIGSEL = 7; 		// Trigger on ePWM2 SOCA/C
	AdcaRegs.ADCINTSEL1N2.bit.INT1SEL = 0; 		// End of SOC0 will set INT1 flag
	AdcaRegs.ADCINTSEL1N2.bit.INT1E = 1;   		// Enable INT1 flag
	AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; 		// Make sure INT1 flag is cleared
	EDIS;
}


interrupt void adca1_isr(void)
{
	// Read the ADC result and store in circular buffer
	AdcaResults[resultsIndex++] = AdcaResultRegs.ADCRESULT0;
	if(RESULTS_BUFFER_SIZE <= resultsIndex)
	{
		resultsIndex = 0;
	}

	// Return from interrupt
	AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; 		// Clear ADC INT1 flag
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;		// Acknowledge PIE group 1 to enable further interrupts
}

 // End of File
