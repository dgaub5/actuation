// ----------------------------------------------------------------------------- //
// Beginning of File
//
// File: actuation_cpu01.c
//
// File Description:
// This file contains the code for HIL communication with the OPAL-RT
// to the TI-Microcontoller (This Board). Here this code demonstrates
// two inputs and two outputs.
//
// Last Edit: 01/18/2022
//
// ----------------------------------------------------------------------------- //

#include "F28x_Project.h"       // Device Header File and Examples Include File

// Initialize the variables for output
Uint16 resultsIndex;            // Variable for holding the results buffer
Uint16 mmSpeed = 0x000;         // {-600, 600} [rad/s] - Motor Mechanical Speed rad/s | {0.0 V, 3.3 V}
Uint16 maCurrent = 0x000;       // {0, 100} [A] - Motor Armature Current in A | {0.0 V, 3.3 V}

// Initialize the variables for input
Uint16 LoadTorque = 0xFFF;      // {-0.2, 0.2} [Nm] - Load Torque in Nm | {0.0 V, 3.0 V}
Uint16 DutyCycle = 0x7FF;       // {0, 100} [%]  - Load Torque in % | {0.0 V, 3.0 V} --> 0x7FF = 50

// Definitions for PWM generation
#define PWM1_PERIOD 0xC350          // PWM1 frequency = 2kHz
#define PWM1_CMPR25 PWM1_PERIOD>>2  // PWM1 initial duty cycle = 25%

// Function Prototypes
void ConfigureADC(void);            // Write ADC configurations and power up the ADC for both ADC A and ADC C
void ConfigureDAC(void);            // Write DAC configurations and power up the DAC for both DAC A and DAC C
void SetupADCEpwm(void);            // Select the channels to convert and end of conversion flag for the Pulse Width Modulator
void InitEPwm1(void);               // Configure ePWM module 1
void InitEPwm2(void);               // Configure ePWM module 2
void InitEPwm5(void);               // Configure ePWM module 5
interrupt void adca1_isr(void);     // ADC interrupt service routine

// Variables
Uint16 period1 = PWM1_PERIOD;       // PWM1 period = 2kHz PWM
Uint16 dutyCycle1 = PWM1_CMPR25;    // PWM1 duty cycle = 25%
Uint16 dutyCycle5 = PWM1_CMPR25;    // PWM5 duty cycle = 25%
Uint16 phaseOffset5 = 0;            // PWM5 phase offset = 0

// Buffers for storing ADC conversion results
#define RESULTS_BUFFER_SIZE 256             // Set the max buffer size of the results to 256 bits
Uint16 AdcaResults[RESULTS_BUFFER_SIZE];    // Allocate memory for the Adca registers
Uint16 AdccResults[RESULTS_BUFFER_SIZE];    // Allocate memory for the Adcc registers
Uint16 pretrig = 0;                         // The pretrig variable helps identifies a low-to-high transition on PWM1A
Uint16 trigger = 0;                         // The trigger variable helps identifies a low-to-high transition on PWM1A

