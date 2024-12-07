#include <iostream>
#include <minizip/unzip.h>
#include <string>
#include <zlib.h>
#include <atomic>


std::atomic<bool> passwordFound(false);

bool trypass(std::string &filezip, std::string &password) {
    unzFile file =unzOpen(filezip.c_str());
    if (file == NULL) {
        std::cout<<"Loi khong mo duoc file";
        return false;
    }

    if (unzGoToFirstFile(file)!=UNZ_OK) {
        std::cout<<"khong mo duoc file";
        unzClose(file);
        return false;
    }

    if (unzOpenCurrentFilePassword(file , password.c_str())==UNZ_OK) {
        char dem[1024];
        int bytesread=0;
        long long crc=0;
        while ((bytesread=unzReadCurrentFile(file, dem, sizeof(dem)))>0) {
            crc=crc32(crc, (unsigned char*) dem , bytesread);
        }

        unz_file_info fileinfo;

        if (unzGetCurrentFileInfo(file, &fileinfo,nullptr,0,nullptr,0,nullptr,0) == UNZ_OK) {
            if (fileinfo.crc==crc) {
                std::cout<<"Tim thay mat khau :"<<password<<std::endl;
                passwordFound.store(true);
                unzCloseCurrentFile(file);
                unzClose(file);
                return true;
            }
        }
    }
    unzClose(file);
    return false;
}

void bungnotohop(int len,std::string &filezip, std::string password, std::string midpassword) {
    if (len==0 || passwordFound.load()) {
        if (!passwordFound.load()) {
            std::cout<<midpassword<<std::endl;
            trypass(filezip, midpassword);
        }
        return;
    }

    for (int i=0;i<password.size();i++) {
        if (passwordFound.load()) {
            return;
        }

        bungnotohop(len-1,filezip,password,midpassword+password[i]);
    }
}

int main() {
    std::string filezip="D:/testzip/huy.zip"; //duong dan file zip
    std::string password="0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int maxlen=4;
    for (int i=0;i<=maxlen;i++) {
        bungnotohop(i,filezip,password,"");
    }

}