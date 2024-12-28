#include <bits/stdc++.h>
#include <minizip/unzip.h>
#include <zlib.h>
#include <atomic>
#include <chrono>
#include <thread>
#include <fstream>
#include <filesystem>
#include <windows.h>
#include <csignal>
#include <conio.h>
#include <zip.h>

using namespace std;

atomic<bool> check(false);// Co hieu mat khau
atomic<bool> exiting(false); // Co hieu thoat

vector<thread> threads;

DWORD_PTR affinity_mask ; //Mac dinh su dung 12 cpu

string passwordtext = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
int numpassword = 0; // do dai mat khau
int numthread = 1; //so luong
int passwordlength = 0; //do dai khong gian ky tu
long long maxindex=0; // khong gian mat khau
long long LastIndex=0; //Diem cuoi cung duoc doc
bool checkTuDien= false;
set<long long> LastPoint;

string copyfile[100]; //Toi da 100 file zip copy
string zipfile; // Đường dẫn

// Hàm sao chép file zip
void copyFile(const string& source, const string& destination) {
    string command = "copy \"" + source + "\" \"" + destination + "\" /Y";
    int result =system(command.c_str());

    // Kiểm tra kết quả thực thi
    if (result != 0) {
        throw runtime_error("Failed to copy file: " + source + " to " + destination);
    }
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

    //Che do chay
    unsigned int max_cores = thread::hardware_concurrency();
    unsigned int half_cores = max_cores / 2;
    unsigned int quarter_cores = max_cores / 4;

    cout << "So luong loai CPU cua he thong: " << max_cores << endl;

    cout << "Chon che do chay chuong trinh:" << endl;
    cout << "1. Hieu suat toi da (" << max_cores << " CPU)" << endl;
    cout << "2. Hieu suat trung binh (" << half_cores << " CPU)" << endl;
    cout << "3. Hieu suat thap (" << quarter_cores << " CPU)" << endl;

    int mid;
    cin >> mid;

    // Chọn số lõi CPU dựa trên lựa chọn của người dùng
    // if (mid == 1) {
    //     affinity_mask = (1 << max_cores) - 1;  // Sử dụng tất cả các lõi
    // }
    // else if (mid == 2) {
    //     affinity_mask = (1 << half_cores) - 1;  // Sử dụng một nửa số lõi
    // }
    // else if (mid == 3) {
    //     affinity_mask = (1 << quarter_cores) - 1;  // Sử dụng một phần tư số lõi
    // }
    // else {
    //     cout << "Chon sai che do, su dung che do hieu suat toi da!" << endl;
    //     affinity_mask = (1 << max_cores) - 1;  // Nếu chọn sai, mặc định sử dụng tất cả các lõi
    // }

    //test p core e core

    //affinity_mask= (1<<4)-1;
    //affinity_mask= ((1<<12)-1) ^ ((1<<8)-1);
    //affinity_mask= (((1<<12)-1) ^ ((1<<8)-1)) ^ (1<<4)-1 ^ ((1<<12)-1);
    affinity_mask = 0b110010101010; //4p 2e
    //affinity_mask = 0b000011111010;//6p
    cout << "Da chon che do CPU voi mask: " << affinity_mask << endl;

    //Lay thong tin lastpoin
    ifstream inputFile("LastPoint.txt");
    if (inputFile.is_open()) {
        string mid ;
        getline(inputFile, mid);
        long long num= stoll(mid);
        getline(inputFile, mid);
        cout << "Ban co muon tiep tuc tu lan duyet truoc tai vi tri " << num << " voi gia tri la " << mid << endl;
        cout << "Y/N"<<endl;
        cin>>mid;
        if(mid == "Y") {
            LastIndex = num;
            inputFile.close();
        }
        else if (mid == "N") {
            cout<< "Chuong trinh se chay tu dau" <<endl;
            inputFile.close();
            deleteFile("LastPoint.txt");
        }
    }

    //tao ban sao zip
    for(int i = 0; i < numthread; i++) {
        copyfile[i] = "copy_" + to_string(i) + ".zip"; // Tạo bản sao của file zip cho mỗi luồng
        copyFile(zipfile, copyfile[i]); // Sao chép file zip
    }

    cout << "Bat dau chuong trinh:" << endl;
}

