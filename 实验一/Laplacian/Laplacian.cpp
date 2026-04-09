#include <pthread.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>
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

struct ThreadArgs {
    const Image* input = nullptr;
    std::vector<float>* output = nullptr;
    int start_row = 0;
    int end_row = 0;
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

    // 转换为灰度图
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int idx = (y * width + x) * channels;
            if (channels >= 3) {
                // RGB/RGBA - 使用标准灰度转换
                float gray = 0.299f * data[idx] + 0.587f * data[idx + 1] + 0.114f * data[idx + 2];
                img.data[y * width + x] = static_cast<unsigned char>(gray);
            } else {
                // 灰度图
                img.data[y * width + x] = data[idx];
            }
        }
    }

    stbi_image_free(data);
    return img;
}

void saveImage(const std::string& filename, const Image& img) {
    // 根据扩展名判断格式
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
        // 默认保存为 PNG
        success = stbi_write_png(filename.c_str(), img.width, img.height, 1, img.data.data(), img.width);
    }

    if (!success) {
        throw std::runtime_error("保存图像失败: " + filename);
    }
}

static inline float getPixel(const Image& img, int r, int c) {
    if (r < 0 || r >= img.height || c < 0 || c >= img.width) {
        return 0.0f;
    }
    return static_cast<float>(img.data[static_cast<size_t>(r) * img.width + c]);
}

void* laplacianWorker(void* arg) {
    auto* t = static_cast<ThreadArgs*>(arg);
    const Image& input = *t->input;
    std::vector<float>& output = *t->output;

    const int laplacian[3][3] = {
        {0,  1, 0},
        {1, -4, 1},
        {0,  1, 0}
    };

    for (int i = t->start_row; i < t->end_row; ++i) {
        for (int j = 0; j < input.width; ++j) {
            float sum = 0.0f;
            for (int ki = -1; ki <= 1; ++ki) {
                for (int kj = -1; kj <= 1; ++kj) {
                    float pix = getPixel(input, i + ki, j + kj);
                    sum += pix * laplacian[ki + 1][kj + 1];
                }
            }
            output[static_cast<size_t>(i) * input.width + j] = std::fabs(sum);
        }
    }
    return nullptr;
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

Image applyLaplacianParallel(const Image& input, int num_threads) {
    if (num_threads <= 0) {
        throw std::runtime_error("线程数必须大于 0");
    }

    std::vector<float> output(static_cast<size_t>(input.width) * input.height, 0.0f);
    std::vector<pthread_t> threads(static_cast<size_t>(num_threads));
    std::vector<ThreadArgs> args(static_cast<size_t>(num_threads));

    int rows_per_thread = input.height / num_threads;
    int rem = input.height % num_threads;

    for (int t = 0; t < num_threads; ++t) {
        int extra = (t < rem) ? 1 : 0;
        int start = t * rows_per_thread + std::min(t, rem);
        int end = start + rows_per_thread + extra;

        args[t].input = &input;
        args[t].output = &output;
        args[t].start_row = start;
        args[t].end_row = end;

        if (pthread_create(&threads[t], nullptr, laplacianWorker, &args[t]) != 0) {
            throw std::runtime_error("创建线程失败");
        }
    }

    for (int t = 0; t < num_threads; ++t) {
        pthread_join(threads[t], nullptr);
    }

    return normalizeToImage(output, input.width, input.height);
}

void printUsage(const char* prog) {
    std::cout << "用法:\n"
              << "  " << prog << " <input> <output> <num_threads>\n\n"
              << "支持格式: PNG, JPG, JPEG, BMP, PGM\n\n"
              << "示例:\n"
              << "  " << prog << " lena.png out_laplacian.png 4\n"
              << "  " << prog << " photo.jpg result.jpg 8\n";
}

int main(int argc, char* argv[]) {
#ifdef _WIN32
    // 设置控制台输出为 UTF-8 编码，解决中文乱码问题
    SetConsoleOutputCP(CP_UTF8);
#endif
    try {
        if (argc != 4) {
            printUsage(argv[0]);
            return 1;
        }

        std::string input_file = argv[1];
        std::string output_file = argv[2];
        int num_threads = std::stoi(argv[3]);

        Image input = loadImage(input_file);

        auto t1 = std::chrono::high_resolution_clock::now();
        Image output = applyLaplacianParallel(input, num_threads);
        auto t2 = std::chrono::high_resolution_clock::now();

        saveImage(output_file, output);

        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        std::cout << "Laplacian 并行卷积完成\n";
        std::cout << "线程数: " << num_threads << "\n";
        std::cout << "耗时: " << ms << " ms\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << '\n';
        return 1;
    }
}
