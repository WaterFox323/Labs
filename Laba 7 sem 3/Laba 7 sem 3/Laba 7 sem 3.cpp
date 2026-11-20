// 7 лаба
#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <algorithm>
#include <fstream>
#include <limits>
#include <cstdlib>
#include <ctime>
#ifdef _WIN32
#define NOMINMAX // иначе windows.h объявляет макрос max, и numeric_limits<streamsize>::max() не компилируется
#include <windows.h>
#endif

using namespace std;

const int ROWS{ 11 };  // число строк лабиринта (вариант 4: 11 x 15)
const int COLS{ 15 };  // число столбцов лабиринта
const int CELL{ 40 };  // размер клетки на рисунке, пикселей
const int NO_MARK{ -1 }; // клетка ещё не помечена волной
const int DR[4]{ -1, 0, 1, 0 }; // смещения по строке: вверх, вправо, вниз, влево
const int DC[4]{ 0, 1, 0, -1 }; // смещения по столбцу

// Клетка лабиринта: номер строки и номер столбца (нумерация с 0).
struct Cell
{
    int row;
    int col;
};

// ---------- Вспомогательная функция ввода ----------

// Выводит приглашение prompt и читает целое число.
// Если введено не число, сообщает об ошибке, очищает поток и спрашивает снова.
int readInt(string prompt)
{
    int x{};
    bool ok{ false }; // введено ли целое число
    while (!ok)
    {
        cout << prompt;
        if (cin >> x)
        {
            ok = true;
        }
        else
        {
            cout << "  Ошибка: нужно ввести целое число." << endl;
            cin.clear(); // сбрасываем флаг ошибки потока
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // убираем остаток строки (или неверный ввод)
    }
    return x;
}

// ---------- Класс «Лабиринт» ----------

class Maze
{
public:
    // Конструктор: создаёт лабиринт p_rows x p_cols, все клетки проходимы.
    Maze(int p_rows, int p_cols) : rows{ p_rows }, cols{ p_cols }, cells(p_rows, vector<int>(p_cols, 0))
    {
    }
    // Конструктор без параметров: лабиринт размера варианта 4.
    Maze() : Maze(ROWS, COLS)
    {
    }

    // Заполняет лабиринт датчиком случайных чисел: клетка становится стеной
    // с вероятностью p_wallPercent процентов. Вход и выход всегда проходимы.
    void generate(int p_wallPercent)
    {
        for (int i = 0; i < rows; i++)
        {
            for (int j = 0; j < cols; j++)
            {
                cells[i][j] = (rand() % 100 < p_wallPercent) ? 1 : 0;
            }
        }
        cells[0][0] = 0;               // вход - клетка (1, 1)
        cells[rows - 1][cols - 1] = 0; // выход - клетка (m, n)
    }

    int getRows() const
    {
        return rows;
    }
    int getCols() const
    {
        return cols;
    }
    Cell getStart() const
    {
        return { 0, 0 };
    }
    Cell getFinish() const
    {
        return { rows - 1, cols - 1 };
    }

    // Возвращает true, если клетка (r, c) лежит внутри лабиринта.
    bool isInside(int r, int c) const
    {
        return r >= 0 && r < rows && c >= 0 && c < cols;
    }
    // Возвращает true, если клетка (r, c) внутри лабиринта и проходима.
    bool isFree(int r, int c) const
    {
        return isInside(r, c) && cells[r][c] == 0;
    }

    // Программная проверка пути: путь начинается во входе, кончается в выходе,
    // идёт только по проходимым клеткам, и каждый ход - в соседнюю клетку.
    bool checkPath(const vector<Cell>& p_path) const
    {
        bool result{ true }; // результат проверки
        if (p_path.empty()
            || p_path.front().row != getStart().row || p_path.front().col != getStart().col
            || p_path.back().row != getFinish().row || p_path.back().col != getFinish().col)
        {
            result = false; // путь пуст, или не начинается во входе, или не кончается в выходе
        }
        else
        {
            int k = 0; // номер проверяемой клетки пути
            while (k < static_cast<int>(p_path.size()) && result)
            {
                if (!isFree(p_path[k].row, p_path[k].col)
                    || (k > 0 && abs(p_path[k].row - p_path[k - 1].row) + abs(p_path[k].col - p_path[k - 1].col) != 1))
                {
                    result = false; // клетка - стена, или ход не в соседнюю клетку
                }
                k++;
            }
        }
        return result;
    }

    // Выводит лабиринт в консоль: # - стена, . - проход, * - путь, S - вход, F - выход.
    void print(const vector<Cell>& p_path) const
    {
        vector<string> view(rows, string(cols, '.'));
        for (int i = 0; i < rows; i++)
        {
            for (int j = 0; j < cols; j++)
            {
                if (cells[i][j] == 1)
                {
                    view[i][j] = '#';
                }
            }
        }
        for (int k = 0; k < static_cast<int>(p_path.size()); k++)
        {
            view[p_path[k].row][p_path[k].col] = '*';
        }
        view[0][0] = 'S';
        view[rows - 1][cols - 1] = 'F';
        for (int i = 0; i < rows; i++)
        {
            cout << "  ";
            for (int j = 0; j < cols; j++)
            {
                cout << view[i][j] << ' ';
            }
            cout << endl;
        }
    }

private:
    int rows;                  // число строк m
    int cols;                  // число столбцов n
    vector<vector<int>> cells; // матрица A: 0 - клетка проходима, 1 - стена (бесконечность)
};

// ---------- Абстрактный класс «Поиск пути» ----------

class PathFinder
{
public:
    virtual ~PathFinder() = default;
    // Находит кратчайший путь от входа к выходу; если пути нет, возвращает пустой вектор.
    virtual vector<Cell> findPath(const Maze& p_maze) = 0;
    // Возвращает название алгоритма.
    virtual string getName() const = 0;
    // Возвращает число шагов алгоритма (обработанных клеток) при последнем поиске.
    long getSteps() const
    {
        return steps;
    }

protected:
    long steps{}; // счётчик шагов - для сравнения алгоритмов
};

// ---------- Волновой алгоритм (поиск в ширину) ----------

class WaveFinder : public PathFinder
{
public:
    string getName() const override
    {
        return "Волновой алгоритм";
    }

    vector<Cell> findPath(const Maze& p_maze) override
    {
        steps = 0;
        int rows = p_maze.getRows();
        int cols = p_maze.getCols();
        Cell start = p_maze.getStart();
        Cell finish = p_maze.getFinish();
        vector<vector<int>> mark(rows, vector<int>(cols, NO_MARK)); // пометки - номер шага волны
        queue<Cell> wave; // очередь клеток фронта волны

        // 1. Распространение волны.
        mark[start.row][start.col] = 0;
        wave.push(start);
        while (!wave.empty() && mark[finish.row][finish.col] == NO_MARK)
        {
            Cell cur = wave.front();
            wave.pop();
            steps++;
            for (int d = 0; d < 4; d++)
            {
                int r = cur.row + DR[d];
                int c = cur.col + DC[d];
                if (p_maze.isFree(r, c) && mark[r][c] == NO_MARK)
                {
                    mark[r][c] = mark[cur.row][cur.col] + 1;
                    wave.push({ r, c });
                }
            }
        }

        vector<Cell> path; // если волна не дошла до выхода, путь останется пустым - пути нет
        if (mark[finish.row][finish.col] != NO_MARK)
        {
            // 2. Обратный ход: от выхода к входу по клеткам с пометкой на 1 меньше.
            Cell cur = finish;
            path.push_back(cur);
            while (mark[cur.row][cur.col] != 0)
            {
                int d = 0;          // номер направления
                bool found{ false }; // найден ли сосед с пометкой на 1 меньше
                while (d < 4 && !found)
                {
                    int r = cur.row + DR[d];
                    int c = cur.col + DC[d];
                    if (p_maze.isInside(r, c) && mark[r][c] == mark[cur.row][cur.col] - 1)
                    {
                        cur = { r, c };
                        found = true;
                    }
                    else
                    {
                        d++; // переходим к следующему направлению
                    }
                }
                path.push_back(cur);
            }
            reverse(path.begin(), path.end()); // путь собран с конца - разворачиваем
        }
        return path;
    }
};

// ---------- Перебор с возвратом (поиск в глубину) ----------

class BacktrackFinder : public PathFinder
{
public:
    string getName() const override
    {
        return "Перебор с возвратом";
    }

    vector<Cell> findPath(const Maze& p_maze) override
    {
        steps = 0;
        way.clear();
        optimalWay.clear();
        dist.assign(p_maze.getRows(), vector<int>(p_maze.getCols(), numeric_limits<int>::max()));
        Cell start = p_maze.getStart();
        dist[start.row][start.col] = 0;
        way.push_back(start); // стек Way - текущий путь
        tryStep(p_maze, start);
        return optimalWay;
    }

private:
    // Рекурсивный шаг перебора из клетки cur (она уже лежит на вершине стека way).
    void tryStep(const Maze& p_maze, Cell cur)
    {
        steps++;
        Cell finish = p_maze.getFinish();
        if (cur.row == finish.row && cur.col == finish.col)
        {
            if (optimalWay.empty() || way.size() < optimalWay.size())
            {
                optimalWay = way; // найден более короткий путь
            }
        }
        else
        {
            int len = static_cast<int>(way.size()); // столько ходов будет сделано до соседа
            for (int d = 0; d < 4; d++)
            {
                int r = cur.row + DR[d];
                int c = cur.col + DC[d];
                bool better = optimalWay.empty() || len + 1 < static_cast<int>(optimalWay.size());
                // Идём в соседа, только если он проходим, попадаем в него короче, чем раньше
                // (значит, его нет в Way), и текущий путь ещё может стать оптимальным.
                if (p_maze.isFree(r, c) && len < dist[r][c] && better)
                {
                    dist[r][c] = len;
                    way.push_back({ r, c }); // добавить Neighbor в Way
                    tryStep(p_maze, { r, c });
                    way.pop_back();          // возврат: извлечь из Way
                }
            }
        }
    }

    vector<Cell> way;          // текущий путь (стек Way)
    vector<Cell> optimalWay;   // лучший найденный путь OptimalWay
    vector<vector<int>> dist;  // Dist: наименьшее число ходов, за которое дошли до клетки
};

// ---------- Класс «Рисунок лабиринта» ----------

class MazePicture;
#ifdef _WIN32
const MazePicture* shownPicture{ nullptr }; // рисунок, который сейчас показан в окне
LRESULT CALLBACK MazeWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

class MazePicture
{
public:
    // Конструктор: рисует лабиринт p_maze и путь p_path в буфер пикселей.
    MazePicture(const Maze& p_maze, const vector<Cell>& p_path)
        : width{ p_maze.getCols() * CELL }, height{ p_maze.getRows() * CELL }, pixels(width * height, WHITE)
    {
        for (int i = 0; i < p_maze.getRows(); i++)
        {
            for (int j = 0; j < p_maze.getCols(); j++)
            {
                unsigned color = p_maze.isFree(i, j) ? WHITE : WALL_COLOR;
                if (i == 0 && j == 0)
                {
                    color = START_COLOR;
                }
                if (i == p_maze.getRows() - 1 && j == p_maze.getCols() - 1)
                {
                    color = FINISH_COLOR;
                }
                fillRect(j * CELL, i * CELL, CELL, CELL, GRID_COLOR);              // рамка клетки
                fillRect(j * CELL + 1, i * CELL + 1, CELL - 2, CELL - 2, color); // сама клетка
            }
        }
        // Путь - красная линия через центры клеток.
        for (int k = 1; k < static_cast<int>(p_path.size()); k++)
        {
            int x1 = p_path[k - 1].col * CELL + CELL / 2, y1 = p_path[k - 1].row * CELL + CELL / 2;
            int x2 = p_path[k].col * CELL + CELL / 2, y2 = p_path[k].row * CELL + CELL / 2;
            fillRect(min(x1, x2) - 3, min(y1, y2) - 3, abs(x2 - x1) + 7, abs(y2 - y1) + 7, PATH_COLOR);
        }
    }

    // Сохраняет рисунок в файл формата BMP (24 бита на пиксель).
    bool saveBmp(string p_fileName) const
    {
        ofstream out(p_fileName, ios::binary);
        bool opened = static_cast<bool>(out); // удалось ли открыть файл
        if (opened)
        {
            int rowSize = (width * 3 + 3) / 4 * 4; // строка BMP дополняется до кратной 4 байтам
            int dataSize = rowSize * height;
            out << 'B' << 'M';
            writeInt(out, 54 + dataSize, 4); // размер файла
            writeInt(out, 0, 4);             // зарезервировано
            writeInt(out, 54, 4);            // смещение до пикселей
            writeInt(out, 40, 4);            // размер заголовка BITMAPINFOHEADER
            writeInt(out, width, 4);
            writeInt(out, height, 4);
            writeInt(out, 1, 2);             // число плоскостей
            writeInt(out, 24, 2);            // бит на пиксель
            writeInt(out, 0, 4);             // без сжатия
            writeInt(out, dataSize, 4);
            writeInt(out, 2835, 4);          // 72 точки на дюйм
            writeInt(out, 2835, 4);
            writeInt(out, 0, 4);
            writeInt(out, 0, 4);
            for (int y = height - 1; y >= 0; y--) // в BMP строки идут снизу вверх
            {
                for (int x = 0; x < width; x++)
                {
                    unsigned p = pixels[y * width + x];
                    out.put(static_cast<char>(p & 0xFF));         // синий
                    out.put(static_cast<char>((p >> 8) & 0xFF));  // зелёный
                    out.put(static_cast<char>((p >> 16) & 0xFF)); // красный
                }
                for (int k = width * 3; k < rowSize; k++)
                {
                    out.put(0);
                }
            }
        }
        return opened;
    }

    // Показывает рисунок в графическом окне (в Windows). Возврат - после закрытия окна.
    void show() const
    {
#ifdef _WIN32
        shownPicture = this;
        HINSTANCE hInstance = GetModuleHandleW(nullptr);
        WNDCLASSW wc{};
        wc.lpfnWndProc = MazeWindowProc;
        wc.hInstance = hInstance;
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
        wc.lpszClassName = L"MazeWindow";
        RegisterClassW(&wc); // при повторном показе класс уже зарегистрирован - это не ошибка
        DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
        RECT rect{ 0, 0, width, height };
        AdjustWindowRect(&rect, style, FALSE); // размер окна вместе с рамкой и заголовком
        HWND hwnd = CreateWindowW(L"MazeWindow", L"Лабиринт 11 x 15 (закройте окно, чтобы вернуться в меню)",
            style, CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top,
            nullptr, nullptr, hInstance, nullptr);
        ShowWindow(hwnd, SW_SHOW);
        UpdateWindow(hwnd);
        MSG msg;
        while (GetMessageW(&msg, nullptr, 0, 0) > 0) // цикл обработки сообщений окна
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        shownPicture = nullptr;
#else
        cout << "Графическое окно доступно только в Windows, рисунок сохранён в файл maze.bmp." << endl;
#endif
    }

#ifdef _WIN32
    // Выводит буфер пикселей в окно (вызывается при сообщении WM_PAINT).
    void paint(HDC hdc) const
    {
        BITMAPINFO info{};
        info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        info.bmiHeader.biWidth = width;
        info.bmiHeader.biHeight = -height; // минус - строки идут сверху вниз
        info.bmiHeader.biPlanes = 1;
        info.bmiHeader.biBitCount = 32;
        info.bmiHeader.biCompression = BI_RGB;
        SetDIBitsToDevice(hdc, 0, 0, width, height, 0, 0, 0, height, pixels.data(), &info, DIB_RGB_COLORS);
    }
#endif

private:
    // Закрашивает прямоугольник цветом color (0xRRGGBB), обрезая его по границам рисунка.
    void fillRect(int x, int y, int w, int h, unsigned color)
    {
        for (int yy = max(y, 0); yy < min(y + h, height); yy++)
        {
            for (int xx = max(x, 0); xx < min(x + w, width); xx++)
            {
                pixels[yy * width + xx] = color;
            }
        }
    }
    // Записывает целое число в файл младшими байтами вперёд.
    static void writeInt(ofstream& out, int value, int bytes)
    {
        for (int k = 0; k < bytes; k++)
        {
            out.put(static_cast<char>((value >> (8 * k)) & 0xFF));
        }
    }

    static constexpr unsigned WHITE{ 0xFFFFFF };
    static constexpr unsigned WALL_COLOR{ 0x3C3C3C };   // стена - тёмно-серая
    static constexpr unsigned GRID_COLOR{ 0xC8C8C8 };   // линии сетки
    static constexpr unsigned START_COLOR{ 0x6CCB6C };  // вход - зелёный
    static constexpr unsigned FINISH_COLOR{ 0x6C9CEB }; // выход - синий
    static constexpr unsigned PATH_COLOR{ 0xE02020 };   // путь - красный
    int width;               // ширина рисунка, пикселей
    int height;              // высота рисунка, пикселей
    vector<unsigned> pixels; // буфер пикселей 0xRRGGBB, строки сверху вниз
};

#ifdef _WIN32
// Оконная процедура: перерисовывает окно и завершает цикл сообщений при закрытии.
LRESULT CALLBACK MazeWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        if (shownPicture != nullptr)
        {
            shownPicture->paint(hdc);
        }
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}
#endif

