#include <bits/stdc++.h>
#include <zlib.h>
#include <atomic>
#include <chrono>
#include <thread>
#include <fstream>
#include <filesystem>
#include <windows.h>
#include <conio.h>
#include <zip.h>

using namespace std;

atomic<bool> check(false);// Co hieu mat khau
atomic<bool> exiting(false); // Co hieu thoat
atomic<int> countthread(0);
atomic<unsigned long long> indexPassword(0);

vector<thread> threads;

DWORD_PTR affinity_mask ; //Mac dinh su dung tat ca cpu
unsigned int coreCPU= 0;
unsigned int threadCPU = 0;
unsigned int pcore=0;
unsigned int ecore=0;

//Che do chay
unsigned int max_cores = thread::hardware_concurrency();
unsigned int half_cores = max_cores / 2;
unsigned int quarter_cores = max_cores / 4;

string passwordtext = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
unsigned int numpassword = 0; // do dai mat khau
unsigned int numthread = 1; //so luong
unsigned long long passwordlength = 0; //do dai khong gian ky tu
unsigned long long maxindex=0; // khong gian mat khau

bool checkTuDien= false;
bool hyperThread = false;//Kiểm tra siêu phân luồng
queue<string> passQueue[20];

string zipfile; // Đường dẫn
string directoryfile; //Đường dẫn file từ điển
ifstream filePassword;

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
            break;
        }

    }
    _pclose(fp);
}

void kiemsoatCPU(unsigned int mid) {
    getInfoCPU();

    //cout << coreCPU << " " << threadCPU << endl;
    //mid=11;
    if(coreCPU*2 == threadCPU){
        pcore = coreCPU;
        hyperThread = true;
    }
    else if(coreCPU == threadCPU){
        pcore = coreCPU;
        ecore = 0;
        hyperThread= false;
    }
    else{
        ecore = threadCPU - coreCPU;
        pcore = coreCPU - ecore;
        hyperThread = true;
    }

    //cout<< pcore <<" " <<ecore<<endl;

    if(!hyperThread){ // neu khong ho tro sieu phan luong
        affinity_mask =0;
        for(unsigned int i = threadCPU; i > threadCPU - mid ; i--){
            affinity_mask += 1<<(i-1);
        }
    }
    else{
        affinity_mask =0;
        if(mid <= pcore){
            for(unsigned int i = pcore; i > pcore - mid ; i-- ){
                affinity_mask += 1<<(i*2 - 1);
            }
        }

        else if(mid > pcore && mid <= (pcore + ecore)){
            for(unsigned int i = pcore; i > 0 ; i-- ){
                affinity_mask += 1<<(i*2 - 1);
            }
            for(unsigned int i= threadCPU; i > threadCPU - mid + pcore ; i--){
                affinity_mask += 1 << (i-1);
            }
        }

        else if((mid > pcore + ecore) && mid <= threadCPU){
            for(unsigned int i = pcore; i > 0 ; i-- ){
                affinity_mask += 1 << (i*2 - 1);
            }
            for(unsigned int i= threadCPU; i > threadCPU - ecore ; i--){
                affinity_mask += 1 << (i-1);
            }

            for(unsigned int i = pcore; i > 2 * pcore + ecore - mid; i-- ){
                affinity_mask += 1 << (i*2 - 2);
            }

        }
    }

    //test p core e core

    //affinity_mask= (1<<4)-1;
    //affinity_mask= ((1<<12)-1) ^ ((1<<8)-1);
    //affinity_mask= (((1<<12)-1) ^ ((1<<8)-1)) ^ (1<<4)-1 ^ ((1<<12)-1);
    //affinity_mask = 0b110010101010; //4p 2e
    //affinity_mask = 0b000011111010;//6p

    // Lay handle cua tien trinh hien tai
    HANDLE process = GetCurrentProcess();

    // Thiết lập CPU affinity cho tiến trình
    if (!SetProcessAffinityMask(process, affinity_mask)) {
        cout << "Loi khi thao tac tai nguyen cpu "<<endl;
        cout << "Chay mac dinh voi hieu suat toi da" << endl;
    }
}

