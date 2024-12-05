#include <iostream>
#include <vector>
#include <zlib.h>
#include <cstring>

int main() {
    const char* input = "Hello, CLion with zlib!";
    size_t inputLength = strlen(input) + 1;  // Include null terminator

    // Calculate maximum compressed size
    uLongf compressedLength = compressBound(inputLength);
    std::vector<Bytef> compressedData(compressedLength);

    int ret = compress(compressedData.data(), &compressedLength,
                       reinterpret_cast<const Bytef*>(input), inputLength);
    if (ret == Z_OK) {
        std::cout << "Compression successful! Compressed size: " << compressedLength << std::endl;
    } else {
        std::cerr << "Compression failed with error code: " << ret << std::endl;
    }

    return 0;
}
