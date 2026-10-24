# Stepper Motor Control with STM32F411RE and ULN2003

## 1. Giới thiệu

Project điều khiển động cơ bước **28BYJ-48 5V** bằng board **NUCLEO-F411RE (STM32F411RE)** và module driver ULN2003.

Chương trình sử dụng GPIO điều khiển trực tiếp thông qua thanh ghi, không sử dụng HAL cho phần điều khiển động cơ.

Các chức năng chính:

- Cấu hình GPIO bằng thanh ghi.
- Điều khiển bốn ngõ vào IN1–IN4 của ULN2003.
- Tạo chuỗi kích nửa bước (half-step).
- Điều chỉnh tốc độ động cơ thông qua thời gian delay giữa các bước.

## 2. Phần cứng

- NUCLEO-F411RE.
- Động cơ bước 28BYJ-48 loại 5V.
- Module driver ULN2003.
- Nguồn 5V DC phù hợp với dòng tiêu thụ của động cơ.
- Dây nối.

## 3. Sơ đồ kết nối

| NUCLEO-F411RE | Chân STM32 | ULN2003 |
|---|---|---|
| A0 | PA0 | IN1 |
| A1 | PA1 | IN2 |
| A2 | PA4 | IN3 |
| A3 | PB0 | IN4 |
| GND | GND | GND (-) |
| Nguồn ngoài 5V | — | VCC (+) |

## 4. Nguyên lý hoạt động

Sau khi khởi tạo GPIO, chương trình chạy vòng lặp vô hạn:

1. Đọc mẫu hiện tại từ `step_sequence[]`.
2. Gọi `motor_write()` để xuất trạng thái điều khiển.
3. Tăng chỉ số `index`.
4. Khi chỉ số đạt 8, đưa về 0.
5. Chờ theo `STEP_DELAY_MS` rồi thực hiện bước tiếp theo.

Chuỗi trạng thái được lặp lại liên tục, tạo chuyển động quay của động cơ khi thứ tự pha và nguồn cấp phù hợp.

## 5. Kiến thức đạt được

Qua project này, người học thực hành:

- Điều khiển GPIO bằng thanh ghi trên STM32.
- Sử dụng RCC để bật clock ngoại vi.
- Cấu hình `MODER`, `OTYPER`, `PUPDR` và `BSRR`.
- Điều khiển động cơ bước qua ULN2003.
- Hiểu chuỗi kích half-step và mối liên hệ giữa delay với tốc độ động cơ.