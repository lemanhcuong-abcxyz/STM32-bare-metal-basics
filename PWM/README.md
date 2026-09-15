# PWM và Input Capture – STM32F411RE

## 1. Mục tiêu

* Tạo tín hiệu PWM bằng **TIM3**.
* Đo tần số PWM bằng **TIM2 Input Capture**.
* Làm quen với Timer và các thanh ghi STM32 ở mức **Bare-metal**.
* Hiển thị kết quả đo qua **UART2**.

---

## 2. Kiến thức đã học

* Cấu hình GPIO ở chế độ **Alternate Function**.
* Sử dụng **PSC** và **ARR** để thiết lập tần số Timer.
* Tạo PWM và điều chỉnh **Duty Cycle** bằng `CCR1`.
* Sử dụng **Input Capture** để đo chu kỳ tín hiệu.
* Đọc giá trị Timer thông qua `CCR1`.
* Tính tần số từ khoảng thời gian giữa hai lần Capture.
* Sử dụng `printf()` thông qua UART để debug.

### Công thức

```text
f_CNT = f_TIM / (PSC + 1)

f_PWM = f_TIM / [(PSC + 1)(ARR + 1)]

f_signal = f_CNT / Difference
```

---

## 3. Thanh ghi và phần cứng sử dụng

### GPIO

| Chân | Chức năng                |
| ---- | ------------------------ |
| PA6  | TIM3_CH1 – PWM Output    |
| PA0  | TIM2_CH1 – Input Capture |
| PA2  | USART2_TX                |
| PA3  | USART2_RX                |

### RCC

* `AHB1ENR` → bật clock GPIOA.
* `APB1ENR` Bit 0 → TIM2.
* `APB1ENR` Bit 1 → TIM3.
* `APB1ENR` Bit 17 → USART2.

### Timer

**TIM3:** tạo tín hiệu PWM.

```text
PSC = 15
ARR = 999
CCR1 = 500
```

**TIM2:** sử dụng Input Capture để đo tín hiệu PWM.

```text
PSC = 15
ARR = 0xFFFFFFFF
```

---

## 4. Sơ đồ kết nối

```text
TIM3_CH1
  PA6
   │
   │ PWM
   ▼
  PA0
TIM2_CH1
   │
   ▼
Input Capture
   │
   ▼
CCR1 → Tính tần số
```

Kết nối:

```text
PA6 ───────── PA0
```

Khi kết nối với thiết bị hoặc board bên ngoài cần **GND chung**.

---

## 5. Kết quả đạt được

* Tạo được tín hiệu PWM bằng TIM3.
* Đo được chu kỳ và tần số PWM bằng TIM2 Input Capture.
* Hiển thị kết quả qua UART2 và `picocom`.
* Làm quen với quy trình:

```text
PWM → Input Capture → CCR → Tính tần số → UART → PC
```