/*
Description: this file calls functions in the powerConverterDLL library; its purpose is to
verify that the functions in the library *can* be called (not perform full unit tests).

Written by:
James Cale, Ph.D.
Colorado State University
Contact: jcale@colostate.edu

Revision Notes:
- 12 July 2021: added call of abcs2qd0() [J.C.]
 */

#include <stdio.h>
#include <math.h>
#include "powerConverterDLL.h"

#define PI 3.1415926535

int main() {
    
    double t     = 0;       // time variable [s]
    double dt    = 50e-6;   // time step of function loop [s]
    double Vs    = 100;     // voltage magnitude (rms) of 3-phase voltage source [V]
    double fe    = 100;      // frequency of electrical source [Hz]
    double Te    = 0;       // time period of electrical source [s]
    double we    = 0;       // angular radial frequency of electrical source [rad/s]
    double v_as  = 0;       // a-phase voltage in stationary ref. frame [V]
    double v_bs  = 0;       // b-phase voltage in stationary ref. frame [V]
    double v_cs  = 0;       // c-phase voltage in stationary ref. frame [V]
    double x_as  = 0;       // a-phase voltage in stationary ref. frame (check) [V]
    double x_bs  = 0;       // b-phase voltage in stationary ref. frame (check) [V]
    double x_cs  = 0;       // c-phase voltage in stationary ref. frame (check) [V]
    double vq    = 0;       // voltage in q variable [V]
    double vd    = 0;       // voltage in d variable [V]
    double vz    = 0;       // voltage in zero variable [V]
    double v_alpha = 0;     // voltage in alpha variable [V]
    double v_beta  = 0;     // voltage in beta variable [V]
    double theta = 0;       // angle of arbitrary reference frame [rad]

    // initalize variable for filter
    double x     = 0;       // signal to be filtered
    double xf    = 0;       // filtered signal
    double xf0   = 0;       // filtered signal, previous iteration
    double fc    = 500;     // filter cutoff frequency [Hz]

    // initalize variables for integrator
    double fval  = 0;       // function value to be integrated
    double fval0 = 0;       // function value to be integrated, previous
    double f_int = 0;       // cumulative integral of function
 
    // initalize variables for PLL
    double fe_est = 0;      // frequency to be identified by PLL [Hz]
    double omegae_est = 0;  // estimated radial frequency of PLL [rad/s]
    double thetae_est = 0;  // estimated angular position of PLL [rad]
    double Kp_pll = 20;     // proportional term for PI block in PLL
    double Ki_pll = 1;      // integral term for PI block in PLL
    double vd_err = 0;      // error in vd variable for PLL block
    double vd_err_int = 0;  // integral of vd variable error for PLL block

    // compute supplementary variables
    Te = 1.0 / fe;
    
    for ( t = 0; t < 2.0 * Te ; t += dt ) // <- simulates main microprocessor loop
    {
        // compute voltage set (simulated voltage measurements into microprocessor)
        we   = 2.0 * PI * fe;
        v_as = sqrt(2.0) * Vs * cos( we * t );
        v_bs = sqrt(2.0) * Vs * cos( we * t - 2.0 * PI / 3.0 );
        v_cs = sqrt(2.0) * Vs * cos( we * t + 2.0 * PI / 3.0 );
        
        // compute arbitrary reference frame angle
        theta = we * t; // <- transformation to synchronous ref. frame
        
        // transform voltages to arbitrary ref. frame
        abcs2qd0( theta, v_as, v_bs, v_cs, &vq, &vd, &vz );
 
        // transform voltages from arbitrary ref. frame to stationary ref. frame
        qd02abcs( theta, &x_as, &x_bs, &x_cs, vq, vd, vz );
 
        // transform voltages to alpha-beta variables
        abcs2alphabeta( v_as, v_bs, v_cs, &v_alpha, &v_beta );
        
        // implement low-pass filter
        folpf( x, &xf, dt, fc );

        // implement integration function
        fval = 1.0 - exp(-0.1*t); // <- simulates some measurement into the microprocessor
        integrator( fval, fval0, &f_int, dt );
        fval0 = fval;
 
        // implement phase-lock loop
        pll( v_as, v_bs, v_cs, &fe_est, &thetae_est, &vd_err, &vd_err_int, &omegae_est, Kp_pll, Ki_pll, dt );
        
    }
    
    // output for diagnostics
    printf("-------------------------------------\n");
    printf("    Park's Transformation Output\n");
    printf("vq = %1.3f [V] \n", vq);
    printf("vd = %1.3f [V] \n", vd);
    printf("v0 = %1.3f [V] \n", vz);
    printf("-------------------------------------\n\n");
 
    printf("-------------------------------------\n");
    printf("    Integrator Output\n");
    printf("f_int = %1.3e \n", f_int);
    printf("-------------------------------------\n\n");
    
    printf("-------------------------------------\n");
    printf("    PLL Output\n");
    printf("fe_est = %1.3f \n", fe_est);
    printf("-------------------------------------\n\n");
    
    return 0;
}
