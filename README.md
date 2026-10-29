# AK Mini Arcade — STM32L151

Bộ sưu tập 5 game chạy trên **AK Embedded Base Kit STM32L151**, phát triển bằng C/C++ trên nền tảng AK Framework.

Project sử dụng màn hình OLED, 3 nút nhấn và buzzer có sẵn trên kit để tạo một máy chơi game mini với menu lựa chọn chung.

## Các game hiện có

| Game | Nội dung |
| --- | --- |
| Frog Journey | Điều khiển ếch vượt đường, qua sông và rừng; thu thập xu, hồi máu và hoàn thành màn trước khi hết giờ. |
| Snake | Điều khiển rắn ăn mồi, tăng điểm và tránh va chạm. |
| Brick Pong | Điều khiển thanh đỡ để đánh bóng phá các viên gạch. |
| Road Dodge | Chuyển làn để né xe, ghi điểm và vượt qua tốc độ tăng dần. |
| Flappy Bird | Điều khiển chim bay qua các cặp ống, tính điểm và ghi nhận kỷ lục trong phiên chơi. |

## Tính năng

- Màn hình khởi động và menu chung **AK MINI ARCADE**.
- Menu cuộn để lựa chọn giữa 5 game.
- Điều khiển bằng 3 nút có sẵn trên kit.
- Hiệu ứng âm thanh cho các sự kiện trong game.
- Giữ nút MODE khoảng 2 giây để quay về menu chọn game.
- Xử lý logic game theo sự kiện và timer của AK Framework.
- Các mini-game dùng chung vùng nhớ trạng thái để tiết kiệm RAM.

Điểm cao nhất hiện được lưu trong RAM và sẽ mất khi reset hoặc ngắt nguồn.

## Phần cứng

- AK Embedded Base Kit.
- Vi điều khiển STM32L151CBT6.
- Flash: 128 KB.
- RAM cấu hình cho ứng dụng: 16 KB.
- Màn hình OLED đơn sắc, độ phân giải 128 × 64.
- 3 nút điều khiển: DOWN, UP và MODE.
- Buzzer.
- Cổng USB-UART để nạp firmware qua bootloader.

Kit còn hỗ trợ các kết nối mở rộng như RS485, Qwiic và Grove.

[Sơ đồ nguyên lý của kit](hardware/schematic/schematic-ak-embedded-base-kit-version-3.pdf)

![AK Embedded Base Kit](hardware/images/ak-embedded-base-kit-version-3.jpg)

## Điều khiển

Các nút ở hàng dưới, từ trái sang phải: **DOWN — UP — MODE**.

### Menu chọn game

| Thao tác | Chức năng |
| --- | --- |
| DOWN / MODE | Di chuyển lựa chọn theo hướng hiển thị trên menu |
| UP | Chọn game |
| Giữ MODE khoảng 2 giây khi đang trong game | Quay về menu chọn game |

### Trong game

| Game | DOWN | UP | MODE |
| --- | --- | --- | --- |
| Frog Journey | Sang trái | Tiến lên | Sang phải |
| Snake | Rẽ trái so với hướng đang đi | Tạm dừng / tiếp tục | Rẽ phải so với hướng đang đi |
| Brick Pong | Di chuyển thanh đỡ sang trái | Tạm dừng / tiếp tục | Di chuyển thanh đỡ sang phải |
| Road Dodge | Chuyển sang làn trái | Tạm dừng / tiếp tục | Chuyển sang làn phải |
| Flappy Bird | Tạm dừng / tiếp tục | Vỗ cánh | Giữ để về menu |

Với Snake, hai nút rẽ điều khiển theo hướng hiện tại của đầu rắn, không phải hướng trái/phải cố định trên màn hình.

Làm theo hướng dẫn trên màn hình để bắt đầu hoặc chơi lại. Với Frog Journey, chọn độ khó trước khi bắt đầu; độ khó cao hơn có ít HP hơn.

## Môi trường phát triển

Project hiện được phát triển với:

- Ubuntu 24.04 chạy trong VMware.
- GNU Arm Embedded Toolchain.
- GNU Make.
- AK Flash để nạp ứng dụng qua USB-UART.
- STM32CubeProgrammer và ST-Link để phục hồi firmware khi cần.
- VS Code và GitHub Desktop để chỉnh sửa, quản lý phiên bản.

Máy ảo là môi trường phát triển đang sử dụng, không phải yêu cầu bắt buộc của kit.

## Build firmware

Mở terminal tại thư mục `application`:

```bash
cd ~/ak-base-kit-stm32l151/application
make
```

Để xóa kết quả build cũ và biên dịch lại:

```bash
make clean
make
```

Firmware được tạo tại:

```text
application/build_ak-base-kit-stm32l151-application/ak-base-kit-stm32l151-application.bin
```

## Nạp firmware qua USB-UART

