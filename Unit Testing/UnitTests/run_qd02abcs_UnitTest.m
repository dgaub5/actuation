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
%   - 12 July 2021: included test of qd02abcs() C code [J.C.]
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
Xas    = zeros( length(t), 1);  % storage array for phase a voltage [V]
Xbs    = zeros( length(t), 1);  % storage array for phase b voltage [V]
Xcs    = zeros( length(t), 1);  % storage array for phase c voltage [V]

% initialize pointers of type double; set initial value at address to 0
x_as = libpointer('doublePtr', 0.0 );
x_bs = libpointer('doublePtr', 0.0 );
x_cs = libpointer('doublePtr', 0.0 );

% define ideal three phase voltage set
we   = 2.0 * pi * fe;
Vas = sqrt(2.0) * Vs * cos( we * t );
Vbs = sqrt(2.0) * Vs * cos( we * t - 2.0 * pi / 3.0 );
Vcs = sqrt(2.0) * Vs * cos( we * t + 2.0 * pi / 3.0 );
    
for k = 1:length(t) % <-- represents looping on the microcontroller
   

    % compute arbitrary reference frame angle
    theta = we * t(k); %<-- assumes synchronous ref. frame
   
    % define ideal v_qd0 voltages
    vq = sqrt(2) * Vs;
    vd = 0;
    vz = 0;    
 
    calllib('libpowerConverterDLL','qd02abcs', theta, x_as, x_bs, x_cs, vq, vd, vz );
    
    % save values stored at pointer addresses (for plotting)
    Xas(k) = x_as.Value;
    Xbs(k) = x_bs.Value;
    Xcs(k) = x_cs.Value;    
    
end


p = plot( t, Xas, t, Vas );
set( p, 'linewidth',1.5,'color','black');
hold on
p = plot( t, Xbs, t, Vbs );
set( p, 'linewidth',1.5,'color','red');
hold on
p = plot( t, Xcs, t, Vcs );
set( p, 'linewidth',1.5,'color','blue');

xlabel('t [s]');
ylabel('Inverse-transformed voltages [V]');
set(gca, 'Fontsize',20);
set(gcf, 'color','white');


unloadlibrary libpowerConverterDLL