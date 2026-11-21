clc;
clear;
close all;

%% =========================================================
% UAV PID SIMULATION - 2 AXIS ROLL & PITCH
% PID + MPU6050-like sensor noise + Complementary Filter
% 4 Motor Mixer + Motor Dynamics + External Disturbance
%% =========================================================

%% 1. SIMULATION PARAMETERS
dt = 0.005;              % Sample time (s)
T  = 12;                 % Simulation time (s)
t  = 0:dt:T;
N  = length(t);

%% 2. UAV SIMPLIFIED DYNAMICS
K_roll  = 90;            % Roll torque gain
K_pitch = 90;            % Pitch torque gain

D_roll  = 2.5;           % Roll damping
D_pitch = 2.5;           % Pitch damping

motor_tau = 0.04;        % Motor response time constant

%% 3. PID PARAMETERS
% Roll PID
Kp_roll = 0.035;
Ki_roll = 0.004;
Kd_roll = 0.0025;

% Pitch PID
Kp_pitch = 0.035;
Ki_pitch = 0.004;
Kd_pitch = 0.0025;

%% 4. SETPOINT
roll_setpoint  = 0;      % Degree
pitch_setpoint = 0;      % Degree

%% 5. INITIAL CONDITIONS
roll_angle  = 10;        % Initial roll angle (degree)
pitch_angle = -7;        % Initial pitch angle (degree)

roll_rate  = 0;           % deg/s
pitch_rate = 0;           % deg/s

%% 6. PID VARIABLES
roll_integral  = 0;
pitch_integral = 0;

roll_integral_limit  = 30;
pitch_integral_limit = 30;

%% 7. MOTOR PARAMETERS
base_throttle = 0.55;

motor_min = 0;
motor_max = 1;

motor = ones(4,1) * base_throttle;

motor_command = ones(4,1) * base_throttle;

%% 8. SENSOR PARAMETERS
gyro_noise_std = 0.5;       % deg/s
acc_noise_std  = 0.15;      % degree

gyro_roll_bias  = 0.2;      % deg/s
gyro_pitch_bias = -0.15;    % deg/s

alpha_filter = 0.98;

%% 9. COMPLEMENTARY FILTER
roll_estimated  = roll_angle;
pitch_estimated = pitch_angle;

%% 10. DATA STORAGE
roll_log         = zeros(1,N);
pitch_log        = zeros(1,N);

roll_est_log     = zeros(1,N);
pitch_est_log    = zeros(1,N);

roll_acc_log     = zeros(1,N);
pitch_acc_log    = zeros(1,N);

roll_rate_log    = zeros(1,N);
pitch_rate_log   = zeros(1,N);

roll_pid_log     = zeros(1,N);
pitch_pid_log    = zeros(1,N);

motor_log        = zeros(4,N);

roll_set_log     = zeros(1,N);
pitch_set_log    = zeros(1,N);