### 1. Kết nối kit với Ubuntu

Nếu sử dụng VMware, chuyển thiết bị **QinHeng USB Serial / CH340** sang máy ảo qua:

```text
VM → Removable Devices → QinHeng USB Serial → Connect
```

Kiểm tra cổng:

```bash
ls -l /dev/serial/by-id/
```

Tên thiết bị trên hệ thống phát triển hiện tại:

```text
/dev/serial/by-id/usb-1a86_USB_Serial-if00-port0
```

Tên thực tế có thể khác tùy bộ chuyển đổi USB-UART.

### 2. Đưa kit vào chế độ bootloader

1. Giữ nút MODE.
2. Bấm rồi thả RESET trong khi vẫn giữ MODE.
3. Chờ khoảng 1 giây, sau đó thả MODE.

### 3. Nạp ứng dụng

Chạy tại thư mục `application`:

```bash
ak-flash /dev/serial/by-id/usb-1a86_USB_Serial-if00-port0 \
  build_ak-base-kit-stm32l151-application/ak-base-kit-stm32l151-application.bin \
  0x08003000
```

Thay đường dẫn thiết bị bằng kết quả kiểm tra trên máy của bạn.

Chờ công cụ hoàn tất trước khi rút cáp hoặc reset kit.

## Bản đồ bộ nhớ Flash

| Địa chỉ bắt đầu | Chức năng |
| --- | --- |
| `0x08000000` | Bootloader |
| `0x08002000` | BSF — dữ liệu chia sẻ giữa bootloader và ứng dụng |
| `0x08003000` | Firmware ứng dụng |

**Ứng dụng được liên kết để chạy tại `0x08003000`. Không nạp file ứng dụng vào `0x08000000`, vì sẽ ghi đè bootloader.**

Khi phục hồi bằng ST-Link:

- Nạp bootloader vào `0x08000000`.
- Nạp ứng dụng vào `0x08003000`.
- Nếu dữ liệu BSF đã bị xóa hoặc không còn hợp lệ, nạp lại ứng dụng bằng AK Flash để công cụ cập nhật thông tin firmware cho bootloader.

Không sử dụng Full Chip Erase cho các lần cập nhật ứng dụng thông thường.

## Mức sử dụng bộ nhớ

Kết quả build tham khảo của phiên bản có 5 game:

| Bộ nhớ | Đã sử dụng | Dung lượng khả dụng | Tỷ lệ |
| --- | ---: | ---: | ---: |
| Flash ứng dụng | 64.440 byte | 118.784 byte | 54,25% |
| RAM tĩnh | 13.596 byte | 16.384 byte | 82,98% |

Các số liệu thay đổi theo phiên bản code và cấu hình build. RAM tĩnh không phản ánh toàn bộ mức sử dụng stack khi chương trình chạy.

## Xử lý lỗi thường gặp

| Hiện tượng | Kiểm tra / xử lý |
| --- | --- |
| `Can't open dev path` | Kiểm tra cổng USB, quyền truy cập và việc chuyển thiết bị USB sang máy ảo. |
| `handshake faulted` | Kiểm tra kit đã vào bootloader bằng MODE + RESET và không có chương trình khác đang dùng cổng serial. |
| Cổng đổi từ `ttyUSB0` sang `ttyUSB1` | Kiểm tra lại thiết bị; ưu tiên đường dẫn `/dev/serial/by-id/`. |
| Màn hình đen sau khi ngắt giữa lúc nạp | Nạp lại ứng dụng đầy đủ; sử dụng ST-Link để phục hồi bootloader nếu cần. |
| CubeProgrammer bị treo giao diện | Có thể sử dụng `STM32_Programmer_CLI` thay cho giao diện đồ họa. |
| Thư mục project không truy cập được sau khi mở lại máy ảo | Kiểm tra và mount lại VMware Shared Folders nếu project nằm trong thư mục chia sẻ. |

## Ghi công

Project được phát triển dựa trên **AK Embedded Base Kit** và **AK Framework** của cộng đồng **AK Foundation**.

[<img src="hardware/images/ak-foundation-logo.png" width="240" alt="AK Foundation"/>](https://github.com/the-ak-foundation)

Xin cảm ơn **AK Foundation** và **EPCB** đã phát triển, hỗ trợ phần cứng, framework và tài liệu nền tảng cho kit.

- [AK Foundation](https://github.com/the-ak-foundation)
- [Repository AK Embedded Base Kit gốc](https://github.com/ak-embedded-software/ak-base-kit-stm32l151)
- [AK Flash](https://github.com/ak-embedded-software/ak-flash)
- [EPCB](https://epcb.vn/pages/frontpage)
- [Tài liệu hướng dẫn AK Embedded Software](https://epcb.vn/blogs/ak-embedded-software)

## Giấy phép

Xem file [LICENSE](LICENSE) trong repository.
