## EXTI_va_Debounce

## 1. Mục tiêu

- Đọc nút nhấn bằng **ngắt ngoài EXTI** thay vì liên tục kiểm tra bằng polling.
- Cấu hình GPIO trực tiếp bằng thanh ghi trên STM32F411RE.
- Hiểu cách kết nối chân GPIO với EXTI thông qua **SYSCFG**.
- Làm quen với **NVIC và Interrupt Handler**.
- Hiểu và áp dụng kỹ thuật **debounce** để chống dội phím.
- Điều khiển LED LD2 bằng tín hiệu từ nút USER.
- Tiếp tục làm quen với lập trình **STM32 Bare-metal, không sử dụng HAL**.

---

## 2. Kiến thức đã học

### 2.1. GPIO

- GPIO được chia thành nhiều port như GPIOA, GPIOB, GPIOC.
- Mỗi port có các chân từ 0 đến 15.
- Mỗi chân GPIO có thể được cấu hình thành:
  - Input
  - Output
  - Alternate Function
  - Analog
- Sử dụng thanh ghi `MODER` để cấu hình mode của GPIO.
- Sử dụng `IDR` để đọc tín hiệu đầu vào.
- Sử dụng `ODR` để điều khiển tín hiệu đầu ra.
- Sử dụng các phép toán bit như `&`, `|`, `^`, `~` và `<<`.

### 2.2. EXTI

- EXTI (External Interrupt/Event Controller) dùng để tạo interrupt khi tín hiệu trên GPIO thay đổi.
- Hiểu cách một chân GPIO được kết nối với đường EXTI.
- Hiểu:
  - Rising edge: `LOW → HIGH`
  - Falling edge: `HIGH → LOW`
- Với nút USER trên NUCLEO-F411RE:

```text
Kết quả đạt được: PC13 → EXTI13


Sơ đồ đơn giản:

             STM32F411RE
                  |
        +---------+---------+
        |                   |
      PA5                 PC13
        |                   |
       LED              USER Button
