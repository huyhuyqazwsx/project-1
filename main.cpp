#include <iostream>
#include <minizip/unzip.h>
#include <zlib.h>
#include <atomic>
#include <fstream>
#include <chrono>
using namespace std;

atomic<bool> check(false);

void trypass(string &zipfile, string &password) {
    unzFile file=unzOpen(zipfile.c_str())    ;

    if (file == NULL) {
        cout<<"Khong mo duoc file zip"<<endl;
        return;
    }

    if (unzGoToFirstFile(file) != UNZ_OK) {
        cout<<"File rong hoac loi file";
        return;
    }

    if (unzOpenCurrentFilePassword(file, password.c_str())==UNZ_OK) {
        char dem[1024];
        int bytetoread= unzReadCurrentFile(file, dem, sizeof(dem));
        long long crc=0;
        while (bytetoread > 0) {
            crc= crc32(crc, (unsigned char *)dem, bytetoread);
            bytetoread= unzReadCurrentFile(file, dem, sizeof(dem));
        }

        unz_file_info file_info;
        unzGetCurrentFileInfo(file, &file_info,NULL,0,NULL,0,NULL,0);
        if (file_info.crc==crc) {
            cout<<"Mat khau tim duoc: "<<password<<endl;
            check.store(true);
            unzCloseCurrentFile(file);
            unzClose(file);
            return;
        }
    }

    unzClose(file);

}

void getpassword(string &zipfile,string &passwordfile) {
    ifstream file(passwordfile.c_str());
    string password;

    while (getline(file,password)) {
        if (check.load()) break;
        // cout<<password<<endl;
        trypass(zipfile,password);
    }

}

int main() {
    string zipfile = "D:/testzip/huy.zip"; // duong dan
    string passwordfile = "D:/testzip/bungnotohop.txt";// dung dan file mat khau


    auto start = chrono:: high_resolution_clock::now();

    getpassword(zipfile,passwordfile);

    auto end = chrono::high_resolution_clock::now();

    chrono::duration<double> diff = end - start;
    cout << diff.count() << endl;

}