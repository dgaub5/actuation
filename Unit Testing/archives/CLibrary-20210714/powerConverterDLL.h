#ifndef powerConverterDLL_h
#define powerConverterDLL_h

extern int abcs2qd0( double theta, double x_as, double x_bs, double x_cs, double *vq, double *vd, double *vz );
extern int folpf( double x, double *xf, double deltat, double fc );
extern int integrator( double fval, double fval0, double *f_int, double deltat );

#endif
