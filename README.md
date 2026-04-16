# 并行算法与 GPU 编程实验仓库

本仓库收录了《并行算法与 GPU 编程》课程的两个核心实验：
- **实验一**：基于 **pthread** 的 CPU 多核并行图像卷积
- **实验二**：基于 **CUDA** 的 GPU 并行图像卷积

四种经典卷积算子（Sobel、Scharr、Laplacian、Gaussian）分别用 pthread 和 CUDA 实现，支持 PNG / JPG / BMP 等常见格式。

---

## 目录结构

```
pthreaddemo/
├── 实验一/                    # 实验一：CPU 多线程（pthread）
│   ├── Gaussian/
│   ├── Laplacian/
│   ├── Scharr/
│   └── Sobel/
├── 实验二/                    # 实验二：GPU 并行（CUDA）
│   ├── Gaussian/
│   ├── Laplacian/
│   ├── Scharr/
│   └── Sobel/
├── pthreaddemo/               # pthread 基础练习项目
├── 求pi/                      # pthread 计算圆周率练习
├── Sobel/                     # Sobel 边缘检测（VS 练习项目）
├── pthread范例/               # 课堂 pthread 范例代码
├── docs/                      # 实验报告、模板、归档压缩包（已 Git 忽略）
│   ├── reports/
│   ├── templates/
│   └── archives/
└── README.md
```

> **隐私说明**：个人实验报告及模板文件已放入 `docs/`，并在 `.gitignore` 中全局忽略，避免隐私信息泄露。

---

## 环境要求

| 实验 | 依赖 |
|------|------|
| **实验一** | Windows + MinGW-w64 / g++（支持 `-pthread`） |
| **实验二** | Windows + NVIDIA CUDA Toolkit 12.4 + Visual Studio 2022/2026 |
| 通用 | `stb_image.h` / `stb_image_write.h`（已随代码分发） |

> 注：本机 CUDA 12.4 与 VS 2026 存在官方版本兼容警告，编译时需加 `-allow-unsupported-compiler`。

---

## 实验一：CPU 多线程（pthread）

### 编译

进入对应算法目录，使用 g++ 编译（以 Sobel 为例）：

```powershell
cd "实验一\Sobel"
g++ -O3 -o Sobel.exe Sobel.cpp -pthread -std=c++11
```

### 运行

```powershell
.\Sobel.exe t53.png t53_sobel.png 4
```

参数说明：
1. `t53.png` — 输入图像
2. `t53_sobel.png` — 输出图像
3. `4` — 线程数

---

## 实验二：GPU 并行（CUDA）

### 编译

需要先在 **x64 Native Tools Command Prompt**（或已加载 `vcvarsall.bat` 的 PowerShell）中执行：

```powershell
cd "实验二\Sobel"
& cmd /c '"C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvarsall.bat" x64 && nvcc -allow-unsupported-compiler -O3 -o Sobel.exe Sobel.cu'
```

其余三个算法（Gaussian / Laplacian / Scharr）只需替换目录名和文件名即可。

### 运行

CUDA 版本简化为两个参数（去掉了线程数参数）：

```powershell
.\Sobel.exe t53.png t53_sobel_cuda.png
```

---

## 核心设计对比

| 特性 | 实验一（pthread） | 实验二（CUDA） |
|------|-------------------|----------------|
| **并行粒度** | 按图像行划分 | 按像素划分 |
| **线程组织** | `pthread_create` / `pthread_join` | `dim3 grid / block` + Kernel |
| **卷积核存储** | 栈上局部数组 | `__constant__` 常量显存 |
| **边界处理** | 零填充（Zero Padding） | 零填充（Zero Padding） |
| **后处理** | Gaussian 截断；其余 min-max 归一化 | 与实验一保持一致 |

---

## 验证结果

使用同一测试图 `t53.png`，实验二的 CUDA 输出与实验一的 pthread 输出**字节级完全一致**（`fc /b` 对比无差异），验证了 CUDA 实现的正确性。

