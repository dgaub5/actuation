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
Vs    = 100;     % voltage magnitude (rms) of 3-phase voltage source [V]
fe    = 60;      % frequency of electrical source [Hz]
Te    = 1/fe;    % time period of electrical source [s]
dt    = 50e-6;   % time step of function loop [s]

% define time array
t     = 0:dt:2*Te;

% initialize memory
Valpha   = zeros( length(t), 1);  % storage array for voltage in q variable [V]
Vbeta    = zeros( length(t), 1);  % storage array for voltage in d variable [V]

%initialize pointers of type double; set initial value at address to 0
v_alpha = libpointer('doublePtr', 0.0 );
v_beta  = libpointer('doublePtr', 0.0 );

% electrical angular frequency
we   = 2.0 * pi * fe;

for k = 1:length(t) % <-- represents looping on the microcontroller
   
    % compute phase voltages (represents measurements into controller)
    v_as = sqrt(2.0) * Vs * cos( we * t(k) );
    v_bs = sqrt(2.0) * Vs * cos( we * t(k) - 2.0 * pi / 3.0 );
    v_cs = sqrt(2.0) * Vs * cos( we * t(k) + 2.0 * pi / 3.0 );
    
    calllib('libpowerConverterDLL','abcs2alphabeta', v_as, v_bs, v_cs, v_alpha, v_beta );
    
    % save values stored at pointer addresses (for plotting)
    V_alpha(k) = v_alpha.Value;
    V_beta(k)  = v_beta.Value;   
    
end


p = plot( t, V_alpha );
set( p, 'linewidth',1.5,'color','black');
hold on
p = plot( t, V_beta );
set( p, 'linewidth',1.5,'color','red');

xlabel('t [s]');
ylabel('Voltages in $\alpha\beta$ ref. frame [V]');
set(gca, 'Fontsize',20);
set(gcf, 'color','white');


unloadlibrary libpowerConverterDLL