%-------------------------------------------------------------------------%
% Description: this script performs "on the fly" numerical integration using
% the trapezoidal method.
%
% Inputs:
%   - funValue : previous function value
%   - newfunValue : current function value
%   - tValue : previous time
%   - newtValue : curent time
%   - sum : cumulative integral (to this time)
%
% Outputs:
%   - newSum : updated cumulative integral
%
% Written by: 
% James Cale, Ph.D.
% Colorado State University
% Contact: jcale@colostate.edu
%
% Revision Notes:
%   - 08 June 2021: initial release (JC)
%-------------------------------------------------------------------------%
 
function newSum = integrate( funValue, newfunValue, tValue, newtValue, sum ) 

% time step
tstep = newtValue - tValue;

% new cumulative integral (sum)
newSum = sum + (1/2) * tstep * ( funValue + newfunValue );

