%-------------------------------------------------------------------------%
% Description: this script loads a shared library (compiled using gcc), 
% tests functions calls, and ensures the correct answers are obtained.
%
% Inputs:
%   - none (stand-alone script)
% Outputs:
%   - note (print to screen)
%
% Written by: 
% James Cale, Ph.D.
% Colorado State University
% Contact: jcale@colostate.edu
%
% Revision Notes:
%   - 12 July 2021: included test of abc2alphabeta() C code [J.C.]
%-------------------------------------------------------------------------%

clear all; clc; clear all; close all;

% add path to C library
addpath ../CLibrary/

% load shared library and associated header file
loadlibrary('libpowerConverterDLL.so', 'powerConverterDLL.h');

% print the names of all functions in the shared library (as a check)
functionNames = libfunctions('libpowerConverterDLL')

%-------------------------------------------------------------------------%
%        Initialization for Park's Transformation Unit Test
%-------------------------------------------------------------------------%

% set fixed parameters
Vs    = 10;      % voltage magnitude (rms) of 3-phase voltage source [V]
fe    = 60;      % frequency of electrical source [Hz]
Te    = 1/fe;    % time period of electrical source [s]
dt    = 1e-5;    % time step of function loop [s]

% define time array
t     = 0:dt:5*Te;

% initialize memory
F     = zeros( length(t), 1);  % storage array for frequency variable [Hz]
Theta = zeros( length(t), 1);  % storage array for angle variable [V]

%initialize pointers of type double; set initial value at address to 0
f      = libpointer('doublePtr', 0.0 );
theta  = libpointer('doublePtr', 0.0 );
vd_err      = libpointer('doublePtr', 0.0 );
vd_err_int  = libpointer('doublePtr', 0.0 );
omega       = libpointer('doublePtr', 0.0 );

% electrical angular frequency
we   = 2.0 * pi * fe;
Kp   = 200;
Ki   = 10;

noiseLevel = 10e-3;

for k = 1:length(t) % <-- represents looping on the microcontroller
   
    % compute phase voltages (represents measurements into controller)
    v_as = sqrt(2.0) * Vs * cos( we * t(k) ) + noiseLevel*(rand-0.5);
    v_bs = sqrt(2.0) * Vs * cos( we * t(k) - 2.0 * pi / 3.0 ) + noiseLevel*(rand-0.5);
    v_cs = sqrt(2.0) * Vs * cos( we * t(k) + 2.0 * pi / 3.0 ) + noiseLevel*(rand-0.5);
    
    calllib('libpowerConverterDLL','pll', v_as, v_bs, v_cs, f, theta, vd_err, vd_err_int, omega, Kp, Ki ,dt );
    
    % save values stored at pointer addresses (for plotting)
    F(k)      = f.Value;
    Theta(k)  = theta.Value;   
    
end

subplot(211)
p = plot( t, F );
set( p, 'linewidth',1.5,'color','black');
ylabel('$\hat{f}$ [Hz]');
xlabel('t [s]');
set(gca, 'Fontsize',20);
set(gcf, 'color','white');
ylim([0.95*fe 1.05*fe])

subplot(212)
p = plot( t, Theta );
hold on
vas = sqrt(2.0) * Vs * cos( we * t );
plot(t, vas/100);
ylabel('$\hat{\theta}$ [rad]');
set( p, 'linewidth',1.5,'color','black');
xlabel('t [s]');
set(gca, 'Fontsize',20);
set(gcf, 'color','white');

f.Value
unloadlibrary libpowerConverterDLL