void runProgressBar(unsigned long long maxindex) {
    unsigned long long index;
    int percentage = 0;

    ostringstream oss;

    while (indexPassword.load() < maxindex && !check.load() && !exiting.load()) {
        index = indexPassword.load();
        percentage = (index*100) / maxindex;

        oss.str(""); // Xóa chuỗi cũ
        oss.clear();
        oss << percentage << " % (" << index << "/" << maxindex << ")";
        cout << "\r" << oss.str();
        cout.flush();

        this_thread::sleep_for(chrono::milliseconds(100));
    }
    if(check.load() || countthread.load()==numthread) {
        cout << "100 %" <<endl;
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


//    cout << "Nhap so luong ban muon thuc hien chuong trinh (luong toi da la 12): " << endl;
//    cin>>numthread;

    //nhap so luong loi su dung
    cout << "So luong loai CPU cua he thong: " << max_cores << endl;

    cout << "Chon che do chay chuong trinh:" << endl;
    cout << "1. Hieu suat toi da (" << max_cores << " CPU)" << endl;
    cout << "2. Hieu suat trung binh (" << half_cores << " CPU)" << endl;
    cout << "3. Hieu suat thap (" << quarter_cores << " CPU)" << endl;

    int mid;
    cin >> mid;

    // numthread= max_cores / mid;

    // Chọn số lõi CPU dựa trên lựa chọn của người dùng
    if (mid == 1) {
        kiemsoatCPU(max_cores); // Sử dụng tất cả các lõi
        numthread = max_cores - 1;
    }
    else if (mid == 2) {
        kiemsoatCPU(half_cores);// Sử dụng một nửa số lõi
        numthread = half_cores -1 ;
    }
    else if (mid == 3) {
        kiemsoatCPU(quarter_cores);  // Sử dụng một phần tư số lõi
        numthread = half_cores -1;
    }
    else {
        cout << "Chon sai che do, su dung che do hieu suat toi da!" << endl;
        kiemsoatCPU(max_cores);  // Nếu chọn sai, mặc định sử dụng tất cả các lõi
        numthread = max_cores - 1;
    }

    cout << "Da chon che do CPU voi mask: " << affinity_mask << endl;

    //check tu dien
    cout << "Ban co muon thu mat khau voi tu dien khong" << endl;
    cout << "Y/N" <<endl;
    string input;
    cin>>input;
    if(input=="Y") {
        checkTuDien = true;

        cout << "Nhap duong dan file tu dien" <<endl;
        cin >> directoryfile;

        filePassword.open(directoryfile);
        if(!filePassword.is_open()){
            cout << "\nLoi mo file tu dien" <<endl;
            cout<<endl;

            checkTuDien=false;
        }

        string line;
        unsigned long long lineindex = 0;

        while (filePassword>>line) {
            passQueue[lineindex % numthread].push(line);
            lineindex++;
        }
    }



    //Xu ly sau nhap lieu
    passwordlength = passwordtext.length();
    maxindex = (long long)pow(passwordlength, numpassword);


    //Lay thong tin lastpoin
    ifstream inputFile("LastPoint.txt");
    if (inputFile.is_open()) {
        string mid;
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

    cout << "Bat dau chuong trinh:" << endl;
}

void KiemTraDung() {
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
string indexTransfer(string &passwordtext, unsigned long long i) {
    unsigned long long size = passwordlength;
    unsigned long long index = 0;
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
            long long bytes_read = zip_fread(zf, buffer, sizeof(buffer));
            unsigned long crc = 0;

            while (bytes_read > 0) {
                crc = crc32(crc, (unsigned char*)buffer, bytes_read);
                bytes_read = zip_fread(zf, buffer, sizeof(buffer));
            }

            // So sánh CRC32 với giá trị CRC32 mong muốn
            if (st.crc == crc) {
                check.store(true);
                cout << "\nMat khau tim duoc: " << password << endl;
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

    while (!check.load() && !passQueue[idnumthread].empty()) {
        password=passQueue[idnumthread].front();
        passQueue[idnumthread].pop();

        zip_file_t* zf = zip_fopen_encrypted(archive, st.name, 0, password.c_str());
        if (zf) {
            // Đọc dữ liệu và kiểm tra CRC
            char buffer[4096]; // Đọc mỗi lần 4 KB
            long long bytes_read = zip_fread(zf, buffer, sizeof(buffer));
            unsigned long crc = 0;

            while (bytes_read > 0) {
                crc = crc32(crc, (unsigned char*)buffer, bytes_read);
                bytes_read = zip_fread(zf, buffer, sizeof(buffer));
            }

            // So sánh CRC32 với giá trị CRC32 mong muốn
            if (st.crc == crc) {
                check.store(true);
                cout << "\nMat khau tim duoc: " << password << endl;
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
            threads.emplace_back(TryPassWithDictionary, zipfile, i);
        }

        for (auto &th : threads) {
            th.join();
        }

        threads.clear();

        auto endTudien = chrono::high_resolution_clock::now();

        chrono::duration<double> diffTudien = endTudien - startTudien;

        cout << "Giai ma tu dien voi thoi gian " << diffTudien.count() << "s" << endl;

    }


    if (!check.load()) { // neu tim dc mat khau
        if(checkTuDien) cout << "Khong tim thay mat khau voi tu dien" << endl;

        auto start = chrono::high_resolution_clock::now();
        cout << "Dang thu voi bung no to hop " << endl;
        for (int i = 0; i < numthread; i++) {
            threads.emplace_back(TryPassWithBruteForce, zipfile, maxindex, passwordtext);
        }

        cout << "Nhan F de tam dung chuong trinh neu muon"<<endl;
        thread hienphantram(runProgressBar,maxindex);
        thread stopThread(KiemTraDung);

        hienphantram.join();
        stopThread.join();

        for (auto &th : threads) {
            th.join();
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

int main(){
    //Nhap du lieu
    input();

    //Bat dau chuong trinh
    start();

    system("pause");
    return 0;
}