%% 11. SIMULATION LOOP
for k = 1:N

    %% A. Simulated sensor readings

    gyro_roll_measured = roll_rate ...
        + gyro_roll_bias ...
        + gyro_noise_std * randn();

    gyro_pitch_measured = pitch_rate ...
        + gyro_pitch_bias ...
        + gyro_noise_std * randn();

    accel_roll_measured = roll_angle ...
        + acc_noise_std * randn();

    accel_pitch_measured = pitch_angle ...
        + acc_noise_std * randn();

    %% B. Complementary Filter

    roll_estimated = alpha_filter * ...
        (roll_estimated + gyro_roll_measured * dt) ...
        + (1-alpha_filter) * accel_roll_measured;

    pitch_estimated = alpha_filter * ...
        (pitch_estimated + gyro_pitch_measured * dt) ...
        + (1-alpha_filter) * accel_pitch_measured;

    %% C. PID ROLL

    roll_error = roll_setpoint - roll_estimated;

    roll_integral = roll_integral + roll_error * dt;

    roll_integral = max(min(roll_integral, ...
        roll_integral_limit), -roll_integral_limit);

    roll_pid = Kp_roll * roll_error ...
             + Ki_roll * roll_integral ...
             - Kd_roll * gyro_roll_measured;

    %% D. PID PITCH

    pitch_error = pitch_setpoint - pitch_estimated;

    pitch_integral = pitch_integral + pitch_error * dt;

    pitch_integral = max(min(pitch_integral, ...
        pitch_integral_limit), -pitch_integral_limit);

    pitch_pid = Kp_pitch * pitch_error ...
              + Ki_pitch * pitch_integral ...
              - Kd_pitch * gyro_pitch_measured;

    %% E. LIMIT PID OUTPUT

    roll_pid = max(min(roll_pid, 0.30), -0.30);
    pitch_pid = max(min(pitch_pid, 0.30), -0.30);

    %% F. 4-MOTOR MIXER

    % Motor layout:
    %
    %          FRONT
    %
    %       M1       M2
    %
    %
    %       M4       M3
    %
    %          REAR
    %
    % Roll:  right - left
    % Pitch: rear - front

    motor_command(1) = base_throttle ...
                     - roll_pid/2 ...
                     - pitch_pid/2;

    motor_command(2) = base_throttle ...
                     + roll_pid/2 ...
                     - pitch_pid/2;

    motor_command(3) = base_throttle ...
                     + roll_pid/2 ...
                     + pitch_pid/2;

    motor_command(4) = base_throttle ...
                     - roll_pid/2 ...
                     + pitch_pid/2;

    %% G. MOTOR LIMIT

    motor_command = max(min(motor_command, motor_max), motor_min);

    %% H. MOTOR DYNAMICS

    for m = 1:4

        motor(m) = motor(m) ...
                  + (motor_command(m) - motor(m)) ...
                  * dt / motor_tau;

    end

    %% I. MOTOR DIFFERENTIAL

    roll_motor_diff = ...
        (motor(2) + motor(3)) ...
        - (motor(1) + motor(4));

    pitch_motor_diff = ...
        (motor(3) + motor(4)) ...
        - (motor(1) + motor(2));

    %% J. EXTERNAL DISTURBANCE

    disturbance_roll = 0;
    disturbance_pitch = 0;

    % Roll disturbance
    if t(k) >= 3 && t(k) <= 3.15
        disturbance_roll = 20;
    end

    % Pitch disturbance
    if t(k) >= 7 && t(k) <= 7.15
        disturbance_pitch = -15;
    end

    %% K. UAV ANGULAR DYNAMICS

    roll_acceleration = K_roll * roll_motor_diff ...
                      - D_roll * roll_rate ...
                      + disturbance_roll;

    pitch_acceleration = K_pitch * pitch_motor_diff ...
                       - D_pitch * pitch_rate ...
                       + disturbance_pitch;

    %% L. UPDATE ANGULAR RATE

    roll_rate = roll_rate + roll_acceleration * dt;

    pitch_rate = pitch_rate + pitch_acceleration * dt;

    %% M. UPDATE ANGLES

    roll_angle = roll_angle + roll_rate * dt;

    pitch_angle = pitch_angle + pitch_rate * dt;

    %% N. SAVE DATA

    roll_log(k) = roll_angle;
    pitch_log(k) = pitch_angle;

    roll_est_log(k) = roll_estimated;
    pitch_est_log(k) = pitch_estimated;

    roll_acc_log(k) = accel_roll_measured;
    pitch_acc_log(k) = accel_pitch_measured;

    roll_rate_log(k) = roll_rate;
    pitch_rate_log(k) = pitch_rate;

    roll_pid_log(k) = roll_pid;
    pitch_pid_log(k) = pitch_pid;

    motor_log(:,k) = motor;

    roll_set_log(k) = roll_setpoint;
    pitch_set_log(k) = pitch_setpoint;

end

%% =========================================================
% 12. PLOT ROLL ANGLE
%% =========================================================

figure('Name','Roll Angle','NumberTitle','off');

plot(t, roll_log, 'LineWidth', 1.5);
hold on;

plot(t, roll_est_log, '--', 'LineWidth', 1.2);

plot(t, roll_set_log, ':', 'LineWidth', 1.5);

grid on;

