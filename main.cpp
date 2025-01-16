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
atomic<bool> checkQueue(false); //co hieu hang doi
atomic<bool> checkEof(false);
atomic<int> countthread(0);
atomic<unsigned long long> indexPassword(0);
atomic<int> indexPasswordQueue(0);

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
unsigned int const maxIndexQueue = 10000;
unsigned int midMaxIndexQueue = maxIndexQueue;

bool checkTuDien= false;
bool hyperThread = false;//Kiểm tra siêu phân luồng

string passQueue[maxIndexQueue];//Hang doi mat khau cho tu dien

string zipfile; // Đường dẫn
string directoryfile; //Đường dẫn file từ điển
string truePassword;
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

    //bo qua dong dau
    fgets(buffer, sizeof(buffer) , fp);

    //lay thong tin
    fgets(buffer, sizeof(buffer) , fp);
    istringstream iss(buffer);
    iss >> coreCPU >> threadCPU;

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
        pcore = threadCPU - coreCPU;
        ecore = coreCPU - pcore;
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


    //numthread = 4;

    //4p dau
    //affinity_mask= (1<<4)-1;


    //4p giua
    //affinity_mask= (((1<<12)-1) ^ ((1<<8)-1)) ^ (1<<4)-1 ^ ((1<<12)-1);

    //4e
    //affinity_mask= ((1<<12)-1) ^ ((1<<8)-1);

    //4p xen ke
    //affinity_mask = 0b000010101010;

    //macdinh
    //affinity_mask = 0b111111111111;


    //affinity_mask = 0b110010101010; //4p 2e
    //affinity_mask = 0b000011111010;//6p

    // Lay handle cua tien trinh hien tai
    HANDLE process = GetCurrentProcess();

    SetPriorityClass(process, REALTIME_PRIORITY_CLASS);
    // Thiết lập CPU affinity cho tiến trình
    if (!SetProcessAffinityMask(process, affinity_mask)) {
        cout << "Loi khi thao tac tai nguyen cpu "<<endl;
        cout << "Chay mac dinh voi hieu suat toi da" << endl;
    }

}

void runProgressBar(unsigned long long maxIndex) {
    unsigned long long index = indexPassword.load();
    float percentage = 0;
    string mid;

    while ( index < maxIndex && !check.load() && !exiting.load()) {
        this_thread::sleep_for(chrono::milliseconds(500));

        index = min( indexPassword.load(), maxIndex );
        percentage = (float)index / (float)maxIndex;
        percentage *= 100;

        mid = "\033[2K\r" + to_string(percentage) + " %" + " (" + to_string(index) + "/" + to_string(maxIndex) + ")";
        cerr << mid;

        if(index == maxIndex){
            cout << "\nDang ket thuc..." << endl;
        }
    }

    if(check.load() || countthread.load()==numthread) {
        mid = "\033[2K\r100 % (" + to_string(maxIndex) + "/" + to_string(maxIndex) + ")";
        cerr <<  mid;
    }

    cout << endl;
}

// Hàm xóa file tạm
void deleteFile(const string& filepath) {
    if (filesystem::exists(filepath)) {
        filesystem::remove(filepath);
    }
}

