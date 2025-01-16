
# ZIP Cracker

## Giới thiệu

ZIP Cracker là một công cụ giúp bẻ khóa mật khẩu của file ZIP bằng cách sử dụng phương pháp brute force hoặc từ điển (dictionary attack). Chương trình hỗ trợ chạy đa luồng, tối ưu hóa hiệu suất với khả năng kiểm soát việc phân phối các lõi CPU, và có thể tiếp tục từ vị trí đã dừng trong lần chạy trước.

## Các tính năng chính

- **Hỗ trợ phương pháp brute force**: Tự động thử mọi kết hợp của mật khẩu trong một không gian tìm kiếm có thể cấu hình.
- **Hỗ trợ từ điển mật khẩu**: Có thể sử dụng một file từ điển chứa các mật khẩu để thử.
- **Đa luồng**: Hỗ trợ chạy song song trên nhiều luồng, cải thiện tốc độ bẻ khóa mật khẩu.
- **Quản lý tiến trình**: Ghi lại vị trí cuối cùng của quá trình bẻ khóa, cho phép tiếp tục từ vị trí đó trong lần chạy tiếp theo.
- **Tùy chọn cấu hình**: Cấu hình dễ dàng số lượng mật khẩu tối đa và số lượng luồng sử dụng.

## Yêu cầu hệ thống

- **Hệ điều hành**: Windows (do sử dụng `windows.h` và các API liên quan đến hệ điều hành Windows).
- **Công cụ biên dịch**: C++11 trở lên.
- **Thư viện bên ngoài**:
  - `zlib` (để thao tác với các file ZIP).
  - `zip.h` (để mở và thao tác với file ZIP).

## Cách cài đặt

1. **Tải mã nguồn**:
    - Bạn có thể tải mã nguồn từ repository này.
  
2. **Cài đặt thư viện cần thiết**:
    - Đảm bảo rằng hệ thống của bạn có thư viện `zlib` và `zip.h` đã được cài đặt.
    - Trên Windows, bạn có thể tải `zlib` và `libzip` từ các nguồn chính thức và đảm bảo rằng các thư viện này có thể được liên kết với chương trình của bạn.

3. **Biên dịch mã nguồn**:
    - Sử dụng trình biên dịch C++ để biên dịch mã nguồn. Ví dụ với `g++`:

      ```bash
      g++ -o zipcracker main.cpp -lz -lzip
      ```

## Cách sử dụng

1. **Chạy chương trình**:
    - Sau khi biên dịch, chạy chương trình qua dòng lệnh.
    - Bạn sẽ được yêu cầu nhập đường dẫn file ZIP, xác định xem có sử dụng từ điển không, cấu hình độ dài mật khẩu và số lượng luồng.

2. **Nhập các thông số**:
    - Chương trình yêu cầu nhập các thông tin như:
      - Đường dẫn đến file ZIP.
      - Đường dẫn đến file từ điển (nếu sử dụng phương pháp từ điển).
      - Độ dài tối đa của mật khẩu (tối đa 8 ký tự).
      - Số lượng luồng cần sử dụng (tối đa với số luồng hiện tại của phần cứng).

3. **Quản lý tiến trình**:
    - Trong khi chương trình đang chạy, bạn có thể nhấn phím `F` để dừng quá trình.
    - Chương trình sẽ lưu lại vị trí tiến trình trong file `LastPoint.txt` để bạn có thể tiếp tục bẻ khóa sau khi dừng.

4. **Chờ kết quả**:
    - Sau khi mật khẩu được tìm thấy hoặc hết các thử nghiệm, chương trình sẽ thông báo kết quả và kết thúc.

## Ví dụ sử dụng

```bash
> zipcracker.exe
===== ZIP Cracker =====
1. Nhap duong dan file ZIP
> C:\path	o\your.zip
2. Ban co muon thu file voi tu dien khong 
Y/N
> Y
Nhap duong dan file tu dien
> C:\path	o\dictionary.txt
...
```

## Thông tin thêm

- Mật khẩu có thể bao gồm các ký tự số, chữ cái viết hoa và viết thường. Bạn có thể điều chỉnh không gian tìm kiếm này bằng cách thay đổi biến `passwordtext`.
- Chương trình hỗ trợ tiếp tục từ điểm đã dừng, nếu phát hiện một lần chạy trước đó chưa hoàn thành.

  
