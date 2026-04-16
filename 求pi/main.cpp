#include <pthread.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

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

static void skipComments(std::istream& is) {
    while (true) {
        is >> std::ws;
        if (is.peek() == '#') {
            std::string line;
            std::getline(is, line);
        }
        else {
            break;
        }
    }
}

Image loadPGM(const std::string& filename) {
    std::ifstream fin(filename, std::ios::binary);
    if (!fin) {
        throw std::runtime_error("无法打开输入文件: " + filename);
    }

    std::string magic;
    fin >> magic;
    if (magic != "P5" && magic != "P2") {
        throw std::runtime_error("仅支持 P5/P2 格式的灰度 PGM 图像");
    }

    skipComments(fin);
    int width = 0, height = 0, maxval = 0;
    fin >> width;
    skipComments(fin);
    fin >> height;
    skipComments(fin);
    fin >> maxval;
    fin.get();

    if (width <= 0 || height <= 0 || maxval <= 0 || maxval > 255) {
        throw std::runtime_error("PGM 文件头非法");
    }

    Image img;
    img.width = width;
    img.height = height;
    img.data.resize(static_cast<size_t>(width) * height);

    if (magic == "P5") {
        fin.read(reinterpret_cast<char*>(img.data.data()), static_cast<std::streamsize>(img.data.size()));
        if (!fin) {
            throw std::runtime_error("读取 P5 图像数据失败");
        }
    }
    else {
        for (size_t i = 0; i < img.data.size(); ++i) {
            int v;
            fin >> v;
            if (!fin) {
                throw std::runtime_error("读取 P2 图像数据失败");
            }
            img.data[i] = static_cast<unsigned char>(std::clamp(v, 0, 255));
        }
    }

    return img;
}

void savePGM(const std::string& filename, const Image& img) {
    std::ofstream fout(filename, std::ios::binary);
    if (!fout) {
        throw std::runtime_error("无法打开输出文件: " + filename);
    }
    fout << "P5\n" << img.width << ' ' << img.height << "\n255\n";
    fout.write(reinterpret_cast<const char*>(img.data.data()), static_cast<std::streamsize>(img.data.size()));
}

static inline float getPixel(const Image& img, int r, int c) {
    if (r < 0 || r >= img.height || c < 0 || c >= img.width) {
        return 0.0f;
    }
    return static_cast<float>(img.data[static_cast<size_t>(r) * img.width + c]);
}

void* sobelWorker(void* arg) {
    auto* t = static_cast<ThreadArgs*>(arg);
    const Image& input = *t->input;
    std::vector<float>& output = *t->output;

    const int sobelX[3][3] = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1}
    };
    const int sobelY[3][3] = {
        {-1, -2, -1},
        { 0,  0,  0},
        { 1,  2,  1}
    };

    for (int i = t->start_row; i < t->end_row; ++i) {
        for (int j = 0; j < input.width; ++j) {
            float gx = 0.0f;
            float gy = 0.0f;
            for (int ki = -1; ki <= 1; ++ki) {
                for (int kj = -1; kj <= 1; ++kj) {
                    float pix = getPixel(input, i + ki, j + kj);
                    gx += pix * sobelX[ki + 1][kj + 1];
                    gy += pix * sobelY[ki + 1][kj + 1];
                }
            }
            output[static_cast<size_t>(i) * input.width + j] = std::sqrt(gx * gx + gy * gy);
        }
    }
    return nullptr;
}

Image normalizeToImage(const std::vector<float>& buffer, int width, int height) {
    Image out;
    out.width = width;
    out.height = height;
    out.data.resize(static_cast<size_t>(width) * height);

    auto [min_it, max_it] = std::minmax_element(buffer.begin(), buffer.end());
    float minv = *min_it;
    float maxv = *max_it;

    if (std::fabs(maxv - minv) < 1e-6f) {
        std::fill(out.data.begin(), out.data.end(), 0);
        return out;
    }

    for (size_t i = 0; i < buffer.size(); ++i) {
        float v = (buffer[i] - minv) * 255.0f / (maxv - minv);
        int iv = static_cast<int>(std::lround(v));
        out.data[i] = static_cast<unsigned char>(std::clamp(iv, 0, 255));
    }
    return out;
}

Image applySobelParallel(const Image& input, int num_threads) {
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

        if (pthread_create(&threads[t], nullptr, sobelWorker, &args[t]) != 0) {
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
        << "  " << prog << " <input.pgm> <output.pgm> <num_threads>\n\n"
        << "示例:\n"
        << "  " << prog << " lena.pgm out_sobel.pgm 4\n";
}

int main(int argc, char* argv[]) {
    try {
        if (argc != 4) {
            printUsage(argv[0]);
            return 1;
        }

        std::string input_file = argv[1];
        std::string output_file = argv[2];
        int num_threads = std::stoi(argv[3]);

        Image input = loadPGM(input_file);

        auto t1 = std::chrono::high_resolution_clock::now();
        Image output = applySobelParallel(input, num_threads);
        auto t2 = std::chrono::high_resolution_clock::now();

        savePGM(output_file, output);

        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        std::cout << "Sobel 并行卷积完成\n";
        std::cout << "线程数: " << num_threads << "\n";
        std::cout << "耗时: " << ms << " ms\n";
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << '\n';
        return 1;
    }
}