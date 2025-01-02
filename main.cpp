#include <bits/stdc++.h>
#include <zlib.h>
#include <atomic>
#include <chrono>
#include <thread>
#include <fstream>
#include <filesystem>
#include <windows.h>
#include <conio.h>
#include <csignal>
#include <zip.h>

using namespace std;

atomic<bool> check(false);// Co hieu mat khau
atomic<bool> exiting(false); // Co hieu thoat
atomic<int> countthread(0);
atomic<int> indexPassword(0);

vector<thread> threads;

DWORD_PTR affinity_mask ; //Mac dinh su dung 12 cpu
int coreCPU= 0;
int threadCPU = 0;

string passwordtext = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
int numpassword = 0; // do dai mat khau
int numthread = 1; //so luong
int passwordlength = 0; //do dai khong gian ky tu
long long maxindex=0; // khong gian mat khau
bool checkTuDien= false;
queue<string> passQueue[20];

string copyfile[100]; //Toi da 100 file zip copy
string zipfile; // Đường dẫn

void getInfoCPU() {
    FILE *fp;
    char buffer[128];

    // Chạy lệnh CMD và đọc kết quả
    fp = _popen("wmic cpu get NumberOfCores, NumberOfLogicalProcessors", "r");
    if (fp == nullptr) {
        perror("Loi khi mo CMD");
        return ;
    }

    while (fgets(buffer, sizeof(buffer), fp) != nullptr) {
        if (strstr(buffer, "NumberOfCores") == nullptr && strstr(buffer, "NumberOfLogicalProcessors") == nullptr) {
            sscanf(buffer, "%d %d", &coreCPU, &threadCPU);
        }

    }

    _pclose(fp);

}

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
    cin>>zipfile;

    do{
        // cin>>numpassword;
        cout << "Nhap do dai toi da mat khau muon thu (toi da 8): " << endl;
        cin>>numpassword;
    }while (numpassword > 8 || numpassword < 1);


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
    if (mid == 1) {
        affinity_mask = (1 << max_cores) - 1;  // Sử dụng tất cả các lõi
    }
    else if (mid == 2) {
        affinity_mask = (1 << half_cores) - 1;  // Sử dụng một nửa số lõi
    }
    else if (mid == 3) {
        affinity_mask = (1 << quarter_cores) - 1;  // Sử dụng một phần tư số lõi
    }
    else {
        cout << "Chon sai che do, su dung che do hieu suat toi da!" << endl;
        affinity_mask = (1 << max_cores) - 1;  // Nếu chọn sai, mặc định sử dụng tất cả các lõi
    }

    //test p core e core

    //affinity_mask= (1<<4)-1;
    //affinity_mask= ((1<<12)-1) ^ ((1<<8)-1);
    //affinity_mask= (((1<<12)-1) ^ ((1<<8)-1)) ^ (1<<4)-1 ^ ((1<<12)-1);
    //affinity_mask = 0b110010101010; //4p 2e
    //affinity_mask = 0b000011111010;//6p
    cout << "Da chon che do CPU voi mask: " << affinity_mask << endl;


    //check tu dien
    cout << "Ban co muon thu mat khau voi tu dien khong" << endl;
    cout << "Y/N" <<endl;
    string input;
    cin>>input;
    if(input=="Y") {
        checkTuDien = true;
    }

    if (checkTuDien) {
        ifstream directoryFile("bungnotohop.txt");
        string line;
        long long lineindex = 0;

        while (directoryFile>>line) {
            passQueue[lineindex % numthread].push(line);
            lineindex++;
        }
    }


    //Lay thong tin lastpoin
    ifstream inputFile("LastPoint.txt");
    if (inputFile.is_open()) {
        string mid ;
        getline(inputFile, mid);
        if(mid == zipfile) {
            getline(inputFile, mid);
            long long num = stoll(mid);
            getline(inputFile, mid);
            cout << "Ban co muon tiep tuc tu lan duyet truoc tai vi tri " << num << " voi gia tri la " << mid << endl;
            cout << "Y/N" << endl;
            cin >> mid;
            if (mid == "Y") {
                indexPassword.store(num);
                inputFile.close();
            } else if (mid == "N") {
                cout << "Chuong trinh se chay tu dau" << endl;
                inputFile.close();
                deleteFile("LastPoint.txt");
            }
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
    while (!check.load() && countthread.load() != numthread) {
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
void TryPassWithBruteForce(string zipfile, long long maxindex, string passwordtext) {
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


    long long start_index= indexPassword.load();

    while (!check.load() && (start_index < maxindex) && !exiting.load()) {
        start_index = indexPassword.fetch_add(1);
        string password = indexTransfer(passwordtext, start_index);

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

    if (start_index >= maxindex) {
        countthread.fetch_add(1);
    }

    zip_close(archive);
}

void TryPassWithDictionary(string zipfile, int idnumthread) {
    string password;
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

    while (!check.load() && passQueue[idnumthread].size() > 0) {
        password=passQueue[idnumthread].front();
        passQueue[idnumthread].pop();

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
                return;
            }
            zip_fclose(zf);
        }
    }

    zip_close(archive);
}

void start() {
    // Chạy chương trình với nhiều luồng

    if (checkTuDien) {
        cout << "Dang thu voi tu dien" <<endl;
        auto startTudien = chrono::high_resolution_clock::now();

        for (int i = 0; i < numthread; i++) {
            threads.emplace_back(TryPassWithDictionary, copyfile[i], i);
        }

        for (auto &th : threads) {
            th.join();
        }

        threads.clear();

        auto endTudien = chrono::high_resolution_clock::now();

        chrono::duration<double> diffTudien = endTudien - startTudien;
        cout<<"Giai ma tu dien voi thoi gian " << diffTudien.count()<<endl;

    }


    if (!check.load()) { // neu tim dc mat khau
        auto start = chrono::high_resolution_clock::now();
        cout << "Dang thu voi bung no to hop " << endl;
        for (int i = 0; i < numthread; i++) {
            threads.emplace_back(TryPassWithBruteForce, copyfile[i], maxindex, passwordtext);
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

        cout << "Thoi gian giai ma: " << diff.count() << " s" << endl;
    }

    if (!check.load() && !exiting.load()) cout << "Khong tim thay mat khau" << endl;

    if (exiting.load()) {
        ofstream fout("LastPoint.txt");
        long long Point =indexPassword.load();
        string mid = indexTransfer(passwordtext ,Point);
        fout << zipfile << endl;
        fout << Point << endl;
        fout << mid << endl;
        fout.close();
        cout << "Mat khau dung lai tai vi tri "<< Point << endl;
        cout << "Co ki tu la " << mid <<endl;
        cout << "Da luu thong tin last point vao thu muc LastPoint.txt"<<endl;
    }

    threads.clear();

}

int main() {
     getInfoCPU();
     cout<<coreCPU<<endl;
     cout<<threadCPU<<endl;


    //Nhap du lieu
    input();

    //Dieu chinh tai nguyen cpu
    kiemsoatCPU();

    //Bat dau chuong trinh
    start();

    system("pause");
    return 0;
}
