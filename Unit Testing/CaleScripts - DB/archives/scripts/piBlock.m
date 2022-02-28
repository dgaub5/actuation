%-------------------------------------------------------------------------%
% Description: this script performs "on the fly" numerical integration using
% the trapezoidal method.
%
% Inputs:
%   - params: structure of parameters
%   - funValue : previous function value
%   - newfunValue : current function value
%   - tValue : previous time
%   - newtValue : curent time
%   - integralSum : cumulative integral for integral term (to this time)
%
% Outputs:
%   - piOutput : proportional-integral 
%   - newIntegralSum : updated cumulative integral for integral term
%
% Written by: 
% James Cale, Ph.D.
% Colorado State University
% Contact: jcale@colostate.edu
%
% Revision Notes:
%   - 08 June 2021: initial release (JC)
%-------------------------------------------------------------------------%
 
function [ piOutput, newIntegralSum ] = piBlock( params, funValue, newfunValue, tValue, newtValue, integralSum ) 

% extract parameters
Kp = params.Kp;  % proportional gain
Ki = params.Ki;  % integral gain
lowerLim = params.lowerLim;  % upper limit for anti-windup
upperLim = params.upperLim;  % lower limit for anti-windup

newIntegralSum = integrate( funValue, newfunValue, tValue, newtValue, integralSum ); 

piOutput = Kp * newfunValue + Ki * newIntegralSum;

% anti-windup (clamping method)
if piOutput < lowerLim, piOutput = lowerLim;, end;
if piOutput > upperLim, piOutput = upperLim;, end;    
