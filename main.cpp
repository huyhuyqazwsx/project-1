#include <bits/stdc++.h>
#include <minizip/unzip.h>
#include <zlib.h>
#include <atomic>
#include <chrono>
#include <thread>
using namespace std;

atomic<bool> check(false);

string indextranfer(string passwordtext, long long i){
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

void trypass(string zipfile, long long start_index, int numthread, long long maxindex, string passwordtext) {
    unzFile file = unzOpen(zipfile.c_str());

    if (file == NULL) {
        cout << "Khong mo duoc file zip" << endl;
        return;
    }

    if (unzGoToFirstFile(file) != UNZ_OK) {
        cout << "File rong hoac loi file";
        return;
    }

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

            unz_file_info file_info;
            unzGetCurrentFileInfo(file, &file_info, NULL, 0, NULL, 0, NULL, 0);
            if (file_info.crc == crc) {
                cout << "Mat khau tim duoc: " << password << endl;
                check.store(true);
                unzCloseCurrentFile(file);
                unzClose(file);
                return;
            }
            unzCloseCurrentFile(file);
        }
        unzCloseCurrentFile(file);
    }
    unzClose(file);
}

int main() {
    string zipfile = "D:/testzip/huytestZZZZ.zip"; // Đường dẫn
    string passwordtext = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

    int numpassword = 4;

    // Nhập số lượng luồng
    int numthread;
    cout << "Nhap so luong muon chay: ";
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
