%-------------------------------------------------------------------------%
% Description: this script validates the "on the fly" proportional-integral
% with anti-windup function piBlock() by comparing it to built-in MATLAB 
% function trapz().
%
% Inputs:
%   - none
%
% Outputs:
%   - none (print to terminal)
%
% Written by: 
% James Cale, Ph.D.
% Colorado State University
% Contact: jcale@colostate.edu
%
% Revision Notes:
%   - 08 June 2021: initial release (JC)
%-------------------------------------------------------------------------%
 
clear all; clc; close all; 

% define parameters
params.Kp = 2;  % proportional gain
params.Ki = 1;  % integral gain
params.lowerLim = -20;  % upper limit for anti-windup
params.upperLim = 20;   % lower limit for anti-windup

% initialize time array
tStart  = 0;        % start time [s]
tEnd    = 3;        % end time [s]
numPnts = 1000;     % number of points in the time array
t = linspace( 0, 3, 1000);

% define the integrand
alpha = 0.1;        % exponential decay factor
f = inline ( '1-exp(-alpha*x),', 'x','alpha' );   % function to integrate

% initialization
piOutput      = 0;
integralSum   = 0;
funValue = f( t(1), alpha ); % initial function value
tValue   = t(1);             % initial time

% compute the numerical integration
for n = 2:length(t)

   newfunValue = f( t(n), alpha );  % current function value
   newtValue   = t(n);              % current time 
   
   [ piOutput, newIntegralSum ] = piBlock( params, funValue, newfunValue, tValue, newtValue, integralSum );  
   
   % update variables
   integralSum = newIntegralSum;
   funValue = newfunValue;
   tValue = newtValue;
    
end

piOutput

% compare to built-in MATLAB function
matlabpi = params.Kp * f(t,alpha) + params.Ki * trapz(t,f(t,alpha));
matlabpi(end)



