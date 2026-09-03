# UART Driver

## Mục tiêu

Tìm hiểu cách UART hoạt động trên STM32F411RE và cách xây dựng một UART Driver cơ bản bằng cách thao tác trực tiếp với thanh ghi, không sử dụng HAL.

## Kiến thức đã học

- Cách bật Clock cho Peripheral.
- Cách GPIO hoạt động ở chế độ Alternate Function.
- Cách kết nối GPIO với Peripheral UART.
- Nguyên lý giao tiếp UART.
- Baud rate và cách cấu hình Baud rate.
- Cách truyền dữ liệu qua UART.
- Cách sử dụng UART để Debug.
- Cách đưa `printf` qua UART để Debug.

## Đã thực hiện

- Bật Clock cho GPIO và UART.
- Cấu hình các chân GPIO cho UART.
- Cấu hình Alternate Function cho GPIO.
- Cấu hình các thông số của UART.
- Cấu hình Baud rate.
- Viết hàm truyền dữ liệu qua UART.
- Kết nối UART với máy tính để kiểm tra dữ liệu trên Terminal.
- Sử dụng UART làm giao tiếp Debug.
- Sử dụng `printf` để Debug thông qua UART.

## Các thanh ghi chính

- RCC
- GPIO
- USART

## Kết quả đạt được

- Hiểu được nguyên lý cơ bản của giao tiếp UART.
- Hiểu được cách bật Clock cho GPIO và UART.
- Hiểu được cách cấu hình GPIO ở chế độ Alternate Function.
- Hiểu được cách cấu hình Baud rate cho UART.
- Hiểu được quá trình truyền dữ liệu qua UART.
- Xây dựng được UART Driver cơ bản bằng cách thao tác trực tiếp với thanh ghi.
- Có thể gửi dữ liệu từ STM32F411RE tới máy tính.
- Có thể sử dụng Terminal để kiểm tra dữ liệu UART.
- Sử dụng UART làm giao tiếp Debug.
- Sử dụng `printf` để Debug thông qua UART.
- Có nền tảng để tiếp tục tìm hiểu các Peripheral khác của STM32.