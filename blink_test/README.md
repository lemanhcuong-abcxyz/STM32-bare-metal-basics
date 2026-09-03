# Blink Test

## Mục tiêu

Tìm hiểu cách cấu hình và điều khiển GPIO trên STM32F411RE bằng cách thao tác trực tiếp với các thanh ghi, không sử dụng HAL.

## Kiến thức đã học

- Cách RCC cung cấp Clock cho GPIO.
- Cách cấu hình GPIO ở chế độ Output.
- Cách xác định vị trí bit cần thay đổi trong thanh ghi.
- Cách điều khiển GPIO thông qua thanh ghi.
- Khái niệm cơ bản về Memory-mapped Register.

## Đã thực hiện

- Bật Clock cho GPIOA thông qua `RCC->AHB1ENR`.
- Cấu hình chân PA5 ở chế độ Output thông qua `GPIOA->MODER`.
- Điều khiển trạng thái của PA5 thông qua thanh ghi GPIO.
- Làm LED tích hợp trên Nucleo-F411RE sáng.

## Các thanh ghi chính

- `RCC->AHB1ENR`: Bật Clock cho GPIOA.
- `GPIOA->MODER`: Cấu hình chế độ hoạt động của PA5.
- `GPIOA->ODR`: Điều khiển mức logic đầu ra của PA5.

## Kết quả đạt được

- Hiểu được cách bật Clock cho GPIO.
- Hiểu được cách cấu hình một chân GPIO làm Output.
- Hiểu được cách xác định và thay đổi các bit trong thanh ghi.
- Hiểu được cách điều khiển trạng thái của GPIO.
- Làm cho LED tích hợp trên Nucleo-F411RE sáng.
- Có kiến thức nền tảng để tiếp tục tìm hiểu các Peripheral khác của STM32.