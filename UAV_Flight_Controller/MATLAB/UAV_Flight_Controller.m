clc;
clear;
close all;

%% Simulation parameters
dt = 0.01;
T = 10;
t = 0:dt:T;

%% UAV simplified model
Ku = 15;
damping = 1.5;

%% PID parameters
Kp = 3.0;
Ki = 0.5;
Kd = 0.8;

%% Initial conditions
angle = 10 * pi/180;
rate = 0;

setpoint = 0;

integral = 0;
previous_error = 0;

%% Data storage
angle_log = zeros(size(t));
control_log = zeros(size(t));

%% Simulation loop
for k = 1:length(t)

    error = setpoint - angle;

    integral = integral + error * dt;

    derivative = (error - previous_error) / dt;

    control = Kp * error ...
            + Ki * integral ...
            + Kd * derivative;

    % Limit control signal
    control = max(min(control, 1), -1);

    % Simplified quadcopter dynamics
    acceleration = Ku * control - damping * rate;

    rate = rate + acceleration * dt;
    angle = angle + rate * dt;

    previous_error = error;

    angle_log(k) = angle * 180/pi;
    control_log(k) = control;
end

%% Plot angle response
figure;

plot(t, angle_log, 'LineWidth', 1.5);
grid on;

xlabel('Time (s)');
ylabel('Angle (degree)');
title('UAV Roll/Pitch PID Stabilization');

%% Plot control signal
figure;

plot(t, control_log, 'LineWidth', 1.5);
grid on;

xlabel('Time (s)');
ylabel('Control Output');
title('PID Control Signal');