// Beginning the main section of code
void main(void)
{
    InitSysCtrl();                                  // Initialize System Control
    EALLOW;                                         // (Bit 6) — Emulation access enable bit - Enable access to emulation and other protected registers
    ClkCfgRegs.PERCLKDIVSEL.bit.EPWMCLKDIV = 1;     // Enable Clock Configure Registers
    EDIS;                                           // Using EDIS to clear the EALLOW

    // Initialize GPIO
    InitGpio();         // Configure default GPIO
    InitEPwm1Gpio();    // Configure EPWM1 GPIO pins
    InitEPwm5Gpio();    // Configure EPWM5 GPIO pins

    EALLOW;             // (Bit 6) — Emulation access enable bit - Enable access to emulation and other protected registers
    GpioCtrlRegs.GPADIR.bit.GPIO31 = 1;         // Drives LED LD2 on controlCARD
    EDIS;                                       // Using EDIS to clear the EALLOWs
    GpioDataRegs.GPADAT.bit.GPIO31 = 1;         // Turn off LED

    // Map ISR functions
    EALLOW;             // (Bit 6) — Emulation access enable bit - Enable access to emulation and other protected registers
    PieVectTable.ADCA1_INT = &adca1_isr;        // Function for ADCA interrupt 1
    EDIS;                                       // Using EDIS to clear the EALLOW

    // Configure the ADC and power it up
    ConfigureADC();     // Write ADC configurations and power up the ADC for both ADC A and ADC C

    // Setup the ADC for ePWM triggered conversions on channel 0
    SetupADCEpwm();     // Select the channels to convert and end of conversion flag

    // Initialize ePWM modules
    InitEPwm1();        // Initialize ePWM module 1
    InitEPwm2();        // Initialize ePWM module 2
    InitEPwm5();        // Initialize ePWM module 5

    // Configure DAC registers
    ConfigureDAC();     // Call the ConfigureDAC function

    // Initialize results buffers - Adca and Adcc Results are 0 as long as resultsIndex is less than RESULTS_BUFFER_SIZE
    for(resultsIndex = 0; resultsIndex < RESULTS_BUFFER_SIZE; resultsIndex++)
    {
        AdcaResults[resultsIndex] = 0;      // Set AdcaResults at current index equal to 0
        AdccResults[resultsIndex] = 0;      // Set AdcaResults at current index equal to 0
    }
    resultsIndex = 0;       // Reset the resultsIndex variable back to 0 for the next iteration

    // Enable global interrupts and higher priority real-time debug events
    IER |= M_INT1;          // Enable group 1 interrupts
    EINT;                   // Enable Global interrupt INTM
    ERTM;                   // Enable Global real time interrupt DBGM

    PieCtrlRegs.PIEIER1.bit.INTx1 = 1;      // Enable PIE interrupt

    // Sync ePWM
    EALLOW;                                 // (Bit 6) — Emulation access enable bit - Enable access to emulation and other protected registers
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;   // Enable CPU System Registers

    // Start ePWM
    EPwm2Regs.ETSEL.bit.SOCAEN = 1;             // Enable SOCA
    EPwm2Regs.TBCTL.bit.CTRMODE = 0;            // Un-freeze and enter up-count mode

    do {                                        // Infinite do-while loop
        GpioDataRegs.GPADAT.bit.GPIO31 = 0;     // Turn on LED
        DELAY_US(1000 * 500);                   // ON delay
        GpioDataRegs.GPADAT.bit.GPIO31 = 1;     // Turn off LED
        DELAY_US(1000 * 500);                   // OFF delay
        DacaRegs.DACVALS.all = LoadTorque;      // Send Load Torque to OPAL-RT
        DacbRegs.DACVALS.all = DutyCycle;       // Send Duty Cycle to OPAL-RT
    } while(1);                                 // Infinite do-while loop
}

void ConfigureDAC(void)                         // Configure specifc DAC pins needed to utilize communication with I/O
{
    EALLOW;                                     // (Bit 6) — Emulation access enable bit - Enable access to emulation and other protected registers
    DacaRegs.DACCTL.bit.DACREFSEL = 1;          // Use ADC references (HSEC Pin 09)
    DacaRegs.DACCTL.bit.LOADMODE = 0;           // Load on next SYSCLK
    DacaRegs.DACOUTEN.bit.DACOUTEN = 1;         // Enable DAC
    DacbRegs.DACCTL.bit.DACREFSEL = 1;          // Use ADC references (HSEC Pin 11)
    DacbRegs.DACCTL.bit.LOADMODE = 0;           // Load on next SYSCLK
    DacbRegs.DACOUTEN.bit.DACOUTEN = 1;         // Enable DAC
    EDIS;                                       // Using EDIS to clear the EALLOW
}