// ---------- Клиентская программа ----------

// Ищет путь алгоритмом finder, выводит длину, число шагов, путь и лабиринт.
// Возвращает найденный путь (пустой, если пути нет).
vector<Cell> solve(PathFinder& finder, const Maze& maze)
{
    vector<Cell> path = finder.findPath(maze);
    cout << finder.getName() << ":" << endl;
    if (path.empty())
    {
        cout << "  Пути от входа к выходу нет." << endl;
    }
    else
    {
        cout << "  Длина кратчайшего пути (число ходов): " << path.size() - 1 << ", клеток в пути: " << path.size() << endl;
        cout << "  Шагов алгоритма: " << finder.getSteps() << endl;
        cout << "  Путь (строка, столбец):";
        for (int k = 0; k < static_cast<int>(path.size()); k++)
        {
            if (k % 10 == 0)
            {
                cout << endl << "   "; // по 10 клеток в строке
            }
            cout << " (" << path[k].row + 1 << "," << path[k].col + 1 << ")";
        }
        cout << endl;
        cout << "  Проверка пути: " << (maze.checkPath(path) ? "путь верный" : "ОШИБКА") << endl;
    }
    maze.print(path);
    return path;
}

// Запускает оба алгоритма на лабиринте maze и сравнивает результаты.
void compare(const Maze& maze)
{
    WaveFinder wave;
    BacktrackFinder backtrack;
    PathFinder* finders[2]{ &wave, &backtrack }; // работаем через указатель на базовый класс
    int lengths[2]{};
    bool valid[2]{};
    for (int i = 0; i < 2; i++)
    {
        vector<Cell> path = finders[i]->findPath(maze);
        lengths[i] = static_cast<int>(path.size()) - 1; // -1 означает «пути нет»
        valid[i] = path.empty() || maze.checkPath(path);
        cout << "  " << finders[i]->getName() << ": ";
        if (path.empty())
        {
            cout << "пути нет";
        }
        else
        {
            cout << "длина пути " << lengths[i];
        }
        cout << ", шагов алгоритма " << finders[i]->getSteps() << endl;
    }
    if (lengths[0] == lengths[1] && valid[0] && valid[1])
    {
        if (lengths[0] < 0)
        {
            cout << "  Проверка пройдена: оба алгоритма сообщили, что пути нет." << endl;
        }
        else
        {
            cout << "  Проверка пройдена: оба алгоритма дали верный путь одинаковой длины." << endl;
        }
    }
    else
    {
        cout << "  Проверка НЕ пройдена!" << endl;
    }
}

