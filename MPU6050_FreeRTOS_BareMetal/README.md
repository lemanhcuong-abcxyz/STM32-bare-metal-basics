# MPU6050 + FreeRTOS trên STM32F411RE

## 1. Tổng quan

Dự án sử dụng STM32F411RE để đọc dữ liệu từ MPU6050 qua I2C, xử lý và tính góc Roll, Pitch bằng Complementary Filter, sau đó gửi kết quả qua UART.

Dự án được chia thành hai phần:

* **MPU6050_MX:** giao tiếp cảm biến, đọc dữ liệu và tính góc bằng chương trình tuần tự.
* **MPU6050 + FreeRTOS:** tổ chức lại chương trình thành các task, sử dụng Queue để trao đổi dữ liệu.

Phần giao tiếp ngoại vi được viết trực tiếp bằng thanh ghi, không sử dụng HAL.

## 2. Cấu hình phần cứng

| Thành phần | Cấu hình                          |
| ---------- | --------------------------------- |
| MCU        | STM32F411RE                       |
| Cảm biến   | MPU6050                           |
| I2C1       | PB8 (SCL), PB9 (SDA)              |
| USART2     | PA2 (TX), PA3 (RX)                |
| Baud rate  | 115200                            |
| RTOS       | FreeRTOS, sử dụng API CMSIS-RTOS2 |

## 3. Các chức năng đã triển khai

### 3.1. Giao tiếp và đọc dữ liệu MPU6050

* Cấu hình I2C1 ở mức thanh ghi để giao tiếp với MPU6050.
* Kiểm tra thiết bị thông qua thanh ghi `WHO_AM_I` (`0x75`).
* Khởi tạo cảm biến bằng cách cấu hình thanh ghi quản lý nguồn.
* Đọc dữ liệu accelerometer và gyroscope, ghép hai byte thành giá trị `int16_t`.

Dữ liệu thô được chuyển đổi sang đơn vị vật lý theo cấu hình ±2g và ±250°/s:

* Gia tốc: `raw / 16384.0f`
* Vận tốc góc: `raw / 131.0f`

### 3.2. Tính góc bằng Complementary Filter

Sử dụng accelerometer để tính góc nghiêng và gyroscope để theo dõi sự thay đổi góc theo thời gian.

Công thức kết hợp:

```c
angle = alpha * (angle + gyro_dps * dt)
      + (1.0f - alpha) * angle_acc;
```

Với `alpha = 0.98f`, bộ lọc ưu tiên dữ liệu gyroscope trong ngắn hạn và dùng accelerometer để hiệu chỉnh sai số tích lũy.

Kết quả là hai góc Roll và Pitch được cập nhật liên tục.

### 3.3. Tổ chức chương trình bằng FreeRTOS

Chương trình được chia thành ba task:

* **SensorTask:** định kỳ đọc dữ liệu MPU6050 và gửi dữ liệu thô vào `SensorQueue`.
* **ProcessingTask:** nhận dữ liệu từ `SensorQueue`, chuyển đổi đơn vị, tính Roll/Pitch bằng Complementary Filter và gửi kết quả vào `AngleQueue`.
* **UART_LogTask:** nhận góc từ `AngleQueue`, định dạng dữ liệu và truyền qua USART2.

Hai Queue được tạo bằng `osMessageQueueNew()`:

* `SensorQueue`: truyền dữ liệu cảm biến giữa SensorTask và ProcessingTask.
* `AngleQueue`: truyền kết quả góc giữa ProcessingTask và UART_LogTask.

Các task sử dụng `osMessageQueueGet()` để chờ dữ liệu. Khi Queue rỗng, task có thể chuyển sang trạng thái Blocked, nhường CPU cho task khác.

### 3.4. Truyền dữ liệu UART

USART2 được cấu hình ở baud rate 115200 để gửi kết quả Roll và Pitch đến máy tính.

Dữ liệu được định dạng bằng `snprintf()` và truyền bằng cách kiểm tra cờ `TXE` trước khi ghi vào thanh ghi dữ liệu UART.

UART được sử dụng để quan sát sự thay đổi góc khi di chuyển cảm biến và kiểm tra hoạt động của chương trình.

## 4. Luồng hoạt động

```text
MPU6050
   |
   | I2C
   v
SensorTask
   |
SensorQueue
   v
ProcessingTask
   |
   | Chuyển đổi dữ liệu
   | Complementary Filter
   v
AngleQueue
   |
   v
UART_LogTask
   |
   | USART2
   v
Máy tính
```

## 5. Kết quả

* Đọc được dữ liệu gia tốc và vận tốc góc từ MPU6050.
* Tính và xuất được Roll/Pitch qua UART.
* Quan sát được sự thay đổi góc khi di chuyển cảm biến.
* Chạy được mô hình đa task với FreeRTOS và trao đổi dữ liệu qua Queue.

## 6. Kiến thức áp dụng

* Lập trình STM32 ở mức thanh ghi: GPIO, I2C, USART.
* Đọc và xử lý dữ liệu cảm biến MPU6050.
* Chuyển đổi dữ liệu thô sang đơn vị vật lý.
* Ước lượng góc bằng Complementary Filter.
* Sử dụng FreeRTOS: task, scheduler, Queue và trạng thái Blocked.
* Tổ chức firmware thành các thành phần độc lập để thuận tiện mở rộng.