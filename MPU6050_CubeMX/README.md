# STM32F411RE – MPU6050 Angle Measurement

## 1. Giới thiệu

Sử dụng STM32F411RE giao tiếp với cảm biến MPU6050 qua giao thức I2C để đọc dữ liệu gia tốc và vận tốc góc, từ đó tính toán góc nghiêng Roll và Pitch bằng Complementary Filter.

## 2. Sơ đồ kết nối

```text
       STM32 NUCLEO-F411RE
      ┌───────────────────┐
      │                   │
      │       PB8         ├──── SCL
      │       PB9         ├──── SDA
      │       3.3V        ├──── VCC
      │       GND         ├──── GND
      │       PA2         ├──── UART2_TX
      │       PA5         ├──── LED
      │                   │
      └───────────────────┘
                 │
                 │ I2C
                 ▼
          ┌─────────────┐
          │   MPU6050   │
          │             │
          │ SCL         │
          │ SDA         │
          │ VCC         │
          │ GND         │
          │ AD0 ─── GND │
          └─────────────┘
```

| STM32F411RE | MPU6050 |
| ----------- | ------- |
| PB8         | SCL     |
| PB9         | SDA     |
| 3.3V        | VCC     |
| GND         | GND     |
| GND         | AD0     |

## 3. Chức năng chính

* Cấu hình I2C1 bằng thanh ghi để giao tiếp với MPU6050.
* Khởi tạo và đọc dữ liệu cảm biến.
* Hiệu chuẩn độ lệch gyro khi khởi động.
* Tính toán góc Roll và Pitch từ dữ liệu gia tốc và gyro.
* Sử dụng Complementary Filter để kết hợp hai nguồn dữ liệu.
* Sử dụng SysTick tạo chu kỳ lấy mẫu 10 ms.
* Gửi dữ liệu qua UART2 với baud rate 115200.

## 4. Thuật toán xử lý

Complementary Filter kết hợp góc tính từ gyro và góc tính từ accelerometer:

$$
Angle=0.98(Angle_{previous}+Gyro\cdot dt)+0.02Angle_{acc}
$$

Trong đó:

* Hệ số gyro: 0.98.
* Hệ số accelerometer: 0.02.
* Chu kỳ lấy mẫu: 0.01 giây.

## 5. Kết quả

Dữ liệu đầu ra qua UART bao gồm:

* Trạng thái khởi tạo UART và MPU6050.
* Thông báo hiệu chuẩn gyro.
* Giá trị góc Roll và Pitch theo thời gian.

## 6. Công nghệ sử dụng

* STM32F411RE
* Embedded C
* I2C
* UART
* SysTick Timer
* Complementary Filter
