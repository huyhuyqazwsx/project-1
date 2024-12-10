#include <bits/stdc++.h>
#include <minizip/unzip.h>
#include <zlib.h>
#include <atomic>
#include <chrono>
using namespace std;

atomic<bool> check(false);
string passwordtext;

string indextranfer(string passwordtext, long long i){
    long long size= passwordtext.size();
    string mid="";
    while(i!=0){
        long long index= i%size;
        mid+=passwordtext[index];
        i=i/size;
    }
    reverse(mid.begin(), mid.end());
    return mid;
}

void trypass(string &zipfile, long long index, int const numthread, long long const maxindex ) {

    unzFile file=unzOpen(zipfile.c_str());

    if (file == NULL) {
        cout<<"Khong mo duoc file zip"<<endl;
        return;
    }

    if (unzGoToFirstFile(file) != UNZ_OK) {
        cout<<"File rong hoac loi file";
        return;
    }

    //kiem tra mat khau
    while (!check.load() && index<=maxindex ) {
        string password= indextranfer(passwordtext, index);
        index+=numthread;

        if (unzOpenCurrentFilePassword(file, password.c_str())==UNZ_OK) {
            // cout<<password<<endl;
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
            unzCloseCurrentFile(file);
        }
    }
    unzClose(file);
}


int main() {
    string zipfile = "D:/testzip/huytestZZZZ.zip"; // duong dan
    passwordtext = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

    int numpassword=4;

    //nhap so luong
    int numthread;
    cout<<"Nhap so luong muon chay:";
    cin>>numthread;

    int passwordlength= passwordtext.length();
    long long maxindex= pow(passwordlength,numpassword);

    cout<<"Bat dau chuong trinh:"<<endl;
    auto start = chrono:: high_resolution_clock::now();

    //chuogn trinh chinh
    vector<thread> threads;

    for (int i=0;i<numthread;i++) {
        threads.emplace_back(trypass,ref(zipfile),i,numthread,maxindex);
    }

    for (auto &th : threads) {
        th.join();
    }

    auto end = chrono::high_resolution_clock::now();

    chrono::duration<double> diff = end - start;

    if (check.load()==false) cout<<"Khong tim thay mat khau"<<endl;

    cout << "Thoi gian giai ma: " << diff.count() << " s" << endl;

}