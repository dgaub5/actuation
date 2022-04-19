    // ----------------------------------------------------------------------------- //
    // Beginning of File
    //
    // File: actuation_cpu01.c
    /*
    // File Description:
    // This file contains the code for HIL communication with the OPAL-RT to the
    // TI-Microcontoller (This Board). Here this code demonstrates two inputs and two outputs.
    //
    // Last Edit: 02/28/2022
    //
    //
    // Input Variables:
    // Motor Mechanical Speed {-600, 600} [rad/s] | {0.0, 3.3} [V]
    // Motor Armature Current {-2.5, 2.5} [A] | {0.0, 3.3} [V]
    // -----------------------------------------------------------------------------
     */

    #include "F28x_Project.h"       // Device Header File and Examples Include File
    #include <math.h>

    // Definitions for PWM generation
    #define PWM1_PERIOD 0xC350          // PWM1 frequency = 50 kHz
    #define PWM1_CMPR25 PWM1_PERIOD>>2  // PWM1 initial duty cycle = 25%


    // Output Variables
    Uint16 dacOutput;               // Initialize variable for the DAC Outputs - not used (can delete?)
    volatile float32 LoadTorque;    // {-0.2, 0.2} [Nm] - Load Torque in Nm | {0.0 V, 3.0 V}
    volatile float32 DutyCycle;     // {0, 100}    [%]  - Load Torque in %  | {0.0 V, 3.0 V}
    Uint16 resultsIndex;            // Initialization for the results index - this is the array pointer for ADC conversions; resultsIndex increments to place new value in adjacent cell, and reset when array is full


    // PWM Variables
    Uint16 period1 = PWM1_PERIOD;       // PWM1 period = 50kHz PWM
    Uint16 dutyCycle1 = PWM1_CMPR25;    // PWM1 duty cycle = 25%
    Uint16 dutyCycle5 = PWM1_CMPR25;    // PWM5 duty cycle = 25%
    Uint16 phaseOffset5 = 0;            // PWM5 phase offset = 0

    // Function Prototypes
    void ConfigureADC(void);            // Write ADC configurations and power up the ADC for both ADC-A and ADC-C
    void ConfigureDAC(void);            // Write DAC configurations and power up the DAC for both DAC A and DAC C
    void ConfigureEPWM(void);           // Select the channels to convert and end of conversion flag for the Pulse Width Modulator
    void SetupADCEpwm(void);            // Call the ADCE pulse width modulator for GPIO pins
    void InitEPwm1(void);               // Configure ePWM module 1
    void InitEPwm2(void);               // Configure ePWM module 2
    void InitEPwm5(void);               // Configure ePWM module 5
    void PI_Loop(void);
    interrupt void adca1_isr(void);     // ADC interrupt service routine

    // Buffers for storing ADC conversion results
    #define RESULTS_BUFFER_SIZE 256             // Set the max buffer size of the results to 256 bits
    float mmSpeed;       // Allocate memory for the ADC-A registers (motor speed)
    float maCurrent;     // Allocate memory for the ADC-C registers (armature current)
    Uint16 resultsIndex;                        // Initialize the Results Index
    Uint16 pretrig = 0;                         // Set the value of pretrig
    Uint16 trigger = 0;                         // Set the value of trigger

    // Variables for PI_Loop - Equation: d = Kp*e + Ki*(((e0+e1)/2)*delta)
    float ideal_Speed = 0;  // Simulated speed of motors on test bed with a threshold of 0-1800
    float curr_Speed    = 0;    // Speed value gathered from AdcaResults on each interval
    float d           = 0;    // Final result from PI loop
    float Kp          = 0.05;    // Proportional gain - Value for Kp in PI controller equation
    float Ki          = 0.05;    // Integral gain - Value for Ki in PI controller equation
    float e_dt        = 0;    // Change in e, integral part
    float e           = 0;    // Error value total
    float e0          = 0;    // Error value gathering 0 (Previous interval)
    float e1          = 0;    // Error value gathering 1 (Current interval)
    float delta       = 0.00002;   // Change in time for each interval (1000us = 1ms) for the cycles of data gathering
    float total_err   = 0;
    float temp        = 0;
    // Setting up data transfer for --gen_profile_info code coverage
    extern void _TI_stop_pprof_collection(void);

    // Beginning of the main section of code
    void main(void)
    {
        InitSysCtrl();                  // Initialize System Control
        EALLOW;                         // (Bit 6) — Emulation access enable bit - Enable access to emulation and other protected registers
        ClkCfgRegs.PERCLKDIVSEL.bit.EPWMCLKDIV = 1; //Enable Clock Configure Registers
        EDIS;                           // Using EDIS to clear the EALLOW

        // Initialize GPIO
        InitGpio();         // Configure default GPIO
        InitEPwm1Gpio();    // Configure EPWM1 GPIO pins
        InitEPwm5Gpio();    // Configure EPWM5 GPIO pins

        EALLOW;                                     // (Bit 6) — Emulation access enable bit - Enable access to emulation and other protected registers
        GpioCtrlRegs.GPADIR.bit.GPIO31 = 1;         // Drives LED LD2 on controlCARD
        EDIS;                                       // Using EDIS to clear the EALLOWs
        GpioDataRegs.GPADAT.bit.GPIO31 = 1;         // Turn off LED

        DINT;               // Clear all interrupts and initialize PIE vector table
        InitPieCtrl();      // Initialize the PIE Control
        IER = 0x0000;       // Set IER to 0
        IFR = 0x0000;       // Set IFR to 0
        InitPieVectTable(); // Initialize the Pie Vector Table

        // Map ISR functions
        EALLOW;                                      // (Bit 6) — Emulation access enable bit - Enable access to emulation and other protected registers
        PieVectTable.ADCA1_INT = &adca1_isr;         // Function for ADCA interrupt 1
        EDIS;               // Using EDIS to clear the EALLOW

        ConfigureADC();     // Configure the ADC and power it up

        SetupADCEpwm();     // Setup the ADC for ePWM triggered conversions on channel 0

        // Initialize ePWM modules
        InitEPwm1();        // Initialize ePWM 1
        InitEPwm2();        // Initialize ePWM 2
        InitEPwm5();        // Initialize ePWM 5

        ConfigureDAC();     // Configure DACs

        // Initialize results buffers
//        for(resultsIndex = 0; resultsIndex < RESULTS_BUFFER_SIZE; resultsIndex++)
//        {
//            mmSpeed[resultsIndex] = 0;      // Set ADC-A current results index to 0
//            maCurrent[resultsIndex] = 0;      // Set ADC-C current results index to 0
//        }
//        resultsIndex = 0;   // Reset the results index counter

        // Enable global interrupts and higher priority real-time debug events
        IER |= M_INT1;          // Enable group 1 interrupts
        EINT;                   // Enable Global interrupt INTM
        ERTM;                   // Enable Global real time interrupt DBGM

        PieCtrlRegs.PIEIER1.bit.INTx1 = 1;      // Enable PIE interrupt

        // Sync ePWM
        EALLOW;                                 // (Bit 6) — Emulation access enable bit - Enable access to emulation and other protected registers
        CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;   // Set CPU System Registers to active bit

        // Start ePWM
        EPwm2Regs.ETSEL.bit.SOCAEN = 1;             // Enable SOCA
        EPwm2Regs.TBCTL.bit.CTRMODE = 0;            // Un-freeze and enter up-count mode

        // Infinite loop to continuously grab values being inputed
        do {


           // Set DutyCycle to volatage ratio from PI loop (0-100)
           // GpioDataRegs.GPADAT.bit.GPIO31 = 0;     // Turn on LED
           // DELAY_US(1000 * 500);                   // ON delay
           // GpioDataRegs.GPADAT.bit.GPIO31 = 1;     // Turn off LED


        } while(1);
    }

    // Write DAC configurations and power up the DAC for both DAC-A and DAC-C
    void ConfigureDAC(void)
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

    // Write ADC configurations and power up the ADC. ADC-A (mmSpeed), ADC-B (maCurrent), ADC-C (DutyCycle), and ADC-D (LoadTorque)
    void ConfigureADC(void)
    {
        EALLOW;     // (Bit 6) — Emulation access enable bit - Enable access to emulation and other protected registers

        // ADC-A -- mmSpeed
        AdcaRegs.ADCCTL2.bit.PRESCALE = 6;          // Set ADCCLK divider to /4
        AdcaRegs.ADCCTL2.bit.RESOLUTION =  0;       // 12-bit resolution
        AdcaRegs.ADCCTL2.bit.SIGNALMODE = 0;        // Single-ended channel conversions (12-bit mode only)
        AdcaRegs.ADCCTL1.bit.INTPULSEPOS = 1;       // Set pulse positions to late
        AdcaRegs.ADCCTL1.bit.ADCPWDNZ = 1;          // Power up the ADC

        // ADC-C -- maCurrent
        AdccRegs.ADCCTL2.bit.PRESCALE = 6;          // Set ADCCLK divider to /4
        AdccRegs.ADCCTL2.bit.RESOLUTION =  0;       // 12-bit resolution RESOLUTION_12BIT;
        AdccRegs.ADCCTL2.bit.SIGNALMODE = 0;        // Single-ended channel conversions (12-bit mode only)
        AdccRegs.ADCCTL1.bit.INTPULSEPOS = 1;       // Set pulse positions to late
        AdccRegs.ADCCTL1.bit.ADCPWDNZ = 1;          // Power up the ADC

        // ADC-B -- DutyCycle
        AdcbRegs.ADCCTL2.bit.PRESCALE = 6;          // Set ADCCLK divider to /4
        AdcbRegs.ADCCTL2.bit.RESOLUTION =  0;       // 12-bit resolution RESOLUTION_12BIT;
        AdcbRegs.ADCCTL2.bit.SIGNALMODE = 0;        // Single-ended channel conversions (12-bit mode only)
        AdcbRegs.ADCCTL1.bit.INTPULSEPOS = 1;       // Set pulse positions to late
        AdcbRegs.ADCCTL1.bit.ADCPWDNZ = 1;          // Power up the ADC

        // ADC-D -- LoadTorque
        AdcdRegs.ADCCTL2.bit.PRESCALE = 6;          // Set ADCCLK divider to /4
        AdcdRegs.ADCCTL2.bit.RESOLUTION =  0;       // 12-bit resolution RESOLUTION_12BIT;
        AdcdRegs.ADCCTL2.bit.SIGNALMODE = 0;        // Single-ended channel conversions (12-bit mode only)
        AdcdRegs.ADCCTL1.bit.INTPULSEPOS = 1;       // Set pulse positions to late
        AdcdRegs.ADCCTL1.bit.ADCPWDNZ = 1;          // Power up the ADC
        EDIS;                                       // Using EDIS to clear the EALLOW

        DELAY_US(1000);                             // Delay for 1ms to allow ADC time to power up
    }

    // Function to set up the ADC-A (mmSpeed), ADC-B (maCurrent), ADC-C (DutyCycle), and ADC-D (LoadTorque) using EPWM2 as trigger
    void SetupADCEpwm(void)
    {
        // Select the channels to convert and end of conversion flag
        EALLOW;                                     // (Bit 6) — Emulation access enable bit - Enable access to emulation and other protected registers
        // Setup ADC-A2 + interrupt flag
        AdcaRegs.ADCSOC0CTL.bit.CHSEL = 2;          // SOC0 will convert pin A2 (HSEC Pin 15)
        AdcaRegs.ADCSOC0CTL.bit.ACQPS = 14;         // Sample window is 100 SYSCLK cycles
        AdcaRegs.ADCSOC0CTL.bit.TRIGSEL = 7;        // Trigger on ePWM2 SOCA/C
        AdcaRegs.ADCINTSEL1N2.bit.INT1SEL = 0;      // End of SOC0 will set INT1 flag
        AdcaRegs.ADCINTSEL1N2.bit.INT1E = 1;        // Enable INT1 flag
        AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;      // Make sure INT1 flag is cleared

        // Setup ADC-C3
        AdccRegs.ADCSOC0CTL.bit.CHSEL = 3;          // SOC0 will convert pin C3 (HSEC Pin 33)
        AdccRegs.ADCSOC0CTL.bit.ACQPS = 14;         // Sample window is 100 SYSCLK cycles
        AdccRegs.ADCSOC0CTL.bit.TRIGSEL = 7;        // Trigger on ePWM2 SOCA/C

        // Setup ADC-B1
        AdcbRegs.ADCSOC0CTL.bit.CHSEL = 0;          // SOC0 will convert pin B0 (HSEC Pin 12)
        AdcbRegs.ADCSOC0CTL.bit.ACQPS = 14;         // Sample window is 100 SYSCLK cycles
        AdcbRegs.ADCSOC0CTL.bit.TRIGSEL = 7;        // Trigger on ePWM2 SOCA/C

        // Setup ADC-D3
        AdcdRegs.ADCSOC0CTL.bit.CHSEL = 3;          // SOC0 will convert pin D3 (HSEC Pin 36)
        AdcdRegs.ADCSOC0CTL.bit.ACQPS = 14;         // Sample window is 100 SYSCLK cycles
        AdcdRegs.ADCSOC0CTL.bit.TRIGSEL = 7;        // Trigger on ePWM2 SOCA/C
        EDIS;                                       // Using EDIS to clear the EALLOW
    }

    // Function to initialize the Electronic Pulse Width Modulator 1. This PWM functions as a timer for ADC-A
    void InitEPwm1(void)
    {
       // Setup TBCLK
       EPwm1Regs.TBCTL.bit.CTRMODE = 0;             // Count up
       EPwm1Regs.TBPRD = period1;                   // Set timer period (20us)
       EPwm1Regs.TBCTL.bit.PHSEN = 0;               // Disable phase loading
       EPwm1Regs.TBPHS.bit.TBPHS = 0x0000;          // Phase is 0
       EPwm1Regs.TBCTR = 0x0000;                    // Clear counter
       EPwm1Regs.TBCTL.bit.HSPCLKDIV = 1;           // Clock ratio to SYSCLKOUT
       EPwm1Regs.TBCTL.bit.CLKDIV = 0;              // Set the clock division to 0
       EPwm1Regs.TBCTL.bit.SYNCOSEL = 1;            // SYNC output on CTR = 0

       // Setup shadow register load on ZERO
       EPwm1Regs.CMPCTL.bit.SHDWAMODE = 0;          // Set Shadow A Mode to 0
       EPwm1Regs.CMPCTL.bit.SHDWBMODE = 0;          // Set Shadow B Mode to 0
       EPwm1Regs.CMPCTL.bit.LOADAMODE = 0;          // Set Load A Mode to 0
       EPwm1Regs.CMPCTL.bit.LOADBMODE = 0;          // Set Load B Mode to 0

       // Set Compare values
       EPwm1Regs.CMPA.bit.CMPA = dutyCycle1;        // Set compare A value

       // Set actions
       EPwm1Regs.AQCTLA.bit.ZRO = 2;                // Set PWM1A on Zero
       EPwm1Regs.AQCTLA.bit.CAU = 1;                // Clear PWM1A on event A, up count
    }

    // Function to initialize the Electronic Pulse Width Modulator 2. This PWM functions as the triggering mechanism for ADC SOC for all current ADCs
    void InitEPwm2(void)
    {
        EALLOW;                                     // (Bit 6) — Emulation access enable bit - Enable access to emulation and other protected registers
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

    // Function to initialize the Electronic Pulse Width Modulator 5. This PWM functions as a timer for ADC-C
    void InitEPwm5(void)
    {
       // Setup TBCLK
       EPwm5Regs.TBCTL.bit.CTRMODE = 0;             // Count up
       EPwm5Regs.TBPRD = period1;                   // Set timer period (20us)
       EPwm5Regs.TBCTL.bit.PHSEN = 1;               // Enable phase loading
       EPwm5Regs.TBPHS.bit.TBPHS = phaseOffset5;    // PWM5 phase offset = 0
       EPwm5Regs.TBCTR = 0x0000;                    // Clear counter
       EPwm5Regs.TBCTL.bit.HSPCLKDIV = 1;           // Clock ratio to SYSCLKOUT
       EPwm5Regs.TBCTL.bit.CLKDIV = 0;              // Set Clock Division to 0

       // Setup shadow register load on ZERO
       EPwm5Regs.CMPCTL.bit.SHDWAMODE = 0;          // Set Shadow A Mode to 0
       EPwm5Regs.CMPCTL.bit.SHDWBMODE = 0;          // Set Shadow B Mode to 0
       EPwm5Regs.CMPCTL.bit.LOADAMODE = 0;          // Set Load A Mode to 0
       EPwm5Regs.CMPCTL.bit.LOADBMODE = 0;          // Set Load B Mode to 0

       // Set Compare values
       EPwm5Regs.CMPA.bit.CMPA = dutyCycle5;        // Set compare A value

       // Set actions
       EPwm5Regs.AQCTLA.bit.ZRO = 2;                // Set PWM1A on Zero
       EPwm5Regs.AQCTLA.bit.CAU = 1;                // Clear PWM1A on event A, up count
    }

    //Creating function for PI Loop
    void PI_Loop(void)
    {
        // ideal_Speed = 300;                  // Testing with a 2 V input which equals 290.36 mm_Speed
        e = ideal_Speed - mmSpeed;       // Current iteration error value calculation
        e1 = e;                                // Store current value for error in iteration
        e_dt = (((e0+e1)/2)*delta);            // Change in error value
        total_err += e_dt;
        if (total_err > 2000) total_err = 2000;                  // Saturation - Duty cycle should be between 0-100
        if (total_err < -2000) total_err = -2000;
        d = (Kp*e) + (Ki*total_err);                // Final result from PI loop is stored in d
        if (d > 100) d = 100;                  // Saturation - Duty cycle should be between 0-100
        if (d < 0) d = 0;
        e0 = e;                                // Store previous error result for the next iteration
        DutyCycle = d;
    }

    // Interrupt Service Routine for ADC conversion. Triggered from EPWM2 period match using SOCA every 20us.
    interrupt void adca1_isr(void)
    {
        // Read the ADC result and store in circular buffer
        if (trigger != 0)
        {
            mmSpeed = 0.293 * (AdcaResultRegs.ADCRESULT0-2048);       // Store current value of ADC-A in array, scaled to {-600 to 600} [rad/s]
            ideal_Speed = (float)0.293 * (float)(AdcbResultRegs.ADCRESULT0-(float)2048);                                      // Update DutyCycle with ADC-B results
            maCurrent = 0.00122 * (AdccResultRegs.ADCRESULT0-2048);   // Store current value of ADC-C in array, scaled to {-2.5 to 2.5} [A]
            LoadTorque = AdcdResultRegs.ADCRESULT0;                                     // Update LoadTorque with ADC-D results

             PI_Loop();

             // Send Load Torque and Duty Cycle to Opal
           DacaRegs.DACVALS.all = LoadTorque;      // Set the value of the DAC-A Registers to Load Torque
           temp = (float)DutyCycle * (float)40.96;
           DacbRegs.DACVALS.all = floor(temp);       // Set the value of the DAC-B Registers to Duty Cycle
           //_TI_stop_pprof_collection();            // Add a call to _TI_stop_pprof_collection at the point in which you wish to transfer the coverage data



            if(RESULTS_BUFFER_SIZE <= resultsIndex)
            /* Reset resultsIndex once ADC arrays are full
             * Reset pre-trigger and ISR trigger to 0 once arrays are full
             * Update PWM periods and duty cycles, but this may be unnecessary, as the values do not change (?)
             */
            {
                resultsIndex = 0;                         // Reset results index to 0
                pretrig = 0;                              // Reset pretrig to 0
                trigger = 0;                              // Reset ISR trigger to 0

                // Update PWMs -- Shouldn't be necessary (?) try and remove these
                EPwm1Regs.TBPRD = period1;                // Set the EPwm period
                EPwm1Regs.CMPA.bit.CMPA = dutyCycle1;     // Set the EPwm duty cycle
                EPwm5Regs.TBPRD = period1;                // Set the EPwm period
                EPwm5Regs.CMPA.bit.CMPA = dutyCycle5;     // Set the EPwn duty cycle
                EPwm5Regs.TBPHS.bit.TBPHS = phaseOffset5; // Set the phase offset
            }
        }

        /* This code identifies a low-to-high transition on PWM1A so the results buffer always starts
         * on a rising edge.  This makes phase observations between PWM1 and PWM5 clearer.
         */

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
