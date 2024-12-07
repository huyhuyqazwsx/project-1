#include <iostream>
#include <minizip/unzip.h>
#include <string>
#include <zlib.h>
#include <atomic>


std::atomic<bool> passwordFound(false);

bool trypass(std::string &filezip, std::string &password) {
    //unzOpen su dung bien *char nen phai su dung c_str()
    unzFile file =unzOpen(filezip.c_str());

    //Neu khong mo duoc file zip
    if (file == NULL) {
        std::cout<<"Loi khong mo duoc file";
        return false;
    }
    //neu zip rong hoac loi khi mo
    if (unzGoToFirstFile(file)!=UNZ_OK) {
        std::cout<<"khong mo duoc file";
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
                std::cout<<"Tim thay mat khau :"<<password<<std::endl;
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