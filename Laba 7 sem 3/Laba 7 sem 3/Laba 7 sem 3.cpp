// 7 лаба
// Поиск кратчайшего пути в лабиринте 11 x 15 (вариант 4).
// Лабиринт генерируется датчиком случайных чисел. Путь от входа (левый верхний угол)
// к выходу (правый нижний угол) ищется двумя переборными алгоритмами:
//   - волновым алгоритмом (поиск в ширину);
//   - перебором с возвратом (поиск в глубину) с отсечением заведомо длинных путей.
// Результат выводится в консоль, сохраняется в файл maze.bmp и показывается в графическом окне.

#include <iostream>  // cin, cout - консольный ввод и вывод
#include <string>    // string - строки
#include <vector>    // vector - динамический массив (матрицы, пути, стеки)
#include <queue>     // queue - очередь для волнового алгоритма
#include <algorithm> // reverse, min, max
#include <fstream>   // ofstream - запись файла maze.bmp
#include <limits>    // numeric_limits - «бесконечность» и очистка потока ввода
#include <cstdlib>   // rand, srand, abs, system
#include <ctime>     // time - начальное значение датчика по текущему времени
#ifdef _WIN32
#define NOMINMAX // иначе windows.h объявляет макрос max, и numeric_limits<streamsize>::max() не компилируется
#include <windows.h> // Windows API - графическое окно и русские буквы в консоли
#endif

using namespace std;

// ---------- Константы программы ----------

const int ROWS{ 11 };  // число строк лабиринта (вариант 4: 11 x 15)
const int COLS{ 15 };  // число столбцов лабиринта
const int CELL{ 40 };  // размер клетки на рисунке, пикселей
const int NO_MARK{ -1 }; // клетка ещё не помечена волной
// Четыре направления хода из клетки. Направление d задаётся парой (DR[d], DC[d]):
// d = 0 - вверх, d = 1 - вправо, d = 2 - вниз, d = 3 - влево.
// Сосед клетки (row, col) в направлении d - это клетка (row + DR[d], col + DC[d]).
const int DR[4]{ -1, 0, 1, 0 }; // смещения по строке: вверх, вправо, вниз, влево
const int DC[4]{ 0, 1, 0, -1 }; // смещения по столбцу

// Клетка лабиринта: номер строки и номер столбца (нумерация с 0).
// Пользователю клетки выводятся с нумерацией с 1: клетка {0, 0} - это клетка (1, 1).
struct Cell
{
    int row; // номер строки
    int col; // номер столбца
};

// ---------- Вспомогательная функция ввода ----------

