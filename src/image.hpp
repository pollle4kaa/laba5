#pragma once
#include <vector>
#include <cstdint>
#include <iostream>
#include <algorithm>

// Структура для представления цвета пикселя (RGB)
struct Color {
    uint8_t r; // Красный (0-255)
    uint8_t g; // Зеленый (0-255)
    uint8_t b; // Синий (0-255)

    // Конструктор по умолчанию (черный цвет)
    Color() : r(0), g(0), b(0) {}

    // Конструктор с параметрами
    Color(uint8_t red, uint8_t green, uint8_t blue) : r(red), g(green), b(blue) {}
};

// Тип для представления изображения
using Image = std::vector<std::vector<Color>>;

#include <cstdlib>
#include <ctime>

// Генерация случайного изображения заданного размера
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

#include <numeric>

// Функция для получения среднего цвета в окрестности 3x3
Color averageColor(const Image& image, int x, int y) {
    int sum_r = 0, sum_g = 0, sum_b = 0;
    int count = 0;

    // Проверяем соседние пиксели в окрестности 3x3
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            int nx = x + dx;
            int ny = y + dy;

            // Проверяем, что координаты внутри изображения
            if (nx >= 0 && ny >= 0 && nx < image[0].size() && ny < image.size()) {
                sum_r += image[ny][nx].r;
                sum_g += image[ny][nx].g;
                sum_b += image[ny][nx].b;
                count++;
            }
        }
    }

    // Вычисляем средние значения
    return Color(static_cast<uint8_t>(sum_r/count),static_cast<uint8_t>(sum_g/count),static_cast<uint8_t>(sum_b/count));
}

// Последовательное размытие изображения
Image sequentialBlur(const Image& input) {
    Image output = input; // Создаем копию для результата
    // Проходим по всем пикселям изображения
    for (size_t y = 0; y < input.size(); ++y) {
        for (size_t x = 0; x < input[y].size(); ++x) {
            output[y][x] = averageColor(input, x, y);
        }
    }
    return output;
}

#include <thread>
#include <mutex>

// Параллельное размытие с использованием std::thread
Image parallel(const Image& input, int num_threads = 4) {
    Image output = input;
    std::vector<std::thread> threads;
    int height = input.size();
    int strip_height = height/num_threads;

    // Функция, которую будет выполнять каждый поток
    auto blur_task = [&](int start_y, int end_y) {
        for (int y = start_y; y < end_y; ++y) {
            for (size_t x = 0; x < input[y].size(); ++x) {
                output[y][x] = averageColor(input, x, y);
            }
        }
        };

    // Создаем потоки
    for (int i = 0; i < num_threads; ++i) {
        int start_y = i * strip_height;
        int end_y = (i == num_threads - 1) ? height : (i + 1) * strip_height;

        threads.emplace_back(blur_task, start_y, end_y);
    }

    // Ожидаем завершения всех потоков
    for (auto& thread : threads) {
        thread.join();
    }

    return output;
}

#include <chrono>

// Функция для измерения времени выполнения
template<typename Func>
void measureTime(const std::string& name, Func func) {
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << name << " took " << duration.count() << " ms\n";
}

#include <atomic>

// Пример с атомарными операциями
void atomicExample() {
    const int num_iterations = 1000000;
    const int num_threads = 4;

    // Версия с мьютексом
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

    // Версия с atomic
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

#include <windows.h> // Для цветного вывода в Windows

void printColoredImage(const Image& image, int max_size = 10) {
    // Определяем размер для вывода (не больше max_size)
    int size = 10;

    for (int y = 0; y < size; ++y) {
        // Также ограничиваем ширину вывода
        int width = 10;

        for (int x = 0; x < width; ++x) {
            const Color& c = image[y][x];

            // Устанавливаем цвет в консоли
            HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
            WORD color = (c.r > 128 ? 4 : 0) | (c.g > 128 ? 2 : 0) | (c.b > 128 ? 1 : 0);
            SetConsoleTextAttribute(hConsole, color);
            SetConsoleOutputCP(CP_UTF8);
            std::cout << "@";
        }

        // Возвращаем стандартный цвет
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
        std::cout << "\n";
    }
}