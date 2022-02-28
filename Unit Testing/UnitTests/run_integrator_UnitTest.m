%-------------------------------------------------------------------------%
% Description: this script loads a shared library (compiled using gcc), 
% and tests function(s) in the library to ensure correct answers are obtained.
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
%   - 13 July 2021: included test of integrator() C code [J.C.]
%-------------------------------------------------------------------------%

clear all; clc; clear all; close all;

% add path to C library
addpath ../CLibrary/

% load shared library and associated header file
loadlibrary('libpowerConverterDLL.so', 'powerConverterDLL.h');

% print the names of all functions in the shared library (as a check)
functionNames = libfunctions('libpowerConverterDLL')

%-------------------------------------------------------------------------%
%         Initialization for Integrator Unit Test
%-------------------------------------------------------------------------%

% set fixed parameters
f  = 60;
T  = 1/f;
dt = 50e-6;
t  = 0:dt:2*T; % time array

% define the integrand
alpha = 0.1;  % exponential decay constant
f = inline ( '1-exp(-alpha*x),', 'x','alpha' );   % function to integrate

%initialize pointer of type double; set initial value at address to 0
f_int = libpointer('doublePtr', 0.0 );

% initialize variables
fval  = 0;
fval0 = 0;

for k = 1:length(t) % <-- represents looping on the microcontroller
   
    % perform integration
    fval = f( t(k), alpha );  % current function value 
    calllib('libpowerConverterDLL','integrator', fval, fval0, f_int, dt );
    fval0 = fval;
    
end

% print final value of cumlative integral
disp(sprintf('Final value of f_int: %1.3e',f_int.Value));


unloadlibrary libpowerConverterDLL