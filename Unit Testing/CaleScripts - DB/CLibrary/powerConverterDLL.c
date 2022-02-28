/*
Description: this file contains the function library for the power converter.

Written by:
James Cale, Ph.D.
Colorado State University
Contact: jcale@colostate.edu

Revision Notes:
- 12 July 2021: initial release of abcs2qd0(), integrator(), folpf() [J.C.]
- 13 July 2021: initial release of qd02abcs() [J.C.]
- 14 July 2021: initial release of abcs2alphabeta(), pll() [J.C.]
 */

#include "powerConverterDLL.h"
#include <math.h>

#define PI 3.1415926535

/*
Description: transformation of stationary variables to arbitrary reference frame.

Variables:
 - theta: angle of reference frame [rad] (input)
 - x_as: a-phase stationary variable (input)
 - x_bs: b-phase stationary variable (input)
 - x_cs: c-phase stationary variable (input)
 - v_q: q-axis variable (output)
 - v_d: d-axis variable (output)
 - v_z: 0-axis variable (output)
*/
extern int abcs2qd0( double theta, double x_as, double x_bs, double x_cs, double *vq, double *vd, double *vz )
{
    
    *vq = (2.0/3.0) * ( x_as * cos(theta) + x_bs * cos(theta-2.0*PI/3.0) + x_cs * cos(theta+2.0*PI/3.0) );
    *vd = (2.0/3.0) * ( x_as * sin(theta) + x_bs * sin(theta-2.0*PI/3.0) + x_cs * sin(theta+2.0*PI/3.0) );
    *vz = (2.0/3.0) * ( x_as * (1.0/2.0)  + x_bs * (1.0/2.0)             + x_cs * (1.0/2.0) );
    
    return 0;
    
}

/*
Description: transformation arbitrary reference frame to stationary reference frame.

Variables:
 - theta: angle of reference frame [rad] (input)
 - x_as: a-phase stationary variable (output)
 - x_bs: b-phase stationary variable (output)
 - x_cs: c-phase stationary variable (output)
 - v_q: q-axis variable (output) (input)
 - v_d: d-axis variable (output) (input)
 - v_z: 0-axis variable (output) (input)
*/
extern int qd02abcs( double theta, double *x_as, double *x_bs, double *x_cs, double vq, double vd, double vz )
{
    
    *x_as = vq * cos(theta) + vd * sin(theta) + vz ;
    *x_bs = vq * cos(theta-2.0*PI/3.0) + vd * sin(theta-2.0*PI/3.0) + vz  ;
    *x_cs = vq * cos(theta+2.0*PI/3.0) + vd * sin(theta+2.0*PI/3.0) + vz  ;
    
    return 0;
    
}

/*
Description: transformation of stationary variables to alpha-beta variables.

Variables:
 - x_as: a-phase stationary variable (input)
 - x_bs: b-phase stationary variable (input)
 - x_cs: c-phase stationary variable (input)
 - v_alpha: q-axis variable (output)
 - v_beta: d-axis variable (output)
*/
extern int abcs2alphabeta( double x_as, double x_bs, double x_cs, double *v_alpha, double *v_beta )
{
    
    *v_alpha = (2.0/3.0) * ( x_as - x_bs * (1.0/2.0) - x_cs * (1.0/2.0) );
    *v_beta = (2.0/3.0) * (  x_bs * (sqrt(3.0)/2.0) - x_cs * (sqrt(3.0)/2.0));
    
    return 0;
    
}


/*
Description: first-order low-pass filter.

Variables:
 - x: signal to be filtered (input)
 - x_f: filtered signal (output)
 - deltat: time step (input)
 - fc: filter cut-off frequency (input)
*/
extern int folpf( double x, double *xf, double deltat, double fc )
{
    
    double tau = 0; // time constant [s]
    double c1  = 0; // numerical constant
    double c2  = 0; // numerical constant
    
    // compute filter time constant
    tau = 1.0/(2.0 * PI * fc);

    // compute filtered signal
    c1 = deltat / ( tau + deltat );
    c2 = 1 - c1;
    *xf = c1 * x + c2 * (*xf);
    
    return 0;
    
}

/*
Description: numerical integrator, using trapezoidal integration

Variables:
 - fval: value of function to be integrated, at current time (input)
 - fval0: value of function to be integrated, at previous time step (input)
 - f_int: cumulative integration of function (output)
 - deltat: time step (input)
*/
extern int integrator( double fval, double fval0, double *f_int, double deltat )
{
    
    *f_int = (*f_int) + (1.0/2.0) * deltat * ( fval + fval0 );
    
    return 0;
    
}


/*
Description: implements phase-locked loop, using methodology in [1].

Variables:
 - x_as: a-phase stationary variable (input)
 - x_bs: b-phase stationary variable (input)
 - x_cs: c-phase stationary variable (input)
 - v_alpha: q-axis variable (output)
 - v_beta: d-axis variable (output)

References:
[1] {insert paper reference here}
*/
extern int pll( double x_as, double x_bs, double x_cs, double *f, double *theta, double *vd_err, double *vd_err_int, double *omega, double Kp, double Ki, double deltat )
{
    
    // define local variables
    double vq = 0;          // q axis variable
    double vd = 0;          // d axis variable
    double vz = 0;          // 0 axis variable
    double vd_star = 0;     // desired d axis voltage (zero)
    double vd_err_last = 0; // storage for previous value of vd_err
    double omega_last = 0;  // storage for previous value of omega
    
    // transform voltages to qd0 variables
    abcs2qd0( *theta, x_as, x_bs, x_cs, &vq, &vd, &vz );
    
    // compute vd error (input to PI block)
    vd_err_last = *vd_err;
    *vd_err = vd_star - vd;
    
    // compute PI block output
    omega_last = *omega;
    integrator( *vd_err, vd_err_last, &(*vd_err_int), deltat );
    *omega = Kp * (*vd_err) + Ki * (*vd_err_int);
    
    //(note: anti-windup can go here)
    
    // compute output frequency
    *f = (*omega) / (2.0 * PI);
    
    // compute output angle
    integrator( *omega, omega_last, &(*theta), deltat );
    
    // bound angle between [0, 2pi)
    if ( *theta > 2.0 * PI ) {
        *theta -= 2.0 * PI;
    }
    
    return 0;
    
}

