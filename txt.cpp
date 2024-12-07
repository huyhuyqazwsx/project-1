#include <iostream>
#include <string>
#include <vector>

// Hàm sinh bùng nổ tổ hợp các mật khẩu
std::string generatePassword(int length, const std::string& charset) {
    static std::vector<int> indices(length, 0);  // Chỉ số cho từng vị trí trong mật khẩu
    std::string password;

    // Sinh mật khẩu
    for (int i = 0; i < length; ++i) {
        password += charset[indices[i]];  // Dùng chỉ số để lấy ký tự từ charset
    }

    // Tăng chỉ số của mật khẩu giống như một bộ đếm hệ cơ số
    for (int i = length - 1; i >= 0; --i) {
        if (indices[i] < charset.size() - 1) {
            indices[i]++;
            break;
        } else {
            indices[i] = 0;  // Reset chỉ số hiện tại và tiếp tục với chỉ số kế tiếp
        }
    }

    return password;
}



int main() {
    const std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";  // Charset chỉ bao gồm các ký tự 'a' đến 'z'
    const int maxLength = 4;  // Độ dài tối đa của mật khẩu

    for (int length = 1; length <= maxLength; ++length) {
        // Mỗi lần, tạo một mật khẩu có độ dài khác nhau từ 1 đến maxLength
        std::string password = generatePassword(length, charset);
        std::cout << password << std::endl;  // In ra mật khẩu
    }

    return 0;
}