void input(){
    string mid;
    zip_t* archive;


    cout << "===== ZIP Cracker =====\n";
    cout << "1. Nhap duong dan file ZIP\n";
    do{
        getline(cin, zipfile);
        int err = 0;
        archive = zip_open(zipfile.c_str(), ZIP_RDONLY, &err);
        if(!archive){
            cout << "Loi duong dan file" << endl;
            cout << "Hay thao tac lai" << endl;
        }
        else{
            cout << "Nhap file thanh cong voi duong dan " << zipfile <<endl;
            break;
        }
    }while(true);

    //Lay thong tin lastpoin
    ifstream inputFile("LastPoint.txt");
    if (inputFile.is_open()) {
        string line;
        getline(inputFile, line);
        if(line == zipfile) {
            getline(inputFile, line);
            long long num = stoll(line);
            getline(inputFile, line);
            cout << "\nPhat hien lan luu truoc cua file " << zipfile <<endl;
            cout << "Ban co muon tiep tuc tu lan duyet truoc tai vi tri " << num << " voi gia tri la " << line << endl;
            cout << "Y/N" << endl;
            cin >> line;

            if (line == "Y" || line =="y") {
                cout << "Da chon tiep tuc duyet voi lan duyet truoc..." << endl;
                indexPassword.store(num);
                inputFile.close();
            }
            else{
                cout << "Chuong trinh se chay tu dau " << endl;
                inputFile.close();
                deleteFile("LastPoint.txt");
            }
        }
    }

    cout << "\n2. Ban co muon thu file voi tu dien khong " <<endl;
    cout << "Y/N" <<endl;
    string input;
    cin>>input;
    if(input == "Y" || input == "y") {
        checkTuDien = true;
        cout << "Nhap duong dan file tu dien" <<endl;
        cin >> directoryfile;
        filePassword.open(directoryfile);

        while(!filePassword.is_open()){
            checkTuDien=false;
            cout << "\nLoi mo file tu dien" <<endl;
            cout << "Neu muon bo qua hay nhap Y/y" <<endl;

            cin >> directoryfile;
            if(directoryfile =="y" || directoryfile == "Y") break;
            else{
                cout << "Nhap duong dan file tu dien" <<endl;
                cin >> directoryfile;
                filePassword.open(directoryfile);
            }
        }

        if(filePassword.is_open()){
            cout << "Mo tep tu dien thanh cong voi file tu dien " << directoryfile <<endl;
            checkTuDien = true;
            string line;
            for(int i = 0 ;i < maxIndexQueue ; i++ ){
                if(getline(filePassword, line)){
                    passQueue[i] = line;
                }
                else{
                    checkEof.store(true);
                    midMaxIndexQueue=i;
                }
            }
        }
        else{
            cout << "Da bo qua phuong phap thu voi tu dien" << endl;
        }
    }
    else{
        cout << "Da bo qua tu dien" << endl;
    }

    cout << "\n3.Hien tai mat khau toi da mac dinh la 4 ky tu (toi da 8)" << endl;
    cout << "Voi bo ky tu san co la : " << passwordtext <<endl;
    cout << "Ban co muon gioi han lai do dai ky tu khong" << endl;
    cout << "Y/N" << endl ;
    cin>>mid;

    if(mid == "N" || mid == "n"){
        cout << "\nChuong trinh se chay voi do dai ky tu toi da la 4 va bo ky tu" << endl << passwordtext <<endl;
        numpassword = 4;
    }
    else{
        do{
            cout << "\nMoi nhap lai so ky tu mat khau toi da muon thu" << endl;
            cin >> numpassword;
            if(numpassword > 8 || numpassword < 1){
                cout << "Loi cai dat voi do dai ky tu";
            }
        }while(numpassword > 8 || numpassword < 1);

        cout << "Da chon do dai voi kich thuoc " << numpassword <<" ky tu" <<endl;
    }

    cout << "\n4.Chon che do chay" <<endl;
    cout << "So luong loai CPU cua he thong hien tai: " << max_cores << " CPU" << endl;
    cout << "1. Hieu suat toi da (" << max_cores - 2 << " CPU)"  << endl;
    cout << "2. Hieu suat trung binh (" << half_cores << " CPU)" << endl;
    cout << "3. Hieu suat thap (" << quarter_cores << " CPU)" << endl;


    cin >> mid;
    if (mid == "1") {

        kiemsoatCPU(max_cores - 2); // Sử dụng tất cả các lõi
        numthread = max_cores - 2;
        cout << "\nDa chon hieu suat toi da voi so luong mac dinh " << numthread <<endl;
    }
    else if (mid == "2") {
        kiemsoatCPU(half_cores);// Sử dụng một nửa số lõi
        numthread = half_cores;
        cout << "\nDa chon hieu suat trung binh voi so luong mac dinh " << numthread <<endl;
    }
    else if (mid == "3") {
        kiemsoatCPU(quarter_cores);  // Sử dụng một phần tư số lõi
        numthread = quarter_cores;
        cout << "\nDa chon hieu suat thap voi so luong mac dinh " << numthread <<endl;
    }
    else {
        cout << "\nChon sai che do, su dung che do hieu suat toi da!" << endl;
        kiemsoatCPU(max_cores);  // Nếu chọn sai, mặc định sử dụng tất cả các lõi
        numthread = max_cores - 2;
        cout << "So luong mac dinh dang chay" << numthread;
    }

    cout << "Da chon che do CPU voi mask: " << affinity_mask << endl;
    cout << "Ban co muon chon lai so luong khong" <<endl;
    cout << "Y/N" << endl;

    cin >> mid ;
    if(mid == "N" || mid == "n") cout << "\nBan da chon khong thay doi so luong mac dinh" <<endl;
    else{
        do{
            cout << "\nNhap so luong ban muon thuc hien chuong trinh (luong toi da la "<< max_cores <<"): " << endl;
            cin >> numthread;
            if(numthread < 1 || numthread > max_cores) cout << "Loi chon so luong" <<endl;

        }while(numthread < 1 || numthread > max_cores);
    }


    //Xu ly sau nhap lieu
    passwordlength = passwordtext.length();
    for(int i = 1; i <= numpassword ; i++){
        maxindex += (long long)pow(passwordlength, i);
    }

    if(indexPassword.load() >= maxindex){
        cout << "Phat hien vi tri duyet truoc vuot qua khong gian mat khau " << numpassword <<" ky tu" << endl;
        cout << "Chuong trinh se duyet bung no to hop lai tu dau" <<endl;
        indexPassword.store(0);
    }
    cout << "Bat dau chuong trinh:" << endl;
}

void updateQueue(){
    string mid;
    indexPasswordQueue.store(0);

    while( indexPasswordQueue.load() < maxIndexQueue ){
        if(!getline(filePassword, mid)){
            checkEof.store(true);
            midMaxIndexQueue = indexPasswordQueue.load();
            break;
        }

        int currentIndex = indexPasswordQueue.fetch_add(1);
        if (currentIndex < maxIndexQueue) {
            passQueue[currentIndex] = mid;
        } else {
            indexPasswordQueue.fetch_sub(1);
            break;
        }
    }

    indexPasswordQueue.store(0);
}

