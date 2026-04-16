#include <cuda_runtime.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

struct Image {
    int width = 0;
    int height = 0;
    std::vector<unsigned char> data;
};

__constant__ int d_laplacian[3][3] = {
    {0,  1, 0},
    {1, -4, 1},
    {0,  1, 0}
};

Image loadImage(const std::string& filename) {
    int width = 0, height = 0, channels = 0;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &channels, 0);
    if (!data) {
        throw std::runtime_error("无法加载图像: " + filename + " - " + stbi_failure_reason());
    }

    Image img;
    img.width = width;
    img.height = height;
    img.data.resize(static_cast<size_t>(width) * height);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int idx = (y * width + x) * channels;
            if (channels >= 3) {
                float gray = 0.299f * data[idx] + 0.587f * data[idx + 1] + 0.114f * data[idx + 2];
                img.data[y * width + x] = static_cast<unsigned char>(gray);
            } else {
                img.data[y * width + x] = data[idx];
            }
        }
    }

    stbi_image_free(data);
    return img;
}

void saveImage(const std::string& filename, const Image& img) {
    std::string ext;
    size_t dot_pos = filename.rfind('.');
    if (dot_pos != std::string::npos) {
        ext = filename.substr(dot_pos);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    }

    int success = 0;
    if (ext == ".png") {
        success = stbi_write_png(filename.c_str(), img.width, img.height, 1, img.data.data(), img.width);
    } else if (ext == ".jpg" || ext == ".jpeg") {
        success = stbi_write_jpg(filename.c_str(), img.width, img.height, 1, img.data.data(), 95);
    } else if (ext == ".bmp") {
        success = stbi_write_bmp(filename.c_str(), img.width, img.height, 1, img.data.data());
    } else {
        success = stbi_write_png(filename.c_str(), img.width, img.height, 1, img.data.data(), img.width);
    }

    if (!success) {
        throw std::runtime_error("保存图像失败: " + filename);
    }
}

__global__ void laplacianKernel(const unsigned char* input, float* output, int width, int height) {
    int col = blockIdx.x * blockDim.x + threadIdx.x;
    int row = blockIdx.y * blockDim.y + threadIdx.y;

    if (row >= height || col >= width) return;

    float sum = 0.0f;
    for (int ki = -1; ki <= 1; ++ki) {
        for (int kj = -1; kj <= 1; ++kj) {
            int r = row + ki;
            int c = col + kj;
            float pix = 0.0f;
            if (r >= 0 && r < height && c >= 0 && c < width) {
                pix = static_cast<float>(input[r * width + c]);
            }
            sum += pix * static_cast<float>(d_laplacian[ki + 1][kj + 1]);
        }
    }

    output[row * width + col] = fabsf(sum);
}

Image normalizeToImage(const std::vector<float>& buffer, int width, int height) {
    Image out;
    out.width = width;
    out.height = height;
    out.data.resize(static_cast<size_t>(width) * height);

    auto minmax = std::minmax_element(buffer.begin(), buffer.end());
    float minv = *minmax.first;
    float maxv = *minmax.second;

    if (std::fabs(maxv - minv) < 1e-6f) {
        std::fill(out.data.begin(), out.data.end(), 0);
        return out;
    }

    for (size_t i = 0; i < buffer.size(); ++i) {
        float v = (buffer[i] - minv) * 255.0f / (maxv - minv);
        int iv = static_cast<int>(std::lround(v));
        out.data[i] = static_cast<unsigned char>(iv < 0 ? 0 : (iv > 255 ? 255 : iv));
    }
    return out;
}

Image applyLaplacianCUDA(const Image& input) {
    size_t size = static_cast<size_t>(input.width) * input.height;
    unsigned char* d_input = nullptr;
    float* d_output = nullptr;

    cudaMalloc(&d_input, size);
    cudaMalloc(&d_output, size * sizeof(float));
    cudaMemcpy(d_input, input.data.data(), size, cudaMemcpyHostToDevice);

    dim3 block(16, 16);
    dim3 grid((input.width + block.x - 1) / block.x, (input.height + block.y - 1) / block.y);

    laplacianKernel<<<grid, block>>>(d_input, d_output, input.width, input.height);
    cudaDeviceSynchronize();

    std::vector<float> host_output(size);
    cudaMemcpy(host_output.data(), d_output, size * sizeof(float), cudaMemcpyDeviceToHost);

    cudaFree(d_input);
    cudaFree(d_output);

    return normalizeToImage(host_output, input.width, input.height);
}

void printUsage(const char* prog) {
    std::cout << "用法:\n"
              << "  " << prog << " <input> <output>\n\n"
              << "支持格式: PNG, JPG, JPEG, BMP, PGM\n\n"
              << "示例:\n"
              << "  " << prog << " lena.png out_laplacian.png\n"
              << "  " << prog << " photo.jpg result.jpg\n";
}

int main(int argc, char* argv[]) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    try {
        if (argc != 3) {
            printUsage(argv[0]);
            return 1;
        }

        std::string input_file = argv[1];
        std::string output_file = argv[2];

        Image input = loadImage(input_file);

        auto t1 = std::chrono::high_resolution_clock::now();
        Image output = applyLaplacianCUDA(input);
        auto t2 = std::chrono::high_resolution_clock::now();

        saveImage(output_file, output);

        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        std::cout << "Laplacian CUDA 卷积完成\n";
        std::cout << "耗时: " << ms << " ms\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << '\n';
        return 1;
    }
}