// Write ADC configurations and power up the ADC for both ADC A and ADC C
void ConfigureADC(void)
{
    EALLOW;                                     // (Bit 6) — Emulation access enable bit - Enable access to emulation and other protected registers
    // ADC-A
    AdcaRegs.ADCCTL2.bit.PRESCALE = 6;          // Set ADCCLK divider to /4
    AdcaRegs.ADCCTL2.bit.RESOLUTION =  0;       // 12-bit resolution
    AdcaRegs.ADCCTL2.bit.SIGNALMODE = 0;        // Single-ended channel conversions (12-bit mode only)
    AdcaRegs.ADCCTL1.bit.INTPULSEPOS = 1;       // Set pulse positions to late
    AdcaRegs.ADCCTL1.bit.ADCPWDNZ = 1;          // Power up the ADC

    // ADC-C
    AdccRegs.ADCCTL2.bit.PRESCALE = 6;          // Set ADCCLK divider to /4
    AdccRegs.ADCCTL2.bit.RESOLUTION =  0;       // 12-bit resolution RESOLUTION_12BIT;
    AdccRegs.ADCCTL2.bit.SIGNALMODE = 0;        // Single-ended channel conversions (12-bit mode only)
    AdccRegs.ADCCTL1.bit.INTPULSEPOS = 1;       // Set pulse positions to late
    AdccRegs.ADCCTL1.bit.ADCPWDNZ = 1;          // Power up the ADC
    EDIS;                                       // Using EDIS to clear the EALLOW

    DELAY_US(1000);                             // Delay for 1ms to allow ADC time to power up
}


void SetupADCEpwm(void)
{
    // Select the channels to convert and end of conversion flag
    EALLOW;                                     // (Bit 6) — Emulation access enable bit - Enable access to emulation and other protected registers
    AdcaRegs.ADCSOC0CTL.bit.CHSEL = 2;          // SOC0 will convert pin A0 (HSEC Pin 15)
    AdcaRegs.ADCSOC0CTL.bit.ACQPS = 14;         // Sample window is 100 SYSCLK cycles
    AdcaRegs.ADCSOC0CTL.bit.TRIGSEL = 7;        // Trigger on ePWM2 SOCA/C
    AdcaRegs.ADCINTSEL1N2.bit.INT1SEL = 0;      // End of SOC0 will set INT1 flag
    AdcaRegs.ADCINTSEL1N2.bit.INT1E = 1;        // Enable INT1 flag
    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;      // Make sure INT1 flag is cleared

    // Also setup ADC-C3 in this example
    AdccRegs.ADCSOC0CTL.bit.CHSEL = 3;          // SOC0 will convert pin C3 (HSEC Pin 33)
    AdccRegs.ADCSOC0CTL.bit.ACQPS = 14;         // Sample window is 100 SYSCLK cycles
    AdccRegs.ADCSOC0CTL.bit.TRIGSEL = 7;        // Trigger on ePWM2 SOCA/C
    EDIS;                                       // Using EDIS to clear the EALLOW
}


void InitEPwm1(void)
{
   // Setup TBCLK
   EPwm1Regs.TBCTL.bit.CTRMODE = 0;             // Count up
   EPwm1Regs.TBPRD = period1;                   // Set timer period
   EPwm1Regs.TBCTL.bit.PHSEN = 0;               // Disable phase loading
   EPwm1Regs.TBPHS.bit.TBPHS = 0x0000;          // Phase is 0
   EPwm1Regs.TBCTR = 0x0000;                    // Clear counter
   EPwm1Regs.TBCTL.bit.HSPCLKDIV = 1;           // Clock ratio to SYSCLKOUT
   EPwm1Regs.TBCTL.bit.CLKDIV = 0;              // Clock Div is 0
   EPwm1Regs.TBCTL.bit.SYNCOSEL = 1;            // SYNC output on CTR = 0

   // Setup shadow register load on ZERO
   EPwm1Regs.CMPCTL.bit.SHDWAMODE = 0;
   EPwm1Regs.CMPCTL.bit.SHDWBMODE = 0;
   EPwm1Regs.CMPCTL.bit.LOADAMODE = 0;
   EPwm1Regs.CMPCTL.bit.LOADBMODE = 0;

   // Set Compare values
   EPwm1Regs.CMPA.bit.CMPA = dutyCycle1;        // Set compare A value

   // Set actions
   EPwm1Regs.AQCTLA.bit.ZRO = 2;                // Set PWM1A on Zero
   EPwm1Regs.AQCTLA.bit.CAU = 1;                // Clear PWM1A on event A, up count
}