void kiemsoatCPU() {

    // Lay handle cua tien trinh hien tai
    HANDLE process = GetCurrentProcess();

    // Thiết lập CPU affinity cho tiến trình
    if (!SetProcessAffinityMask(process, affinity_mask)) {
        cout << "Loi khi thao tac tai nguyen cpu "<<endl;
        cout << "Chay mac dinh voi hieu suat toi da" << endl;
    }
}

void KiemTraDung() {
    cout << "Nhan F de tam dung chuong trinh neu muon"<<endl;
    string exit;
    while (!check.load()) {
        if (_kbhit()) {
            char ch = _getch();  // lay ki tu an
            if (ch == 'F' || ch == 'f') {
                exiting.store(true);  // Đặt cờ dừng
                cout << "Dang dung..." << endl;
                break;
            }
        }
    }
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
    int err = 0;
    zip_t* archive = zip_open(zipfile.c_str(), ZIP_RDONLY, &err);
    if (!archive) {
        cerr << "Khong mo duoc file ZIP, loi: " << err << std::endl;
        return;
    }

    zip_stat_t st;

    if (zip_stat_index(archive, 0, 0, &st) != 0) {
        cerr << "Khong the lay thong tin file trong ZIP." << std::endl;
        zip_close(archive);
        return;
    }

    while (!check.load() && (start_index < maxindex) && !exiting.load()) {
        string password = indexTransfer(passwordtext, start_index);
        start_index += numthread;

        zip_file_t* zf = zip_fopen_encrypted(archive, st.name, 0, password.c_str());
        if (zf) {
            // Đọc dữ liệu và kiểm tra CRC
            char buffer[4096]; // Đọc mỗi lần 4 KB
            int bytes_read = zip_fread(zf, buffer, sizeof(buffer));
            unsigned long crc = 0;

            while (bytes_read > 0) {
                crc = crc32(crc, (unsigned char*)buffer, bytes_read);
                bytes_read = zip_fread(zf, buffer, sizeof(buffer));
            }

            // So sánh CRC32 với giá trị CRC32 mong muốn
            if (st.crc == crc) {
                check.store(true);
                cout << "Mat khau tim duoc: " << password << endl;
                zip_fclose(zf);
                zip_close(archive);
                deleteFile("LastPoint.txt");
                return;
            }
            zip_fclose(zf);
        }
    }

    if (exiting.load()) {
        LastPoint.insert(start_index);
    }

    zip_close(archive);
}

void TryPassWithDictionary() {

}

void start() {
    // Chạy chương trình với nhiều luồng
    auto start = chrono::high_resolution_clock::now();

    for (int i = 0; i < numthread; i++) {
        long long mid = LastIndex + i;
        threads.emplace_back(TryPassWithBruteForce, copyfile[i], mid, numthread, maxindex, passwordtext);
    }

    thread stopThread(KiemTraDung);
    stopThread.join();

    for (auto &th : threads) {
        th.join();
    }

    for (int i = 0; i < numthread; i++) {
        string copyfile = "copy_" + to_string(i) + ".zip";
        deleteFile(copyfile);
    }

    auto end = chrono::high_resolution_clock::now();

    chrono::duration<double> diff = end - start;

    if (!check.load() && !exiting.load()) cout << "Khong tim thay mat khau" << endl;

    if (exiting.load()) {
        ofstream fout("LastPoint.txt");
        long long Point =*LastPoint.begin();
        string mid = indexTransfer(passwordtext ,Point);
        fout << Point << endl;
        fout << mid << endl;
        fout.close();
        cout << "Mat khau dung lai tai vi tri "<< mid << endl;
        cout << "Da luu thong tin last point vao thu muc LastPoint.txt"<<endl;
    }
    LastPoint.clear();
    threads.clear();

    cout << "Thoi gian giai ma: " << diff.count() << " s" << endl;
}

int main() {
    //Nhap du lieu
    input();

    //Dieu chinh tai nguyen cpu
    kiemsoatCPU();

    //Bat dau chuong trinh
    start();

    system("pause");
    return 0;
}
