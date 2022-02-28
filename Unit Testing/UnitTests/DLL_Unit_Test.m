%-------------------------------------------------------------------------%

%-------------------------------------------------------------------------%

clear all; clc; clear all; close all;

% add path to C library
addpath ../CLibrary/

% load shared library and associated header file
loadlibrary('libpowerConverterDLL.so', 'powerConverterDLL.h');

% print the names of all functions in the shared library (as a check)
functionNames = libfunctions('libpowerConverterDLL')

%-------------------------------------------------------------------------%
%         Initialization for Parks Transform Unit Test
%-------------------------------------------------------------------------%
% set fixed parameters
Vs    = 100;     % voltage magnitude (rms) of 3-phase voltage source [V]
fe    = 60;      % frequency of electrical source [Hz]
Te    = 1/fe;    % time period of electrical source [s]
dt    = 50e-6;   % time step of function loop [s]

% define time array
t     = 0:dt:2*Te;

% initialize memory
Vq    = zeros( length(t), 1);  % storage array for voltage in q variable [V]
Vd    = zeros( length(t), 1);  % storage array for voltage in d variable [V]
V0    = zeros( length(t), 1);  % storage array voltage in zero variable [V]

%initialize pointers of type double; set initial value at address to 0
vq = libpointer('doublePtr', 0.0 );
vd = libpointer('doublePtr', 0.0 );
vz = libpointer('doublePtr', 0.0 );

for k = 1:length(t) % <-- represents looping on the microcontroller
   
    % compute phase voltages (represents measurements into controller)
    we   = 2.0 * pi * fe;
    v_as = sqrt(2.0) * Vs * cos( we * t(k) );
    v_bs = sqrt(2.0) * Vs * cos( we * t(k) - 2.0 * pi / 3.0 );
    v_cs = sqrt(2.0) * Vs * cos( we * t(k) + 2.0 * pi / 3.0 );
    
    % compute arbitrary reference frame angle
    theta = we * t(k);
    
    calllib('libpowerConverterDLL','abcs2qd0', theta, v_as, v_bs, v_cs, vq, vd, vz );
    
    % save values stored at pointer addresses (for plotting)
    Vq(k) = vq.Value;
    Vd(k) = vd.Value;
    V0(k) = vz.Value;    
    
end


p = plot( t, Vq );
set( p, 'linewidth',1.5,'color','black');
hold on
p = plot( t, Vd );
set( p, 'linewidth',1.5,'color','red');
hold on
p = plot( t, V0 );
set( p, 'linewidth',1.5,'color','blue');

xlabel('t [s]');
ylabel('Transformed voltage [V]');
set(gca, 'Fontsize',20);
set(gcf, 'color','white');
ylim([-1 150]);

%wait 5 seconds then close figure
disp('Parks Transform Unit Test');
pause(5.0);
close(1);
%-------------------------------------------------------------------------%
%        Initialization for Inverse Park's Transformation Unit Test
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

%wait 5 seconds then close figure
disp('Inverse Parks Transform Unit Test');
pause(5.0);
close(1);
%-------------------------------------------------------------------------%
%        Initialization for Park's (alpha - beta) Transformation Unit Test
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

%wait 5 seconds then close figure
disp('ABC to Alpha-Beta Transform');
pause(5.0);
close(1);
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
disp('Integration Unit Test');
disp(sprintf('Final value of f_int: %1.3e',f_int.Value));
pause(2.0);

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
disp('Low Pass Filter Unit Test');
pause(5.0);
close(1);

% %-------------------------------------------------------------------------%
% %        Initialization for PLL Unit Test
% %-------------------------------------------------------------------------%
% 
 % set fixed parameters
Vs    = 100;     % voltage magnitude (rms) of 3-phase voltage source [V]
fe    = 60;      % frequency of electrical source [Hz]
Te    = 1/fe;    % time period of electrical source [s]
dt    = 50e-6;   % time step of function loop [s]
 
% define time array
t     = 0:dt:5*Te;
% 
% initialize memory
F     = zeros( length(t), 1);  % storage array for frequency variable [Hz]
Theta = zeros( length(t), 1);  % storage array for angle variable [V]
% 
%initialize pointers of type double; set initial value at address to 0
f      = libpointer('doublePtr', 0.0 );
theta  = libpointer('doublePtr', 0.0 );
vd_err      = libpointer('doublePtr', 0.0 );
vd_err_int  = libpointer('doublePtr', 0.0 );
omega       = libpointer('doublePtr', 0.0 );
% 
% % electrical angular frequency
we   = 2.0 * pi * fe;
Kp   = 20;
Ki   = 1;
% 
for k = 1:length(t) % <-- represents looping on the microcontroller
    
% compute phase voltages (represents measurements into controller)
     v_as = sqrt(2.0) * Vs * cos( we * t(k) );
     v_bs = sqrt(2.0) * Vs * cos( we * t(k) - 2.0 * pi / 3.0 );
     v_cs = sqrt(2.0) * Vs * cos( we * t(k) + 2.0 * pi / 3.0 );
    
     calllib('libpowerConverterDLL','pll', v_as, v_bs, v_cs, f, theta, vd_err, vd_err_int, omega, Kp, Ki, dt );
     
     % save values stored at pointer addresses (for plotting)
     F(k)      = f.Value;
     Theta(k)  = theta.Value;   
     
 end
 
subplot(211)
p = plot( t, F );
set( p, 'linewidth',1.5,'color','black');
ylabel('{f} [Hz]');
xlabel('t [s]');
set(gca, 'Fontsize',20);
set(gcf, 'color','white');
ylim([0 1.05*fe])
 
subplot(212)
p = plot( t, Theta );
ylabel('{theta} [rad]');
set( p, 'linewidth',1.5,'color','black');
xlabel('t [s]');
set(gca, 'Fontsize',20);
set(gcf, 'color','white');
 
%f.Value
disp('PLL Unit Test');
pause(5.0);
close(1);

disp('DLL Unit Tests Finished')
unloadlibrary libpowerConverterDLL