void KiemTraDung() {
    while (!check.load() && countthread.load() != numthread) {
        this_thread::sleep_for(chrono::milliseconds(50));

            if (_kbhit()) {
            unsigned char ch = _getch();  // lay ki tu an
            if (ch == 'F' || ch == 'f') {
                exiting.store(true);  // Đặt cờ dừng
                this_thread::sleep_for(chrono::milliseconds(100));
                break;
            }
        }
    }
    cout << "\nDang dung..." << endl;
}

//Chuyen doi index sang password
string indexTransfer(string &passwordText, unsigned long long i) {
    unsigned static int size = passwordlength;
    int sizePassword = 1; //
    unsigned static long long midsize = (long long)pow(size,sizePassword);

    // Tính `sizePassword` phù hợp với `i`
    while (i >= midsize) {
        i -= midsize;
        sizePassword++;
        midsize =(long long)pow(passwordlength, sizePassword);
    }

    string line = "";
    while (i != 0) {
        unsigned long long index = i % size;
        line += passwordText[index];
        i /= size;
    }
    reverse(line.begin(), line.end());

    while (line.size() < sizePassword) {
        line = "0" + line;
    }
    return line;
}

//Thu mat khau bang bung no to hop
void TryPassWithBruteForce(string zipfile, unsigned long long maxindex, string passwordtext) {
    //gan nhan cpu
//    HANDLE thread = GetCurrentThread();
//    SetThreadAffinityMask(thread, affinity_mask);

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


    unsigned long long start_index= indexPassword.load();
    char buffer[4096];
    while (!check.load() && (start_index < maxindex) && !exiting.load()) {
        start_index = indexPassword.fetch_add(1);
        string password = indexTransfer(passwordtext, start_index);

        //cout << password << endl;

        zip_file_t* zf = zip_fopen_encrypted(archive, st.name, 0, password.c_str());
        if (zf) {
            // Đọc dữ liệu và kiểm tra CRC
            long long bytes_read = zip_fread(zf, buffer, sizeof(buffer));
            unsigned long crc = 0;

            while (bytes_read > 0) {
                crc = crc32(crc, (unsigned char*)buffer, bytes_read);
                bytes_read = zip_fread(zf, buffer, sizeof(buffer));
            }

            // So sánh CRC32 với giá trị CRC32 mong muốn
            if (st.crc == crc) {
                check.store(true);
                truePassword = password;
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

void TryPassWithDictionary(string zipfile) {
    //gan nhan cpu
//    HANDLE thread = GetCurrentThread();
//    SetThreadAffinityMask(thread, affinity_mask);

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

    int index;
    char buffer[4096]; // Đọc mỗi lần 4 KB
    while (!check.load()) {
        //kiem tra vi tri hang doi
        if(indexPasswordQueue.load() > midMaxIndexQueue){
            if(checkEof.load()) break; // xay ra khi doc het file

            while(checkQueue.load()){
                this_thread::sleep_for(chrono::milliseconds (10));
            }

            checkQueue.store(true);
            updateQueue();
            checkQueue.store(false);

        }
        password = passQueue[indexPasswordQueue.fetch_add(1)];


        zip_file_t* zf = zip_fopen_encrypted(archive, st.name, 0, password.c_str());
        if (zf) {
            // Đọc dữ liệu và kiểm tra CRC
            long long bytes_read = zip_fread(zf, buffer, sizeof(buffer));
            unsigned long crc = 0;

            while (bytes_read > 0) {
                crc = crc32(crc, (unsigned char*)buffer, bytes_read);
                bytes_read = zip_fread(zf, buffer, sizeof(buffer));
            }

            // So sánh CRC32 với giá trị CRC32 mong muốn
            if (st.crc == crc) {
                check.store(true);
                truePassword = password;
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
        cout << "\nDang thu voi tu dien" <<endl;
        auto startTudien = chrono::high_resolution_clock::now();

        for (int i = 0; i < numthread; i++) {
            threads.emplace_back(TryPassWithDictionary, zipfile);
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
        cout << "\nDang thu voi bung no to hop " << endl;
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

    if(check.load()){
        cout << "Mat khau tim duoc la: " << truePassword << endl;
    }
    else if (!check.load() && !exiting.load()) cout << "Khong tim thay mat khau" << endl;

    if (exiting.load()) {
        ofstream fout("LastPoint.txt");
        long long Point =indexPassword.load();
        string mid = indexTransfer(passwordtext ,Point);
        fout << zipfile << endl;
        fout << Point << endl;
        fout << mid << endl;
        fout.close();
        cout << "\nMat khau dung lai tai vi tri "<< Point << endl;
        cout << "Co ki tu la " << mid <<endl;
        cout << "Da luu thong tin last point vao thu muc LastPoint.txt"<<endl;
    }
    threads.clear();

}

int main(){

    input();

    start();

    system("pause");
    return 0;
}
