addpath ../CLibraryClaudio

loadlibrary('libpowerConverter.so','powerConverter.h')
%%
fNames = libfunctions('libpowerConverter')

%%

%-------------------------------------------------------------------------%
%        Initialization for Park's Transformation Unit Test
%-------------------------------------------------------------------------%

% set fixed parameters
Vs    = 100/sqrt(2); % voltage magnitude (rms) of 3-phase voltage source [V]
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
Kp   = 20;
Ki   = 400;

% noiseLevel = 0.01*Vs;
noiseLevel = 0;

for k = 1:length(t) % <-- represents looping on the microcontroller
   
%     freq_noise = 0.0001*we*(rand-0.5);
    freq_noise = 0;

    % compute phase voltages (represents measurements into controller)
    v_as = sqrt(2.0) * Vs * cos( (we + freq_noise) * t(k) ) + noiseLevel*(rand-0.5);
    v_bs = sqrt(2.0) * Vs * cos( (we + freq_noise) * t(k) - 2.0 * pi / 3.0 ) + noiseLevel*(rand-0.5);
    v_cs = sqrt(2.0) * Vs * cos( (we + freq_noise) * t(k) + 2.0 * pi / 3.0 ) + noiseLevel*(rand-0.5);
    
    calllib('libpowerConverter','pll', v_as, v_bs, v_cs, f, theta, vd_err, vd_err_int, omega, Kp, Ki ,dt );
    
    % save values stored at pointer addresses (for plotting)
    F(k)      = f.Value;
    Theta(k)  = theta.Value;   
    
end
%%
subplot(211)
p = plot( t, F );
set( p, 'linewidth',1.5,'color','black');
ylabel('$\hat{f}$ [Hz]', 'interpreter', 'Latex');
xlabel('t [s]');
set(gca, 'Fontsize',20);
set(gcf, 'color','white');
% ylim([0.95*fe 1.05*fe])

subplot(212)
p = plot( t, Theta );
hold on
vas = sqrt(2.0) * Vs * cos( we * t );
plot(t, vas/100);
ylabel('$\hat{\theta}$ [rad]', 'interpreter', 'Latex');
set( p, 'linewidth',1.5,'color','black');
xlabel('t [s]');
set(gca, 'Fontsize',20);
set(gcf, 'color','white');