%-------------------------------------------------------------------------%
% Description: this script validates the "on the fly" numerical integration
% function integrate() by comparing it to built-in MATLAB function trapz().
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

% initialize time array
tStart  = 0;        % start time [s]
tEnd    = 3;        % end time [s]
numPnts = 1000;     % number of points in the time array
t = linspace( 0, 3, 1000);

% define the integrand
alpha = 0.1;        % exponential decay factor
f = inline ( '1-exp(-alpha*x),', 'x','alpha' );   % function to integrate

% initialization
sum      = 0;
newSum   = 0;
funValue = f( t(1), alpha ); % initial function value
tValue   = t(1);             % initial time

% compute the numerical integration
for n = 2:length(t)

   newfunValue = f( t(n), alpha );  % current function value
   newtValue   = t(n);              % current time 
   
   newSum = integrate( funValue, newfunValue, tValue, newtValue, sum );  
   
   % update variables
   sum = newSum;
   funValue = newfunValue;
   tValue = newtValue;
    
end

sum 

% compare to built-in MATLAB function
trapz(t,f(t,alpha))