xlabel('Time (s)');
ylabel('Roll Angle (degree)');

title('Roll PID Stabilization');

legend('Actual Roll', ...
       'Estimated Roll', ...
       'Setpoint', ...
       'Location','best');

%% =========================================================
% 13. PLOT PITCH ANGLE
%% =========================================================

figure('Name','Pitch Angle','NumberTitle','off');

plot(t, pitch_log, 'LineWidth', 1.5);
hold on;

plot(t, pitch_est_log, '--', 'LineWidth', 1.2);

plot(t, pitch_set_log, ':', 'LineWidth', 1.5);

grid on;

xlabel('Time (s)');
ylabel('Pitch Angle (degree)');

title('Pitch PID Stabilization');

legend('Actual Pitch', ...
       'Estimated Pitch', ...
       'Setpoint', ...
       'Location','best');

%% =========================================================
% 14. PLOT PID OUTPUT
%% =========================================================

figure('Name','PID Output','NumberTitle','off');

subplot(2,1,1);

plot(t, roll_pid_log, 'LineWidth', 1.5);

grid on;

xlabel('Time (s)');
ylabel('Roll PID Output');

title('Roll Control Output');

subplot(2,1,2);

plot(t, pitch_pid_log, 'LineWidth', 1.5);

grid on;

xlabel('Time (s)');
ylabel('Pitch PID Output');

title('Pitch Control Output');

%% =========================================================
% 15. PLOT MOTOR OUTPUT
%% =========================================================

figure('Name','Motor Outputs','NumberTitle','off');

plot(t, motor_log(1,:), 'LineWidth', 1.2);
hold on;

plot(t, motor_log(2,:), 'LineWidth', 1.2);

plot(t, motor_log(3,:), 'LineWidth', 1.2);

plot(t, motor_log(4,:), 'LineWidth', 1.2);

grid on;

xlabel('Time (s)');
ylabel('Motor Output (0-1)');

title('Four Motor Outputs');

legend('Motor 1','Motor 2','Motor 3','Motor 4', ...
       'Location','best');

%% =========================================================
% 16. PLOT ANGULAR RATE
%% =========================================================

figure('Name','Angular Rate','NumberTitle','off');

subplot(2,1,1);

plot(t, roll_rate_log, 'LineWidth', 1.2);

grid on;

xlabel('Time (s)');
ylabel('Roll Rate (deg/s)');

title('Roll Angular Rate');

subplot(2,1,2);

plot(t, pitch_rate_log, 'LineWidth', 1.2);

grid on;

xlabel('Time (s)');
ylabel('Pitch Rate (deg/s)');

title('Pitch Angular Rate');

%% =========================================================
% 17. PERFORMANCE ANALYSIS
%% =========================================================

settling_band = 1.0;  % +/-1 degree

roll_settling_time = NaN;
pitch_settling_time = NaN;

% Roll settling time
for k = 1:N

    if all(abs(roll_log(k:end)) < settling_band)

        roll_settling_time = t(k);
        break;

    end

end

% Pitch settling time
for k = 1:N

    if all(abs(pitch_log(k:end)) < settling_band)

        pitch_settling_time = t(k);
        break;

    end

end

roll_overshoot = max(abs(roll_log));

pitch_overshoot = max(abs(pitch_log));

roll_final_error = abs(roll_log(end) - roll_setpoint);

pitch_final_error = abs(pitch_log(end) - pitch_setpoint);

%% =========================================================
% 18. PRINT RESULTS
%% =========================================================

fprintf('\n========================================\n');
fprintf('       UAV PID SIMULATION RESULTS\n');
fprintf('========================================\n');

fprintf('Roll settling time: %.3f s\n', roll_settling_time);

fprintf('Pitch settling time: %.3f s\n', pitch_settling_time);

fprintf('Maximum Roll angle: %.3f deg\n', roll_overshoot);

fprintf('Maximum Pitch angle: %.3f deg\n', pitch_overshoot);

fprintf('Final Roll error: %.4f deg\n', roll_final_error);

fprintf('Final Pitch error: %.4f deg\n', pitch_final_error);

fprintf('========================================\n');
