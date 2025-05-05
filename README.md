# 5 ������������ ������ "���������������"

## ��������� �������
- image.hpp - ������������ ����
- main.cpp - �������� ����
- CMakeLists.txt - ���� ��� ������ �������

# ��������� Color (���� ������ ����������� �������)
```
struct Color {
    uint8_t r; // ������� (0-255)
    uint8_t g; // ������� (0-255)
    uint8_t b; // ����� (0-255)

    // ����������� �� ��������� (������ ����)
    Color() : r(0), g(0), b(0) {}

    // ����������� � �����������
    Color(uint8_t red, uint8_t green, uint8_t blue) : r(red), g(green), b(blue) {}
};
```
- ������ ������� ����������� ����� ��� �������� ������: ������� (R), ������� (G) � ����� (B)
- uint8_t - ��� ������ ��� ����� ����� �� 0 �� 255 (1 ����)
- ������������ ��������� ��������� ����:
1. �� ��������� - ������ (0,0,0)
2. � ���������� �������� RGB

#��� Image
```
using Image = std::vector<std::vector<Color>>;
```
- ����������� �������������� � ���� ���������� ������� ������
- ������ (����.) ������ - ������ �����������
- ������ (�����.) - ������� � �������

# ��������� ���������� �����������
```
Image RandomImage(size_t width, size_t height) {
    Image image(height, std::vector<Color>(width));
    std::srand(std::time(0));
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            image[y][x] = Color(std::rand() % 256, std::rand() % 256, std::rand() % 256);
        }
    }
    return image;
}
```
- ��������� ����������� ���������� �������
- std::srand(std::time(0)); - ��������� ��������������� ����� (������� �����)
- ������� ���� ��������� ������ ������� ��������� ������
- ������� ���������� ������� �����������

# �������� ������ ����������� �������
```
Color averageColor(const Image& image, int x, int y) {
    int sum_r = 0, sum_g = 0, sum_b = 0;
    int count = 0;

    // ��������� �������� ������� � ����������� 3x3
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            int nx = x + dx;
            int ny = y + dy;

            // ���������, ��� ���������� ������ �����������
            if (nx >= 0 && ny >= 0 && nx < image[0].size() && ny < image.size()) {
                sum_r += image[ny][nx].r;
                sum_g += image[ny][nx].g;
                sum_b += image[ny][nx].b;
                count++;
            }
        }
    }

    // ��������� ������� ��������
    return Color(static_cast<uint8_t>(sum_r/count),static_cast<uint8_t>(sum_g/count),static_cast<uint8_t>(sum_b/count));
}
```
- ������� ��������� ����������� � ���������� ������� (x,y)
- ���������� �������� ������ �������� �����
- ������� ���� ��������� 8 �������� �������� (3x3) + � ��� �������
- ���� �������� �� ������� �����������, ����� �� ����� �� �������
- ����������� ������� �������� ��� ������� ������ � ������������ ����� ���� - ��������� ��������

# ���������������� ��������
```
Image sequentialBlur(const Image& input) {
    Image output = input; // ������� ����� ��� ����������
    // �������� �� ���� �������� �����������
    for (size_t y = 0; y < input.size(); ++y) {
        for (size_t x = 0; x < input[y].size(); ++x) {
            output[y][x] = averageColor(input, x, y);
        }
    }
    return output;
}
```
- �������� ����� ����������� ��� ���-�
- � ������� �������� ����� ���������� �� ������� �������, �������� ��� ���� ������� ��������
- ��������� �������� ���������� � ����� � ���������� �������� �����������

# ������������ �������� � �������������� �������
```
Image parallel(const Image& input, int num_threads = 4) {
    Image output = input;
    std::vector<std::thread> threads;
    int height = input.size();
    int strip_height = height/num_threads;

    // �������, ������� ����� ��������� ������ �����
    auto blur_task = [&](int start_y, int end_y) {
        for (int y = start_y; y < end_y; ++y) {
            for (size_t x = 0; x < input[y].size(); ++x) {
                output[y][x] = averageColor(input, x, y);
            }
        }
        };

    // ������� ������
    for (int i = 0; i < num_threads; ++i) {
        int start_y = i * strip_height;
        int end_y = (i == num_threads - 1) ? height : (i + 1) * strip_height;

        threads.emplace_back(blur_task, start_y, end_y);
    }

    // ������� ���������� ���� �������
    for (auto& thread : threads) {
        thread.join();
    }

    return output;
}
```
- �������� ����� ����������� ��� �����������
- �������� ������� ��� �������� �������
- ���������� ������ ������ ��� ������� ������
- ������-������� blur_task, ������� ������������ ���� ����� �����������
 - �������� �������:
1. ������ ����� �������� ���� �������� ����� (start_y, end_y)
2. ��������� ����� ������������ ���������� ������
3. �������� ���������� ���� ������� (thread.join())

# ��������� ������� ����������
```
template<typename Func>
void measureTime(const std::string& name, Func func) {
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << name << " took " << duration.count() << " ms\n";
}
```

# ������ � ���������� ����������
```
void atomicExample() {
    const int num_iterations = 1000000;
    const int num_threads = 4;

    // ������ � ���������
    {
        int counter = 0;
        std::mutex counter_mutex;
        std::vector<std::thread> threads;

        auto task = [&]() {
            for (int i = 0; i < num_iterations; ++i) {
                std::lock_guard<std::mutex> lock(counter_mutex);
                counter++;
            }
        };

        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back(task);
        }
        for (auto& thread : threads) {
            thread.join();
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        std::cout << "Mutex counter: " << counter
            << ", time: " << duration.count() << " ms\n";
    }

    // ������ � atomic
    {
        std::atomic<int> counter(0);
        std::vector<std::thread> threads;

        auto task = [&]() {
            for (int i = 0; i < num_iterations; ++i) {
                counter++;
            }
            };

        auto start = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back(task);
        }

        for (auto& thread : threads) {
            thread.join();
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        std::cout << "Atomic counter: " << counter
            << ", time: " << duration.count() << " ms\n";
    }
}
```
- ��������� ������� � ������� ��� ��� ������
- ����������� ������, ������ ����������� �������
- ���������� ����� ���������� � �� �� ����� ����������� � atomic-��������� 

## ������ �� ������� 
1. ���������� ��������� ������������������
Sequential blur took 308 ms
Parallel blur (4 threads) took 124 ms
Atomic operations example:
 Mutex counter: 4000000, time: 139 ms
 Atomic counter: 4000000, time: 57 ms

����� ���������� �������� �������������� ���������� �� ��������� ���� ����������
��������� ����� ����� ����������
������������ ������� ������� �����������

�������� ������������� � �� �������
�������� ��������:
����� ��������� ������� ���������� � ����� ������
����������������
�������� � �������������������

������� �������:
�������� (std::mutex) - ��������� ������ � ����� ������
��������� �������� (std::atomic) - ��� ������� �������� (�������)
���������� ������ - ����� ������ �������� � ������� ��������� ������
