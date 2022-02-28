/*
Description: this file contains the functions for the power converter.

Written by:
James Cale, Ph.D.
Colorado State University
Contact: jcale@colostate.edu

Revision Notes:
- 12 July 2021: initial release of abcs2qd0() [J.C.]
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


