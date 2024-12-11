#include <bits/stdc++.h>
#include <minizip/unzip.h>
#include <zlib.h>
#include <atomic>
#include <chrono>
#include <thread>
#include <fstream>
#include <filesystem>

using namespace std;

namespace fs = std::filesystem;

atomic<bool> check(false);

string indextranfer(string passwordtext, long long i) {
    long long size = passwordtext.size();
    string mid = "";
    while (i != 0) {
        long long index = i % size;
        mid += passwordtext[index];
        i = i / size;
    }
    reverse(mid.begin(), mid.end());
    return mid;
}

// Hàm sao chép file zip
void copyFile(const string& source, const string& destination) {
    ifstream src(source, ios::binary);
    ofstream dst(destination, ios::binary);
    dst << src.rdbuf();
}

// Hàm xóa file tạm
void deleteFile(const string& filepath) {
    if (fs::exists(filepath)) {
        fs::remove(filepath);
    }
}

void trypass(string zipfile, long long start_index, int numthread, long long maxindex, string passwordtext) {
    string copyfile = "copy_" + to_string(start_index) + ".zip"; // Tạo bản sao của file zip cho mỗi luồng
    copyFile(zipfile, copyfile); // Sao chép file zip

    unzFile file = unzOpen(copyfile.c_str()); // Mở bản sao file zip
    if (file == NULL) {
        cout << "Khong mo duoc file zip" << endl;
        deleteFile(copyfile); // Xóa file sao chép nếu không mở được
        return;
    }

    if (unzGoToFirstFile(file) != UNZ_OK) {
        cout << "File rong hoac loi file";
        deleteFile(copyfile); // Xóa file sao chép nếu không thể truy cập file
        return;
    }

    //lay thog tin crc32
    unz_file_info file_info;
    unzGetCurrentFileInfo(file, &file_info, NULL, 0, NULL, 0, NULL, 0);


    // Kiểm tra mật khẩu
    while (!check.load() && start_index < maxindex) {
        string password = indextranfer(passwordtext, start_index);
        start_index += numthread;

        if (unzOpenCurrentFilePassword(file, password.c_str()) == UNZ_OK) {
            // Đọc dữ liệu và kiểm tra CRC
            char buffer[1024];
            int bytes_read = unzReadCurrentFile(file, buffer, sizeof(buffer));
            long long crc = 0;

            while (bytes_read > 0) {
                crc = crc32(crc, (unsigned char*)buffer, bytes_read);
                bytes_read = unzReadCurrentFile(file, buffer, sizeof(buffer));
            }

            if (file_info.crc != crc) {
                unzCloseCurrentFile(file);
                continue;
            }

            check.store(true);
            cout << "Mat khau tim duoc: " << password << endl;
            unzCloseCurrentFile(file);
            unzClose(file);
            deleteFile(copyfile); // Xóa file sao chép sau khi tìm được mật khẩu
            return;
        }
        unzCloseCurrentFile(file);
    }
    unzClose(file);
    deleteFile(copyfile); // Xóa file sao chép nếu không tìm thấy mật khẩu
}

int main() {
    string zipfile = "D:/testzip/huytestZZZZ.zip"; // Đường dẫn
    string passwordtext = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

    int numpassword = 4;

    // Nhập số lượng luồng
    int numthread;
    cout << "Nhap so luong muon chay:";
    cin >> numthread;

    int passwordlength = passwordtext.length();
    long long maxindex = pow(passwordlength, numpassword);

    cout << "Bat dau chuong trinh:" << endl;
    auto start = chrono::high_resolution_clock::now();

    // Chạy chương trình với nhiều luồng
    vector<thread> threads;
    for (int i = 0; i < numthread; i++) {
        threads.emplace_back(trypass, zipfile, i, numthread, maxindex, passwordtext);
    }

    for (auto &th : threads) {
        th.join();
    }

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> diff = end - start;

    if (!check.load()) cout << "Khong tim thay mat khau" << endl;

    cout << "Thoi gian giai ma: " << diff.count() << " s" << endl;

    return 0;
}
