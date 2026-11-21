# UAV Flight Controller – STM32F411RE

## Giới thiệu

Đây là dự án phát triển hệ thống điều khiển UAV mini sử dụng STM32F411RE, tập trung vào nghiên cứu hệ thống điều khiển cân bằng, thuật toán PID, xử lý dữ liệu cảm biến và mô phỏng trước khi triển khai lên phần cứng thực tế.

Dự án hướng đến việc tự xây dựng flight controller, tìm hiểu nguyên lý hoạt động của quadcopter và từng bước triển khai hệ thống điều khiển từ mô phỏng đến thực nghiệm.

## Những nội dung đã thực hiện

### 1. Phát triển hệ thống điều khiển trên STM32F411RE

- Tạo và cấu hình project STM32F411RE trên STM32CubeIDE.
- Xây dựng chương trình điều khiển trên STM32.
- Nghiên cứu cấu hình GPIO, Timer, PWM và các ngoại vi phục vụ hệ thống điều khiển.
- Phát triển chương trình điều khiển theo hướng embedded, hướng tới hạn chế phụ thuộc vào HAL và hiểu rõ hoạt động của phần cứng.

### 2. Nghiên cứu thuật toán PID

Nghiên cứu nguyên lý điều khiển PID và ứng dụng trong việc ổn định góc nghiêng của UAV.

Hai trục điều khiển chính:

- Roll: góc nghiêng ngang.
- Pitch: góc nghiêng trước – sau.

Công thức PID:

\[
u(t)=K_p e(t)+K_i\int e(t)dt+K_d\frac{de(t)}{dt}
\]

Mục tiêu của bộ điều khiển là giảm sai số góc nghiêng và đưa hệ thống về trạng thái cân bằng.

### 3. Xây dựng mô phỏng PID trên MATLAB

Đã xây dựng chương trình MATLAB mô phỏng hệ thống điều khiển cân bằng UAV 2 trục Roll và Pitch.

Các nội dung trong mô phỏng:

- Hai bộ điều khiển PID cho Roll và Pitch.
- Mô hình động học góc nghiêng đơn giản.
- Mô phỏng nhiễu cảm biến Gyroscope và Accelerometer.
- Mô phỏng sai lệch Gyroscope.
- Bộ lọc Complementary Filter để ước lượng góc.
- Mô phỏng tác động ngoại lực lên hệ thống.
- Motor Mixer phân phối tín hiệu điều khiển đến 4 động cơ.
- Mô phỏng đáp ứng động cơ và giới hạn tín hiệu đầu ra.
- Vẽ đồ thị góc nghiêng, tín hiệu PID, tốc độ góc và tín hiệu 4 motor.
- Tính toán thời gian ổn định, góc lệch lớn nhất và sai số cuối.

Mô phỏng được xây dựng nhằm nghiên cứu đáp ứng của bộ điều khiển và đánh giá khả năng ổn định hệ thống trước khi triển khai lên UAV thực tế.

> Mô hình MATLAB hiện tại là mô hình đơn giản phục vụ nghiên cứu và học tập, chưa mô phỏng đầy đủ động lực học của quadcopter thực tế.

## Phần cứng hiện có

- STM32 Nucleo-F411RE
- Động cơ BLDC A2212 930KV
- Pin LiPo 1300mAh

Phần cứng hiện chưa được hoàn thiện thành một quadcopter đầy đủ.

## Công cụ sử dụng

- STM32CubeIDE
- STM32F411RE – ARM Cortex-M4
- C
- MATLAB
- Simulink (dự kiến phát triển tiếp)

## Tiến độ dự án

- [x] Thiết lập project STM32F411RE
- [x] Phát triển chương trình điều khiển STM32
- [x] Nghiên cứu nguyên lý PID
- [x] Xây dựng chương trình MATLAB mô phỏng PID Roll/Pitch
- [x] Xây dựng mô hình mô phỏng cảm biến và Complementary Filter
- [x] Xây dựng Motor Mixer cho 4 động cơ
- [ ] Chạy và kiểm tra kết quả MATLAB
- [ ] Xây dựng mô hình Simulink
- [ ] Hoàn thiện phần cứng quadcopter
- [ ] Tích hợp cảm biến và hệ thống ESC
- [ ] Kiểm tra PID trên giá đỡ
- [ ] Thử nghiệm bay có kiểm soát

## Kế hoạch phát triển tiếp theo

1. Kiểm tra và đánh giá kết quả mô phỏng MATLAB.
2. Xây dựng mô hình hệ thống trên Simulink.
3. Hoàn thiện khung và hệ thống 4 động cơ.
4. Tích hợp MPU6050 và ESC với STM32F411RE.
5. Triển khai và điều chỉnh PID trên phần cứng.
6. Kiểm tra hệ thống trên giá đỡ an toàn.
7. Tiến hành thử nghiệm bay khi hệ thống đã được kiểm tra đầy đủ.

## An toàn

- Tháo cánh quạt trong quá trình kiểm tra firmware và tín hiệu điều khiển ban đầu.
- Cố định UAV trên giá đỡ khi thử nghiệm hệ thống cân bằng.
- Kiểm tra nguồn, ESC, thứ tự động cơ và chiều quay trước khi thử nghiệm.
- Không tiến hành bay khi hệ thống điều khiển và phần cứng chưa được kiểm tra đầy đủ.

---

Dự án được thực hiện với mục đích học tập, nghiên cứu hệ thống điều khiển nhúng và phát triển flight controller cho UAV mini.