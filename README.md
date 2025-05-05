# 5 Лабораторная работа "Многопоточность"

## Структура проекта
- image.hpp - Заголовочный файл
- main.cpp - Основной файл
- CMakeLists.txt - Файл для сборки проекта

# Структура Color (Цвет одного конкретного пикселя)
```
struct Color {
    uint8_t r; // Красный (0-255)
    uint8_t g; // Зеленый (0-255)
    uint8_t b; // Синий (0-255)

    // Конструктор по умолчанию (черный цвет)
    Color() : r(0), g(0), b(0) {}

    // Конструктор с параметрами
    Color(uint8_t red, uint8_t green, uint8_t blue) : r(red), g(green), b(blue) {}
};
```
- Каждый пиксель изображения имеет три цветовых канала: красный (R), зеленый (G) и синий (B)
- uint8_t - тип данных для целых чисел от 0 до 255 (1 байт)
- Конструкторы позволяют создавать цвет:
1. По умолчанию - черный (0,0,0)
2. С указанными значения RGB

#Тип Image
```
using Image = std::vector<std::vector<Color>>;
```
- Изображение представляется в виде двумерного вектора цветов
- Первый (внеш.) вектор - строки изображения
- Второй (внутр.) - пиксели в строчке

# Генерация рандомного изображения
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
- Создается изображение введенного размера
- std::srand(std::time(0)); - генератор псевдослучайных чисел (текущее время)
- Двойной цикл заполняет каждый пиксель случайным цветом
- Функция возвращает готовое изображение

# Размытие одного конкретного пикселя
```
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
```
- Функция принимает изображение и координаты пикселя (x,y)
- Складывает отдельно каждый цветовой канал
- Двойной цикл проверяет 8 соседних пикселей (3x3) + с сам пиксель
- Есть проверка на границы изображения, чтобы не выйти за пределы
- Вычисляется среднее значение для каждого канала и возвращается новый цвет - результат размытия

# Последовательное размытие
```
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
```
- Создание копии изображения для рез-а
- С помощью двойного цикла проходимся по каждому пикселю, вызываем для него функцию размытия
- Результат размытия записываем в копию и возвращает размытое изображение

# Параллельное размытие с использованием потоков
```
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
```
- Создание копии изображения для результатов
- Создание вектора для хранения потоков
- Вычисление высоты полосы для каждого потока
- Лямбда-функция blur_task, которая обрабатывает свою часть изображения
 - Создание потоков:
1. Каждый поток получает свой диапазон строк (start_y, end_y)
2. Последний поток обрабатывает оставшиеся строки
3. Ожидание завершения всех потоков (thread.join())

# Измерение времени выполнения
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

# Пример с атомарными операциями
```
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
```
- Создается счетчик и мьютекс для его защиты
- Запускаются потоки, каждый увеличивает счетчик
- Замеряется время выполнения и то же самое повторяется с atomic-счетчиком 

## Ответы на вопросы 
1. Результаты сравнения производительности
Sequential blur took 308 ms
Parallel blur (4 threads) took 124 ms
Atomic operations example:
 Mutex counter: 4000000, time: 139 ms
 Atomic counter: 4000000, time: 57 ms

Много поточность помогает распалаллелить вычисления на несколько ядер процессора
Уменьшить общее время выполнения
Использовать ресурсы намного эффективнее

Проблемы синхронизации и их решение
Основные проблемы:
Когда несколько потоков обращаются к общим данным
Взаимоблокировки
Проблемы с производительностью

Способы решения:
Мьютексы (std::mutex) - блокируют доступ к общим данным
Атомарные операции (std::atomic) - для простых операций (быстрее)
Разделение данных - чтобы потоки работали с разными участками памяти
