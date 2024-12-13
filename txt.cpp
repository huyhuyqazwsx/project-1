#include <bits/stdc++.h>
#include <minizip/unzip.h>
#include <zlib.h>
#include <atomic>
#include <thread>
#include <fstream>
#include <filesystem>

using namespace std;

atomic<bool> check(false);

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

// Hàm thử mật khẩu từ từ điển
void tryPasswordWithDictionary(const string& zipfile, const vector<string>& passwords, unz_file_info file_info) {
    unzFile file = unzOpen(zipfile.c_str());
    if (!file) {
        cerr << "Failed to open file: " << zipfile << endl;
        return;
    }

    if (unzGoToFirstFile(file) != UNZ_OK) {
        cerr << "ZIP file is empty or corrupted." << endl;
        unzClose(file);
        return;
    }

    for (const string& password : passwords) {
        if (check.load()) break; // Dừng nếu mật khẩu đúng đã được tìm thấy

        if (unzOpenCurrentFilePassword(file, password.c_str()) == UNZ_OK) {
            // Đọc dữ liệu và kiểm tra CRC
            char buffer[1024];
            uLong crc = crc32(0L, Z_NULL, 0);
            int bytesRead;
            while ((bytesRead = unzReadCurrentFile(file, buffer, sizeof(buffer))) > 0) {
                crc = crc32(crc, reinterpret_cast<const Bytef*>(buffer), bytesRead);
            }

            if (crc == file_info.crc) {
                check.store(true);
                cout << "Password found: " << password << endl;
                unzCloseCurrentFile(file);
                unzClose(file);
                return;
            }
            unzCloseCurrentFile(file);
        }
    }

    unzClose(file);
}

void workerThread(const string& zipfile, const string& dictionaryFile, int threadID, int numThreads) {
    ifstream infile(dictionaryFile);
    if (!infile.is_open()) {
        cerr << "Failed to open dictionary file: " << dictionaryFile << endl;
        return;
    }

    vector<string> passwords;
    string line;
    int lineIndex = 0;

    // Lấy thông tin file để kiểm tra CRC
    unzFile file = unzOpen(zipfile.c_str());
    unz_file_info file_info;
    if (unzGoToFirstFile(file) != UNZ_OK || unzGetCurrentFileInfo(file, &file_info, NULL, 0, NULL, 0, NULL, 0) != UNZ_OK) {
        cerr << "Failed to retrieve file info from ZIP file." << endl;
        unzClose(file);
        return;
    }
    unzClose(file);

    // Đọc từ file từ điển và chỉ xử lý các dòng thuộc về luồng hiện tại
    while (getline(infile, line)) {
        if (lineIndex % numThreads == threadID) {
            passwords.push_back(line);
        }
        lineIndex++;
    }
    infile.close();

    // Thử mật khẩu trong danh sách
    tryPasswordWithDictionary(zipfile, passwords, file_info);
}

int main() {
    string zipfile, dictionaryFile;
    int numThreads;

    cout << "Enter ZIP file path: ";
    cin >> zipfile;
    cout << "Enter dictionary file path: ";
    cin >> dictionaryFile;
    cout << "Enter number of threads: ";
    cin >> numThreads;

    vector<string> copyFiles(numThreads);

    // Tạo các bản sao của file ZIP
    for (int i = 0; i < numThreads; ++i) {
        copyFiles[i] = "copy_" + to_string(i) + ".zip";
        copyFile(zipfile, copyFiles[i]);
    }

    auto start = chrono::high_resolution_clock::now();

    vector<thread> threads;
    for (int i = 0; i < numThreads; i++) {
        threads.emplace_back(workerThread, copyFiles[i], dictionaryFile, i, numThreads);
    }

    for (auto& th : threads) {
        th.join();
    }

    // Xóa các bản sao của file ZIP
    for (const auto& file : copyFiles) {
        deleteFile(file);
    }

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = end - start;

    if (!check.load()) {
        cout << "Password not found in dictionary." << endl;
    }
    cout << "Elapsed time: " << elapsed.count() << " seconds" << endl;

    return 0;
}
