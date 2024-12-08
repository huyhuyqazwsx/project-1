#include <iostream>
#include <minizip/unzip.h>
#include <string>
#include <zlib.h>
#include <atomic>
#include <fstream>
#include <chrono>
using namespace std;


atomic<bool> passwordFound(false);

bool trypass(string &filezip, string &password) {
    //unzOpen su dung bien *char nen phai su dung c_str()
    unzFile file =unzOpen(filezip.c_str());

    //Neu khong mo duoc file zip
    if (file == NULL) {
        cout<<"Loi khong mo duoc file";
        return false;
    }
    //neu zip rong hoac loi khi mo
    if (unzGoToFirstFile(file)!=UNZ_OK) {
        cout<<"khong mo duoc file";
        unzClose(file);
        return false;
    }
    //thu mo file voi mat khau
    if (unzOpenCurrentFilePassword(file , password.c_str())==UNZ_OK) {
        char dem[1024];//moi lan doc toi da 1024 byte
        int bytesread=0;//so luong dang co
        long long crc=0;

        //ham kiem tra crc
        //unzReadCurrentFile==0 tuc la da doc het file
        while ((bytesread=unzReadCurrentFile(file, dem, sizeof(dem)))>0) {
            crc=crc32(crc, (unsigned char*) dem , bytesread);
        }

        unz_file_info fileinfo;//ghi thuoc tinh file

        if (unzGetCurrentFileInfo(file, &fileinfo,nullptr,0,nullptr,0,nullptr,0) == UNZ_OK) {
            if (fileinfo.crc==crc) {
                cout<<"Tim thay mat khau :"<<password<<endl;
                passwordFound.store(true);//da tim thay
                unzCloseCurrentFile(file);
                unzClose(file);
                return true;
            }
        }
    }
    unzClose(file);
    return false;
}

void matkhautudanhsach(string &filezip, string &passwordfile) {
    ifstream file(passwordfile);

    if (file.is_open()) {
        string line;
        while (getline(file, line)) {
            if (passwordFound.load()) {
                break;
            }
            cout <<line<<endl;
            trypass(filezip, line);
        }
        file.close();
    }
}

int main() {
    string filezip="D:/testzip/huy.zip"; //duong dan file zip
    string passwordflie="D:/testzip/bungnotohop.txt";

    auto start = chrono::high_resolution_clock::now();

    matkhautudanhsach(filezip, passwordflie);


    //dem thoi gian
     auto end = chrono::high_resolution_clock::now();

     chrono::duration<double> duration = end - start;

     std::cout << duration.count() << std::endl;
}