// Главная функция: меню для генерации лабиринта, поиска пути и вывода в окно.
int main()
{
    setlocale(LC_ALL, "rus");
#ifdef _WIN32
    SetConsoleCP(1251);       // ввод русских букв с клавиатуры
    SetConsoleOutputCP(1251); // вывод русских букв
#endif

    Maze maze;                // лабиринт 11 x 15
    vector<Cell> lastPath;    // последний найденный путь - для рисунка
    WaveFinder wave;
    BacktrackFinder backtrack;
    unsigned seed = static_cast<unsigned>(time(nullptr));
    srand(seed);              // настройка датчика случайных чисел
    maze.generate(30);
    cout << "Сгенерирован лабиринт " << ROWS << " x " << COLS << " (30% стен, начальное значение датчика " << seed << ")" << endl;
    maze.print(lastPath);

    int choice; // пункт меню
    do
    {
        cout << "\n===== Лабиринт " << ROWS << " x " << COLS << " (вариант 4) =====" << endl;
        cout << "1 - сгенерировать новый лабиринт" << endl;
        cout << "2 - найти путь волновым алгоритмом" << endl;
        cout << "3 - найти путь перебором с возвратом" << endl;
        cout << "4 - сравнить алгоритмы (программная проверка)" << endl;
        cout << "5 - показать лабиринт и путь в графическом окне" << endl;
        cout << "0 - выход" << endl;
        choice = readInt("Ваш выбор: ");

        switch (choice)
        {
        case 1:
        {
            int percent = readInt("Процент стен (0-60): ");
            if (percent < 0 || percent > 60)
            {
                cout << "Процент стен должен быть от 0 до 60." << endl;
            }
            else
            {
                int s = readInt("Начальное значение датчика (0 - по текущему времени): ");
                seed = (s == 0) ? static_cast<unsigned>(time(nullptr)) : static_cast<unsigned>(s);
                srand(seed);
                maze = Maze(ROWS, COLS);
                maze.generate(percent);
                lastPath.clear();
                cout << "Сгенерирован лабиринт (начальное значение датчика " << seed << "):" << endl;
                maze.print(lastPath);
            }
            break;
        }
        case 2:
            lastPath = solve(wave, maze);
            break;
        case 3:
            lastPath = solve(backtrack, maze);
            break;
        case 4:
            compare(maze);
            break;
        case 5:
        {
            MazePicture picture(maze, lastPath);
            if (picture.saveBmp("maze.bmp"))
            {
                cout << "Рисунок сохранён в файл maze.bmp." << endl;
            }
            picture.show();
            break;
        }
        case 0:
            cout << "Работа программы завершена." << endl;
            break;
        default:
            cout << "Нет такого пункта меню." << endl;
        }
    } while (choice != 0);

    system("PAUSE");
    return 0;
}