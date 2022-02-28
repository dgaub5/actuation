%-------------------------------------------------------------------------%
% Description: this script loads a shared library (compiled using gcc), 
% and tests function(s) in the library to ensure correct answers are obtained.
%
% Inputs:
%   - none (stand-alone script)
% Outputs:
%   - note (print to screen)
%P 
% Written by: 
% James Cale, Ph.D.
% Colorado State University
% Contact: jcale@colostate.edu
%
% Revision Notes:
%   - 12 July 2021: included test of folpf() C code [J.C.]
%-------------------------------------------------------------------------%

clear all; clc; clear all; close all;

% add path to C library
addpath ../CLibrary/

% load shared library and associated header file
loadlibrary('libpowerConverterDLL.so', 'powerConverterDLL.h');

% print the names of all functions in the shared library (as a check)
functionNames = libfunctions('libpowerConverterDLL')

%-------------------------------------------------------------------------%
%         Initialization for Low-Pass Filter Test
%-------------------------------------------------------------------------%

% set fixed parameters
xpk   = 5;       % magnitude of fundamental signal
f     = 60;      % frequency of fundamental signal [Hz]
T     = 1/f;     % time period of fundamental signal [s]
dt    = T/1000;  % time step of function loop [s]
fc    = 500;     % cut-off frequency of filter [Hz]

% define time array
t     = 0:dt:2*T;

% define noise signal
noise = rand(1,length(t));
noise = noise - mean(noise);

% define total signal
x = xpk * sin(2*pi*f*t) + noise;

% initialize memory
Xf = zeros( length(t), 1);  % storage array for filtered waveform (for plotting)

%initialize pointer of type double; set initial value at address to 0
xf = libpointer('doublePtr', 0.0 );

for k = 1:length(t) % <-- represents looping on the microcontroller
   
    % filter waveform
    calllib('libpowerConverterDLL','folpf', x(k), xf, dt, fc );
    
    % save values stored at pointer addresses (for plotting)
    Xf(k) = xf.Value;
    
end


p = plot( t, x );
set( p, 'linewidth',1.5,'color','black');
hold on
p = plot( t, Xf );
set( p, 'linewidth',1.5,'color','red');
xlabel('t [s]');
ylabel('Noisy and filtered waveforms');
set(gca, 'Fontsize',20);
set(gcf, 'color','white');
%ylim([-1 150]);


unloadlibrary libpowerConverterDLL