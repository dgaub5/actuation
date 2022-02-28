#ifndef powerConverterDLL_h
#define powerConverterDLL_h

extern int abcs2qd0( double theta, double x_as, double x_bs, double x_cs, double *vq, double *vd, double *vz );
extern int qd02abcs( double theta, double *x_as, double *x_bs, double *x_cs, double vq, double vd, double vz );
extern int abcs2alphabeta( double x_as, double x_bs, double x_cs, double *v_alpha, double *v_beta );
extern int folpf( double x, double *xf, double deltat, double fc );
extern int integrator( double fval, double fval0, double *f_int, double deltat );
extern int pll( double x_as, double x_bs, double x_cs, double *f, double *theta, double *vd_err, double *vd_err_int, double *omega, double Kp, double Ki, double deltat );

#endif