void InitEPwm2(void)
{
    EALLOW;         // (Bit 6) — Emulation access enable bit - Enable access to emulation and other protected registers
    // Assumes ePWM clock is already enabled
    EPwm2Regs.TBCTL.bit.CTRMODE = 3;            // Freeze counter
    EPwm2Regs.TBCTL.bit.HSPCLKDIV = 0;          // TBCLK pre-scaler = /1
    EPwm2Regs.TBPRD = 0x07D0;                   // Set period to 2000 counts (50kHz)
    EPwm2Regs.ETSEL.bit.SOCAEN  = 0;            // Disable SOC on A group
    EPwm2Regs.ETSEL.bit.SOCASEL = 2;            // Select SOCA on period match
    EPwm2Regs.ETSEL.bit.SOCAEN = 1;             // Enable SOCA
    EPwm2Regs.ETPS.bit.SOCAPRD = 1;             // Generate pulse on 1st event
    EDIS;                                       // Using EDIS to clear the EALLOW
}


void InitEPwm5(void)
{
   // Setup TBCLK
   EPwm5Regs.TBCTL.bit.CTRMODE = 0;             // Count up
   EPwm5Regs.TBPRD = PWM1_PERIOD;               // Same period as PWM1
   EPwm5Regs.TBCTL.bit.PHSEN = 1;               // Enable phase loading
   EPwm5Regs.TBPHS.bit.TBPHS = phaseOffset5;    // Phase
   EPwm5Regs.TBCTR = 0x0000;                    // Clear counter
   EPwm5Regs.TBCTL.bit.HSPCLKDIV = 1;           // Clock ratio to SYSCLKOUT
   EPwm5Regs.TBCTL.bit.CLKDIV = 0;

   // Setup shadow register load on ZERO
   EPwm5Regs.CMPCTL.bit.SHDWAMODE = 0;
   EPwm5Regs.CMPCTL.bit.SHDWBMODE = 0;
   EPwm5Regs.CMPCTL.bit.LOADAMODE = 0;
   EPwm5Regs.CMPCTL.bit.LOADBMODE = 0;

   // Set Compare values
   EPwm5Regs.CMPA.bit.CMPA = dutyCycle5;        // Set compare A value

   // Set actions
   EPwm5Regs.AQCTLA.bit.ZRO = 2;                // Set PWM1A on Zero
   EPwm5Regs.AQCTLA.bit.CAU = 1;                // Clear PWM1A on event A, up count
}


interrupt void adca1_isr(void)
{
    // Read the ADC result and store in circular buffer
    if (trigger != 0)
    {
        AdcaResults[resultsIndex] = AdcaResultRegs.ADCRESULT0;      // Update Adca results buffer
        AdccResults[resultsIndex++] = AdccResultRegs.ADCRESULT0;    // Update Adcc results buffer
        if(RESULTS_BUFFER_SIZE <= resultsIndex)
        {
            resultsIndex = 0;       // Set ResultIndex to 0
            pretrig = 0;            // Set pretrig to 0
            trigger = 0;            // Set trigger to 0

            // Update PWMs
            EPwm1Regs.TBPRD = period1;
            EPwm1Regs.CMPA.bit.CMPA = dutyCycle1;
            EPwm5Regs.TBPRD = period1;
            EPwm5Regs.CMPA.bit.CMPA = dutyCycle5;
            EPwm5Regs.TBPHS.bit.TBPHS = phaseOffset5;
        }
    }

    // This code identifies a low-to-high transition on PWM1A so the results buffer always starts
    // on a rising edge.  This makes phase observations between PWM1 and PWM5 clearer.
    else if (pretrig != 0)
    {
        trigger |= GpioDataRegs.GPADAT.bit.GPIO0;
    }
    else pretrig = GpioDataRegs.GPADAT.bit.GPIO0 - 1;

    // Return from interrupt
    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;      // Clear ADC INT1 flag
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;     // Acknowledge PIE group 1 to enable further interrupts
}

// ----------------------------------------------------------------------------- //
// End of file
// ----------------------------------------------------------------------------- //