// Выводит приглашение prompt и читает целое число.
// Если введено не число, сообщает об ошибке, очищает поток и спрашивает снова.
int readInt(string prompt)
{
    int x{};          // введённое число
    bool ok{ false }; // введено ли целое число
    while (!ok)       // спрашиваем, пока не будет введено целое число
    {
        cout << prompt;
        if (cin >> x) // чтение удалось - введено целое число
        {
            ok = true;
        }
        else          // введено не число (например, «abc») - поток перешёл в состояние ошибки
        {
            cout << "  Ошибка: нужно ввести целое число." << endl;
            cin.clear(); // сбрасываем флаг ошибки потока, иначе cin больше ничего не прочитает
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // убираем остаток строки (или неверный ввод)
    }
    return x;
}

// ---------- Класс «Лабиринт» ----------

// Лабиринт - матрица A размером rows x cols: 0 - клетка проходима, 1 - стена.
// В методических указаниях стена обозначена бесконечностью, в программе - единицей.
class Maze
{
public:
    // Конструктор: создаёт лабиринт p_rows x p_cols, все клетки проходимы.
    Maze(int p_rows, int p_cols) : rows{ p_rows }, cols{ p_cols }, cells(p_rows, vector<int>(p_cols, 0))
    {
    }
    // Конструктор без параметров: лабиринт размера варианта 4.
    // Вызывает конструктор с параметрами (делегирующий конструктор).
    Maze() : Maze(ROWS, COLS)
    {
    }

    // Заполняет лабиринт датчиком случайных чисел: клетка становится стеной
    // с вероятностью p_wallPercent процентов. Вход и выход всегда проходимы.
    void generate(int p_wallPercent)
    {
        for (int i = 0; i < rows; i++)     // по всем строкам
        {
            for (int j = 0; j < cols; j++) // по всем столбцам
            {
                // rand() % 100 - случайное число от 0 до 99; оно меньше p_wallPercent
                // ровно в p_wallPercent случаях из 100 - тогда клетка становится стеной.
                cells[i][j] = (rand() % 100 < p_wallPercent) ? 1 : 0;
            }
        }
        cells[0][0] = 0;               // вход - клетка (1, 1)
        cells[rows - 1][cols - 1] = 0; // выход - клетка (m, n)
    }

    // Методы доступа к закрытым полям.
    int getRows() const // число строк
    {
        return rows;
    }
    int getCols() const // число столбцов
    {
        return cols;
    }
    Cell getStart() const // вход - левый верхний угол
    {
        return { 0, 0 };
    }
    Cell getFinish() const // выход - правый нижний угол
    {
        return { rows - 1, cols - 1 };
    }

    // Возвращает true, если клетка (r, c) лежит внутри лабиринта.
    bool isInside(int r, int c) const
    {
        return r >= 0 && r < rows && c >= 0 && c < cols;
    }
    // Возвращает true, если клетка (r, c) внутри лабиринта и проходима.
    // Сначала проверяется isInside, поэтому к cells[r][c] за границей матрицы обращения нет.
    bool isFree(int r, int c) const
    {
        return isInside(r, c) && cells[r][c] == 0;
    }

    // Программная проверка пути: путь начинается во входе, кончается в выходе,
    // идёт только по проходимым клеткам, и каждый ход - в соседнюю клетку.
    bool checkPath(const vector<Cell>& p_path) const
    {
        bool result{ true }; // результат проверки
        // 1. Проверяем концы пути.
        if (p_path.empty()
            || p_path.front().row != getStart().row || p_path.front().col != getStart().col
            || p_path.back().row != getFinish().row || p_path.back().col != getFinish().col)
        {
            result = false; // путь пуст, или не начинается во входе, или не кончается в выходе
        }
        else
        {
            // 2. Проверяем каждую клетку пути, пока не найдена первая ошибка.
            int k = 0; // номер проверяемой клетки пути
            while (k < static_cast<int>(p_path.size()) && result)
            {
                // Соседние по стороне клетки отличаются ровно на 1 по строке или по столбцу,
                // поэтому сумма модулей разностей координат должна быть равна 1.
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
        // view - «картинка» лабиринта из символов; сначала все клетки - проходы.
        vector<string> view(rows, string(cols, '.'));
        // Отмечаем стены.
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
        // Отмечаем клетки пути (если путь пустой, цикл не выполнится ни разу).
        for (int k = 0; k < static_cast<int>(p_path.size()); k++)
        {
            view[p_path[k].row][p_path[k].col] = '*';
        }
        // Вход и выход отмечаем последними, чтобы их не закрыли символы пути.
        view[0][0] = 'S';
        view[rows - 1][cols - 1] = 'F';
        // Выводим картинку построчно, символы разделяем пробелами.
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

// Общий интерфейс алгоритмов поиска пути. Объект этого класса создать нельзя
// (есть чисто виртуальные функции = 0) - создаются объекты производных классов
// WaveFinder и BacktrackFinder, а работать с ними можно через указатель или ссылку на PathFinder.
class PathFinder
{
public:
    // Виртуальный деструктор: объект производного класса правильно удаляется через указатель на базовый.
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
    long steps{}; // счётчик шагов - для сравнения алгоритмов (доступен производным классам)
};

// ---------- Волновой алгоритм (поиск в ширину) ----------

// Этап 1 - распространение волны: вход получает пометку 0, его свободные соседи - 1,
// их непомеченные соседи - 2 и т. д. Пометка клетки равна длине кратчайшего пути до неё.
// Этап 2 - обратный ход: от выхода идём в соседнюю клетку с пометкой на 1 меньше, пока не придём во вход.
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
        mark[start.row][start.col] = 0; // вход - шаг 0
        wave.push(start);
        // Волна идёт, пока есть клетки во фронте и выход ещё не помечен.
        // Очередь выдаёт клетки в порядке возрастания пометок, поэтому каждая клетка
        // помечается один раз и сразу кратчайшим расстоянием.
        while (!wave.empty() && mark[finish.row][finish.col] == NO_MARK)
        {
            Cell cur = wave.front(); // берём клетку из начала очереди
            wave.pop();
            steps++;
            for (int d = 0; d < 4; d++) // просматриваем четырёх соседей клетки cur
            {
                int r = cur.row + DR[d];
                int c = cur.col + DC[d];
                if (p_maze.isFree(r, c) && mark[r][c] == NO_MARK) // сосед проходим и волна в нём ещё не была
                {
                    mark[r][c] = mark[cur.row][cur.col] + 1; // пометка соседа на 1 больше
                    wave.push({ r, c });                     // сосед становится частью фронта волны
                }
            }
        }

        vector<Cell> path; // если волна не дошла до выхода, путь останется пустым - пути нет
        if (mark[finish.row][finish.col] != NO_MARK)
        {
            // 2. Обратный ход: от выхода к входу по клеткам с пометкой на 1 меньше.
            Cell cur = finish;
            path.push_back(cur);
            while (mark[cur.row][cur.col] != 0) // пометка 0 только у входа - дошли до него
            {
                int d = 0;          // номер направления
                bool found{ false }; // найден ли сосед с пометкой на 1 меньше
                // Такой сосед всегда есть: из него волна и пришла в клетку cur.
                while (d < 4 && !found)
                {
                    int r = cur.row + DR[d];
                    int c = cur.col + DC[d];
                    if (p_maze.isInside(r, c) && mark[r][c] == mark[cur.row][cur.col] - 1)
                    {
                        cur = { r, c }; // переходим в этого соседа
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

// Текущий путь хранится в стеке Way. Из последней клетки Way пробуем пойти в каждого соседа;
// после рекурсивного перебора из соседа извлекаем его из Way (возврат) и пробуем следующего.
// Лучший найденный путь хранится в OptimalWay.
// Отсечение: путь не продолжается, если он уже не короче OptimalWay
// или если в клетку уже приходили не длиннее, чем сейчас (массив Dist).
class BacktrackFinder : public PathFinder
{
public:
    string getName() const override
    {
        return "Перебор с возвратом";
    }

    // Подготовка и запуск перебора.
    vector<Cell> findPath(const Maze& p_maze) override
    {
        steps = 0;
        way.clear();
        optimalWay.clear();
        // Dist всех клеток - «бесконечность»: до них ещё не доходили.
        dist.assign(p_maze.getRows(), vector<int>(p_maze.getCols(), numeric_limits<int>::max()));
        Cell start = p_maze.getStart();
        dist[start.row][start.col] = 0; // до входа - 0 ходов
        way.push_back(start); // стек Way - текущий путь
        tryStep(p_maze, start);
        return optimalWay;    // пустой, если выход недостижим
    }

private:
    // Рекурсивный шаг перебора из клетки cur (она уже лежит на вершине стека way).
    void tryStep(const Maze& p_maze, Cell cur)
    {
        steps++;
        Cell finish = p_maze.getFinish();
        if (cur.row == finish.row && cur.col == finish.col) // дошли до выхода - вариант готов
        {
            if (optimalWay.empty() || way.size() < optimalWay.size())
            {
                optimalWay = way; // найден более короткий путь
            }
        }
        else // выход ещё не достигнут - пробуем сделать ход в каждого соседа
        {
            int len = static_cast<int>(way.size()); // столько ходов будет сделано до соседа
            for (int d = 0; d < 4; d++)
            {
                int r = cur.row + DR[d];
                int c = cur.col + DC[d];
                // better - путь через соседа (len + 1 клеток) ещё может оказаться короче OptimalWay.
                bool better = optimalWay.empty() || len + 1 < static_cast<int>(optimalWay.size());
                // Идём в соседа, только если он проходим, попадаем в него короче, чем раньше
                // (значит, его нет в Way), и текущий путь ещё может стать оптимальным.
                if (p_maze.isFree(r, c) && len < dist[r][c] && better)
                {
                    dist[r][c] = len;
                    way.push_back({ r, c }); // добавить Neighbor в Way
                    tryStep(p_maze, { r, c }); // перебор всех продолжений из соседа
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

// Объявления нужны заранее: оконная процедура и указатель на показываемый рисунок
// используются в методе show(), а описаны после класса.
class MazePicture;
#ifdef _WIN32
const MazePicture* shownPicture{ nullptr }; // рисунок, который сейчас показан в окне
LRESULT CALLBACK MazeWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

// Рисунок лабиринта и пути. Сначала всё рисуется в буфер пикселей pixels,
// затем буфер сохраняется в файл BMP и выводится в графическое окно.
class MazePicture
{
public:
    // Конструктор: рисует лабиринт p_maze и путь p_path в буфер пикселей.
    MazePicture(const Maze& p_maze, const vector<Cell>& p_path)
        : width{ p_maze.getCols() * CELL }, height{ p_maze.getRows() * CELL }, pixels(width * height, WHITE)
    {
        // Рисуем клетки: каждая клетка - квадрат CELL x CELL пикселей.
        for (int i = 0; i < p_maze.getRows(); i++)
        {
            for (int j = 0; j < p_maze.getCols(); j++)
            {
                // Цвет клетки: проход - белый, стена - тёмно-серый, вход - зелёный, выход - синий.
                unsigned color = p_maze.isFree(i, j) ? WHITE : WALL_COLOR;
                if (i == 0 && j == 0)
                {
                    color = START_COLOR;
                }
                if (i == p_maze.getRows() - 1 && j == p_maze.getCols() - 1)
                {
                    color = FINISH_COLOR;
                }
                // Столбец j - это координата x, строка i - координата y.
                fillRect(j * CELL, i * CELL, CELL, CELL, GRID_COLOR);              // рамка клетки
                fillRect(j * CELL + 1, i * CELL + 1, CELL - 2, CELL - 2, color); // сама клетка
            }
        }
        // Путь - красная линия через центры клеток.
        // Каждый ход соединяет центры двух соседних клеток отрезком толщиной 7 пикселей.
        for (int k = 1; k < static_cast<int>(p_path.size()); k++)
        {
            int x1 = p_path[k - 1].col * CELL + CELL / 2, y1 = p_path[k - 1].row * CELL + CELL / 2; // центр предыдущей клетки
            int x2 = p_path[k].col * CELL + CELL / 2, y2 = p_path[k].row * CELL + CELL / 2;         // центр текущей клетки
            fillRect(min(x1, x2) - 3, min(y1, y2) - 3, abs(x2 - x1) + 7, abs(y2 - y1) + 7, PATH_COLOR);
        }
    }

    // Сохраняет рисунок в файл формата BMP (24 бита на пиксель).
    // Файл BMP: заголовок файла (14 байт) + заголовок BITMAPINFOHEADER (40 байт) + пиксели.
    bool saveBmp(string p_fileName) const
    {
        ofstream out(p_fileName, ios::binary);
        bool opened = static_cast<bool>(out); // удалось ли открыть файл
        if (opened)
        {
            int rowSize = (width * 3 + 3) / 4 * 4; // строка BMP дополняется до кратной 4 байтам
            int dataSize = rowSize * height;       // размер пиксельных данных, байт
            // Заголовок файла (BITMAPFILEHEADER).
            out << 'B' << 'M';                     // подпись формата BMP
            writeInt(out, 54 + dataSize, 4); // размер файла
            writeInt(out, 0, 4);             // зарезервировано
            writeInt(out, 54, 4);            // смещение до пикселей
            // Заголовок изображения (BITMAPINFOHEADER).
            writeInt(out, 40, 4);            // размер заголовка BITMAPINFOHEADER
            writeInt(out, width, 4);         // ширина, пикселей
            writeInt(out, height, 4);        // высота, пикселей
            writeInt(out, 1, 2);             // число плоскостей
            writeInt(out, 24, 2);            // бит на пиксель
            writeInt(out, 0, 4);             // без сжатия
            writeInt(out, dataSize, 4);      // размер пиксельных данных
            writeInt(out, 2835, 4);          // 72 точки на дюйм
            writeInt(out, 2835, 4);
            writeInt(out, 0, 4);             // число цветов палитры - палитры нет
            writeInt(out, 0, 4);             // число важных цветов - все
            // Пиксели: каждый пиксель - 3 байта в порядке синий, зелёный, красный.
            for (int y = height - 1; y >= 0; y--) // в BMP строки идут снизу вверх
            {
                for (int x = 0; x < width; x++)
                {
                    unsigned p = pixels[y * width + x];
                    out.put(static_cast<char>(p & 0xFF));         // синий
                    out.put(static_cast<char>((p >> 8) & 0xFF));  // зелёный
                    out.put(static_cast<char>((p >> 16) & 0xFF)); // красный
                }
                for (int k = width * 3; k < rowSize; k++) // нули для выравнивания строки
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
        shownPicture = this; // оконная процедура будет рисовать этот рисунок
        // Регистрируем класс окна: какая процедура обрабатывает сообщения, курсор, фон, имя класса.
        HINSTANCE hInstance = GetModuleHandleW(nullptr);
        WNDCLASSW wc{};
        wc.lpfnWndProc = MazeWindowProc;
        wc.hInstance = hInstance;
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
        wc.lpszClassName = L"MazeWindow";
        RegisterClassW(&wc); // при повторном показе класс уже зарегистрирован - это не ошибка
        // Окно с заголовком и кнопками «свернуть» и «закрыть», без изменения размера.
        DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
        RECT rect{ 0, 0, width, height };
        AdjustWindowRect(&rect, style, FALSE); // размер окна вместе с рамкой и заголовком
        HWND hwnd = CreateWindowW(L"MazeWindow", L"Лабиринт 11 x 15 (закройте окно, чтобы вернуться в меню)",
            style, CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top,
            nullptr, nullptr, hInstance, nullptr);
        ShowWindow(hwnd, SW_SHOW);
        UpdateWindow(hwnd); // сразу отправляет окну сообщение WM_PAINT
        // Цикл обработки сообщений: работает, пока окно не закрыто
        // (при закрытии PostQuitMessage посылает WM_QUIT, и GetMessageW возвращает 0).
        MSG msg;
        while (GetMessageW(&msg, nullptr, 0, 0) > 0) // цикл обработки сообщений окна
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg); // передаёт сообщение оконной процедуре MazeWindowProc
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
        // Описание буфера pixels: 32 бита на пиксель, без сжатия.
        BITMAPINFO info{};
        info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        info.bmiHeader.biWidth = width;
        info.bmiHeader.biHeight = -height; // минус - строки идут сверху вниз
        info.bmiHeader.biPlanes = 1;
        info.bmiHeader.biBitCount = 32;
        info.bmiHeader.biCompression = BI_RGB;
        // Копируем весь буфер в окно, начиная с левого верхнего угла.
        SetDIBitsToDevice(hdc, 0, 0, width, height, 0, 0, 0, height, pixels.data(), &info, DIB_RGB_COLORS);
    }
#endif

private:
    // Закрашивает прямоугольник цветом color (0xRRGGBB), обрезая его по границам рисунка.
    // (x, y) - левый верхний угол, w и h - ширина и высота прямоугольника.
    void fillRect(int x, int y, int w, int h, unsigned color)
    {
        for (int yy = max(y, 0); yy < min(y + h, height); yy++)
        {
            for (int xx = max(x, 0); xx < min(x + w, width); xx++)
            {
                pixels[yy * width + xx] = color; // пиксель (xx, yy) хранится в элементе yy * width + xx
            }
        }
    }
    // Записывает целое число в файл младшими байтами вперёд.
    // Так требует формат BMP: например, 54 записывается как байты 36 00 00 00 (в шестнадцатеричной системе).
    static void writeInt(ofstream& out, int value, int bytes)
    {
        for (int k = 0; k < bytes; k++)
        {
            out.put(static_cast<char>((value >> (8 * k)) & 0xFF)); // k-й байт числа
        }
    }

    // Цвета в формате 0xRRGGBB (красный, зелёный, синий).
    static constexpr unsigned WHITE{ 0xFFFFFF };        // проход - белый
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
// Windows вызывает её для каждого сообщения окна.
LRESULT CALLBACK MazeWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_PAINT: // окно нужно перерисовать (показ, разворачивание после сворачивания)
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps); // контекст устройства - «холст» окна
        if (shownPicture != nullptr)
        {
            shownPicture->paint(hdc);
        }
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_DESTROY: // окно закрыто
        PostQuitMessage(0); // завершает цикл сообщений в show()
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam); // остальные сообщения - обработка по умолчанию
}
#endif

// ---------- Клиентская программа ----------

// Ищет путь алгоритмом finder, выводит длину, число шагов, путь и лабиринт.
// Возвращает найденный путь (пустой, если пути нет).
// finder - ссылка на базовый класс: сюда можно передать и WaveFinder, и BacktrackFinder,
// а нужный findPath() выберется во время выполнения (виртуальная функция).
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
        // Длина пути - число ходов, оно на 1 меньше числа клеток.
        cout << "  Длина кратчайшего пути (число ходов): " << path.size() - 1 << ", клеток в пути: " << path.size() << endl;
        cout << "  Шагов алгоритма: " << finder.getSteps() << endl;
        cout << "  Путь (строка, столбец):";
        for (int k = 0; k < static_cast<int>(path.size()); k++)
        {
            if (k % 10 == 0)
            {
                cout << endl << "   "; // по 10 клеток в строке
            }
            cout << " (" << path[k].row + 1 << "," << path[k].col + 1 << ")"; // нумерация с 1
        }
        cout << endl;
        cout << "  Проверка пути: " << (maze.checkPath(path) ? "путь верный" : "ОШИБКА") << endl;
    }
    maze.print(path);
    return path;
}

// Запускает оба алгоритма на лабиринте maze и сравнивает результаты.
// Проверка пройдена, если оба результата верны и длины путей совпадают
// (оба алгоритма ищут кратчайший путь, поэтому длины обязаны быть равны).
void compare(const Maze& maze)
{
    WaveFinder wave;
    BacktrackFinder backtrack;
    PathFinder* finders[2]{ &wave, &backtrack }; // работаем через указатель на базовый класс
    int lengths[2]{}; // длины найденных путей
    bool valid[2]{};  // верен ли результат алгоритма
    for (int i = 0; i < 2; i++)
    {
        vector<Cell> path = finders[i]->findPath(maze);
        lengths[i] = static_cast<int>(path.size()) - 1; // -1 означает «пути нет»
        valid[i] = path.empty() || maze.checkPath(path); // «пути нет» тоже верный ответ - проверяется совпадением длин
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
    WaveFinder wave;          // волновой алгоритм
    BacktrackFinder backtrack; // перебор с возвратом
    // Начальный лабиринт: 30 % стен, датчик настроен по текущему времени.
    unsigned seed = static_cast<unsigned>(time(nullptr));
    srand(seed);              // настройка датчика случайных чисел
    maze.generate(30);
    cout << "Сгенерирован лабиринт " << ROWS << " x " << COLS << " (30% стен, начальное значение датчика " << seed << ")" << endl;
    maze.print(lastPath);

    // Меню повторяется, пока не выбран пункт 0 (условие проверяется в конце цикла).
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
        case 1: // новый лабиринт
        {
            int percent = readInt("Процент стен (0-60): ");
            if (percent < 0 || percent > 60)
            {
                cout << "Процент стен должен быть от 0 до 60." << endl;
            }
            else
            {
                // Одно и то же начальное значение датчика даёт один и тот же лабиринт -
                // так можно повторить опыт; 0 - взять текущее время.
                int s = readInt("Начальное значение датчика (0 - по текущему времени): ");
                seed = (s == 0) ? static_cast<unsigned>(time(nullptr)) : static_cast<unsigned>(s);
                srand(seed);
                maze = Maze(ROWS, COLS);
                maze.generate(percent);
                lastPath.clear(); // старый путь к новому лабиринту не относится
                cout << "Сгенерирован лабиринт (начальное значение датчика " << seed << "):" << endl;
                maze.print(lastPath);
            }
            break;
        }
        case 2: // волновой алгоритм
            lastPath = solve(wave, maze);
            break;
        case 3: // перебор с возвратом
            lastPath = solve(backtrack, maze);
            break;
        case 4: // сравнение алгоритмов
            compare(maze);
            break;
        case 5: // рисунок: файл maze.bmp и графическое окно
        {
            MazePicture picture(maze, lastPath);
            if (picture.saveBmp("maze.bmp"))
            {
                cout << "Рисунок сохранён в файл maze.bmp." << endl;
            }
            picture.show();
            break;
        }
        case 0: // выход
            cout << "Работа программы завершена." << endl;
            break;
        default: // любое другое число
            cout << "Нет такого пункта меню." << endl;
        }
    } while (choice != 0);

    system("PAUSE"); // окно консоли не закроется, пока не нажата клавиша
    return 0;
}
