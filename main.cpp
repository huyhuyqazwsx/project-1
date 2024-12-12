#include <bits/stdc++.h>
#include <minizip/unzip.h>
#include <zlib.h>
#include <atomic>
#include <chrono>
#include <thread>
#include <fstream>
#include <filesystem>

using namespace std;

atomic<bool> check(false);// Co hieu

string const passwordtext = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
int numpassword = 0; // do dai mat khau
int numthread = 1; //so luong
int passwordlength = 0; //do dai khong gian ky tu
long long maxindex=0; // khong gian mat khau
string zipfile; // Đường dẫn

void input() {
    // Nhập du lieu vao
    cout<<"Nhap duong dan file zip: "<<endl;
    getline(cin, zipfile);

    cout << "Nhap do dai toi da mat khau muon thu: " << endl;
    cin>>numpassword;

    cout << "Nhap so luong ban muon thuc hien chuong trinh (luong toi da la 12): " << endl;
    cin>>numthread;

    //Xu ly sau nhap lieu
    passwordlength = passwordtext.length();
    maxindex = pow(passwordlength, numpassword);

    cout << "Bat dau chuong trinh:" << endl;
}

string indexTransfer(string &passwordtext, long long i) {
    long long size = passwordtext.size();
    long long index = 0;
    string mid = "";
    while (i != 0) {
        index = i % size;
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
    if (filesystem::exists(filepath)) {
        filesystem::remove(filepath);
    }
}

void TryPassWithBruteForce(string zipfile, long long start_index, int numthread, long long maxindex, string passwordtext) {
    string copyfile = "copy_" + to_string(start_index) + ".zip"; // Tạo bản sao của file zip cho mỗi luồng
    copyFile(zipfile, copyfile); // Sao chép file zip

    unzFile file = unzOpen(copyfile.c_str());

    //mo file zip
    if (file == NULL) {
        cout << "Khong mo duoc file zip" << endl;
        deleteFile(copyfile);
        return;
    }
    //mo file dau tien
    if (unzGoToFirstFile(file) != UNZ_OK) {
        cout << "File rong hoac loi file";
        deleteFile(copyfile);
        return;
    }

    //lay thog tin crc32
    unz_file_info file_info;
    unzGetCurrentFileInfo(file, &file_info, NULL, 0, NULL, 0, NULL, 0);

    // Kiểm tra mật khẩu
    while (!check.load() && start_index < maxindex) {
        //lay mat khau
        string password = indexTransfer(passwordtext, start_index);
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

            if (file_info.crc == crc) {
                check.store(true);
                cout << "Mat khau tim duoc: " << password << endl;
                unzCloseCurrentFile(file);
                unzClose(file);
                deleteFile(copyfile); // Xóa file sao chép sau khi tìm được mật khẩu
                return;
            }
            unzCloseCurrentFile(file);
        }
        unzCloseCurrentFile(file);
    }
    unzClose(file);
    deleteFile(copyfile); // Xóa file sao chép nếu không tìm thấy mật khẩu
}

int main() {
    input();

    // Chạy chương trình với nhiều luồng
    auto start = chrono::high_resolution_clock::now();

    vector<thread> threads;
    for (int i = 0; i < numthread; i++) {
        threads.emplace_back(TryPassWithBruteForce, zipfile, i, numthread, maxindex, passwordtext);
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
