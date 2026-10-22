
# GPS_UART_NMEA

## 1. Mục tiêu

- Giao tiếp với module GPS bằng UART trên STM32F411RE.
- Nhận dữ liệu GPS thông qua USART1.
- Phân tích bản tin NMEA để lấy kinh độ và vĩ độ.
- Hiển thị kết quả qua USART2 trên máy tính.
- Thực hiện giao tiếp ngoại vi bằng cách thao tác trực tiếp với thanh ghi, không sử dụng HAL.

## 2. Kiến thức đã học

### 2.1 GPIO

- Cấu hình GPIO ở chế độ Alternate Function.
- Sử dụng thanh ghi `MODER` để chọn chức năng chân.
- Sử dụng thanh ghi `AFR` để chọn chức năng ngoại vi.
- Ánh xạ chân GPIO với USART.

### 2.2 UART/USART

- UART là giao tiếp nối tiếp dùng để truyền nhận dữ liệu.
- USART1 được sử dụng để nhận dữ liệu từ GPS.
- USART2 được sử dụng để gửi dữ liệu lên máy tính.
- Cấu hình baud rate thông qua thanh ghi `BRR`.
- Sử dụng các thanh ghi `CR1`, `CR2`, `CR3` để cấu hình USART.
- Kiểm tra cờ `RXNE` khi có dữ liệu nhận.
- Kiểm tra cờ `TXE` trước khi truyền dữ liệu.

### 2.3 Giao thức NMEA

- GPS thường gửi dữ liệu dưới dạng các chuỗi NMEA.
- Mỗi bản tin bắt đầu bằng ký tự `$`.
- Các trường dữ liệu được phân cách bằng dấu phẩy.
- Bản tin kết thúc bằng ký tự xuống dòng.
- Bản tin RMC chứa trạng thái định vị và thông tin tọa độ.
- Checksum được sử dụng để kiểm tra tính toàn vẹn dữ liệu.

### 2.4 Xử lý dữ liệu GPS

- Nhận từng byte dữ liệu từ USART1.
- Ghép các byte thành một bản tin hoàn chỉnh.
- Kiểm tra checksum trước khi phân tích.
- Tách các trường dữ liệu trong bản tin RMC.
- Kiểm tra trạng thái định vị hợp lệ.
- Chuyển tọa độ từ định dạng độ và phút sang độ thập phân.
- Lưu kinh độ, vĩ độ và trạng thái định vị.

## 3. Kết nối phần cứng

| Module GPS | STM32F411RE |
|---|---|
| TX | PA10 (USART1_RX) |
| RX | PA9 (USART1_TX, tùy chọn) |
| GND | GND |
| VCC | Nguồn phù hợp với module |

- GPS TX nối với chân nhận USART1 của STM32.
- Hai thiết bị phải có chung GND.
- Chân RX của GPS chỉ cần kết nối nếu muốn gửi lệnh cấu hình cho module.

## 4. Cấu hình USART

### 4.1 USART1

- Chức năng: nhận dữ liệu GPS.
- Chân nhận: PA10.
- Baud rate: 9600.
- Bật chế độ nhận bằng thanh ghi `CR1`.
- Đọc dữ liệu từ thanh ghi `DR` khi cờ `RXNE` được bật.

### 4.2 USART2

- Chức năng: gửi kết quả lên máy tính.
- Chân TX: PA2.
- Baud rate: 115200.
- Dữ liệu được truyền qua cổng Serial của ST-LINK.
- Sử dụng Serial Terminal để theo dõi kết quả.

## 5. Nguyên lý hoạt động

1. Khởi tạo GPIO và USART.
2. Khởi tạo bộ xử lý dữ liệu GPS.
3. USART1 nhận dữ liệu từ module GPS.
4. Chương trình đọc từng byte trong thanh ghi `DR`.
5. Các byte được ghép thành bản tin NMEA.
6. Kiểm tra checksum của bản tin.
7. Phân tích bản tin RMC và kiểm tra trạng thái định vị.
8. Lấy kinh độ, vĩ độ và lưu vào cấu trúc dữ liệu.
9. USART2 gửi kết quả lên máy tính.

## 6. Kết quả đạt được

- Xây dựng chương trình giao tiếp GPS với STM32F411RE.
- Cấu hình USART bằng thanh ghi, không sử dụng HAL.
- Xây dựng bộ nhận và phân tích dữ liệu NMEA.
- Kiểm tra checksum và trạng thái định vị.
- Tách kinh độ, vĩ độ từ bản tin RMC.
- Gửi thông báo và dữ liệu GPS qua USART2.