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

string copyfile[100]; //Toi da 100 file zip copy
string zipfile; // Đường dẫn

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

//Nhap lieu
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

    //tao ban sao zip
    for(int i = 0; i < numthread; i++) {
        copyfile[i] = "copy_" + to_string(i) + ".zip"; // Tạo bản sao của file zip cho mỗi luồng
        copyFile(zipfile, copyfile[i]); // Sao chép file zip
    }

    cout << "Bat dau chuong trinh:" << endl;
}

//Chuyen doi index sang password
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

//Thu mat khau bang bung no to hop
void TryPassWithBruteForce(string zipfile, long long start_index, int numthread, long long maxindex, string passwordtext) {

    unzFile file = unzOpen(zipfile.c_str());

    //mo file zip
    if (file == NULL) {
        cout << "Khong mo duoc file zip" << endl;
        return;
    }
    //mo file dau tien
    if (unzGoToFirstFile(file) != UNZ_OK) {
        cout << "File rong hoac loi file";
        return;
    }

    //lay thog tin crc32
    unz_file_info file_info;
    if (unzGetCurrentFileInfo(file, &file_info, NULL, 0, NULL, 0, NULL, 0)!=UNZ_OK) {
        unzClose(file);
        return;
    }

    // Kiểm tra mật khẩu
    while (!check.load() && start_index < maxindex) {
        //lay mat khau
        string password = indexTransfer(passwordtext, start_index);
        start_index += numthread;

        if (unzOpenCurrentFilePassword(file, password.c_str()) == UNZ_OK) {
            // Đọc dữ liệu và kiểm tra CRC
            char buffer[1024]; //Doc moi lan 1 kb
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
                return;
            }
            unzCloseCurrentFile(file);
        }
        unzCloseCurrentFile(file);
    }
    unzClose(file);
}

int main() {
    //Nhap du lieu
    input();

    // Chạy chương trình với nhiều luồng
    auto start = chrono::high_resolution_clock::now();

    vector<thread> threads;
    for (int i = 0; i < numthread; i++) {
        threads.emplace_back(TryPassWithBruteForce, copyfile[i], i, numthread, maxindex, passwordtext);
    }

    for (auto &th : threads) {
        th.join();
    }

    for (int i = 0; i < numthread; i++) {
        string copyfile = "copy_" + to_string(i) + ".zip";
        deleteFile(copyfile);
    }

    auto end = chrono::high_resolution_clock::now();

    chrono::duration<double> diff = end - start;

    if (!check.load()) cout << "Khong tim thay mat khau" << endl;

    cout << "Thoi gian giai ma: " << diff.count() << " s" << endl;

    return 0;
}
