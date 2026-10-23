#include <iostream>
#include <vector>
#include <iomanip>

using namespace std;

int n;                          // количество вершин графа
vector<vector<int>> adj;        // матрица смежности (1 - ребро есть, 0 - нет)
vector<vector<int>> L;          // списки смежности L(v)

void addEdge(int u, int v) // функция добавления ребра u - v
{
    adj[u][v] = 1; // ребро u - v
    adj[v][u] = 1; // граф неориентированный
}

int degree(int v) // степень вершины
{
    int d = 0; // счётчик рёбер
    for (int u = 1; u <= n; u++) d += adj[v][u]; // суммируем строку матрицы
    return d; // возвращаем степень
}

void buildLists() // составление списков смежности L(v)
{
    L.assign(n + 1, vector<int>()); // очищаем списки
    for (int v = 1; v <= n; v++) // для каждой вершины
    {
        for (int u = 1; u <= n; u++) // перебираем остальные вершины
        {
            if (adj[v][u])
            {
                L[v].push_back(u); // если смежны - добавляем в список
            }
        }
    }
}

bool isConnected() // проверка связности графа обходом в ширину
{
    vector<bool> visited(n + 1, false); // отметки посещения
    vector<int> queue; // очередь обхода
    queue.push_back(1); // начинаем с вершины 1
    visited[1] = true;
    for (int k = 0; k < (int)queue.size(); k++) // пока очередь не пройдена
    {
        int v = queue[k]; // текущая вершина
        for (int u : L[v]) // перебираем соседей
        {
            if (!visited[u]) // если сосед ещё не посещён
            {
                visited[u] = true; // отмечаем его
                queue.push_back(u); // добавляем в очередь
            }
        }
    }
    return (int)queue.size() == n; // связный, если посещены все вершины
}

bool checkDirac() // достаточное условие Дирака: d(v) >= n/2 для всех v
{
    for (int v = 1; v <= n; v++) // перебираем вершины
    {
        if (2 * degree(v) < n)
        {
            return false; // условие нарушено
        }
    }
    return n >= 3; // условие выполнено
}

bool checkOre() // достаточное условие Оре: d(u) + d(v) >= n для несмежных u, v
{
    for (int u = 1; u <= n; u++) // перебираем пары вершин
    {
        for (int v = u + 1; v <= n; v++)
        {
            if (!adj[u][v] && degree(u) + degree(v) < n)
            {
                return false; // условие нарушено
            }
        }
    }
    return n >= 3; // условие выполнено
}

// Перебор с возвратом (шаги 2-3 алгоритма). cycle = true - ищем цикл, false - путь
bool hamilton(vector<int>& path, vector<bool>& inPath, bool cycle)
{
    int i = (int)path.size(); // текущая длина пути
    if (i == n) // путь содержит все вершины
    {
        return !cycle || adj[path[n - 1]][path[0]]; // для цикла нужно ребро {xp, x1}
    }
    int last = path[i - 1]; // последняя вершина пути
    for (int u : L[last]) // выбираем очередную вершину из списка смежности
    {
        if (inPath[u])
        {
            continue; // вершина уже входит в путь
        }
        path.push_back(u); inPath[u] = true; // добавляем вершину в путь
        if (hamilton(path, inPath, cycle))
        {
            return true; // продолжаем поиск
        }
        path.pop_back(); inPath[u] = false; // возврат на шаг назад
    }
    return false; // продолжить путь нельзя
}

int countCycles(vector<int>& path, vector<bool>& inPath) // подсчёт всех гамильтоновых циклов из вершины 1
{
    int i = (int)path.size(); // текущая длина пути
    if (i == n) // все вершины в пути
    {
        return (adj[path[n - 1]][path[0]] && path[1] < path[n - 1]) ? 1 : 0; // каждый цикл учитываем один раз
    }
    int cnt = 0; // число найденных циклов
    for (int u : L[path[i - 1]]) // перебираем соседей последней вершины
    {
        if (inPath[u])
        {
            continue; // вершина уже в пути
        }
        path.push_back(u); inPath[u] = true; // добавляем вершину
        cnt += countCycles(path, inPath); // считаем циклы с таким продолжением
        path.pop_back(); inPath[u] = false; // возврат
    }
    return cnt; // возвращаем количество
}

bool findCycle(vector<int>& path) // поиск гамильтонова цикла из вершины 1
{
    path.assign(1, 1); // x1 = 1
    vector<bool> inPath(n + 1, false); // отметки вершин пути
    inPath[1] = true;
    return hamilton(path, inPath, true); // запускаем перебор
}

bool findPath(vector<int>& path) // поиск гамильтонова пути (перебор стартовых вершин)
{
    for (int s = 1; s <= n; s++) // пробуем каждую стартовую вершину
    {
        path.assign(1, s); // x1 = s
        vector<bool> inPath(n + 1, false);
        inPath[s] = true;
        if (hamilton(path, inPath, false))
        {
            return true; // путь найден
        }
    }
    return false; // гамильтонова пути нет
}

void printPath(const vector<int>& path, bool cycle) // вывод пути или цикла
{
    for (int i = 0; i < (int)path.size(); i++) // перебираем вершины
    {
        cout << "v" << path[i]; // выводим вершину
        if (i + 1 < (int)path.size())
        {
            cout << " - "; // разделитель
        }
    }
    if (cycle)
    {
        cout << " - v" << path[0]; // замыкаем цикл
    }
    cout << endl;
}

void printMatrix() // вывод матрицы смежности
{
    cout << "    ";
    for (int v = 1; v <= n; v++)
    {
        cout << setw(3) << v; // номера столбцов
    }
    cout << endl;
    for (int u = 1; u <= n; u++) // строки матрицы
    {
        cout << setw(3) << u << " ";
        for (int v = 1; v <= n; v++)
        {
            cout << setw(3) << adj[u][v];
        }
        cout << endl;
    }
}

void printInfo() // вывод списков смежности, степеней и достаточных условий
{
    cout << "Списки смежности и степени вершин:" << endl;
    for (int v = 1; v <= n; v++) // для каждой вершины
    {
        cout << "  L(v" << v << ") = {";
        for (int k = 0; k < (int)L[v].size(); k++) // выводим соседей
        {
            cout << L[v][k] << (k + 1 < (int)L[v].size() ? ", " : "");
        }
        cout << "}, d(v" << v << ") = " << degree(v) << endl;
    }
    cout << "Условие Дирака (d(v) >= n/2 = " << n / 2.0 << "): "
        << (checkDirac() ? "выполняется" : "не выполняется") << endl;
    cout << "Условие Оре (d(u) + d(v) >= n для несмежных u, v): "
        << (checkOre() ? "выполняется" : "не выполняется") << endl;
}

int main() // главная функция
{
    setlocale(LC_ALL, "rus");

    int m; // количество рёбер
    int choice; // способ задания графа
    cout << "Использовать граф варианта 4 (1 - да, 0 - ввести свой граф): ";
    cin >> choice;

    switch (choice) // выбираем способ задания графа
    {
    case 1: // граф варианта 4
    {
        n = 8; // 8 вершин
        adj.assign(n + 1, vector<int>(n + 1, 0)); // создаём пустую матрицу смежности
        int edges[12][2] = { { 1,2 },{ 1,5 },{ 2,4 },{ 2,7 },{ 2,8 },{ 3,4 },
                             { 3,7 },{ 4,5 },{ 4,6 },{ 6,7 },{ 6,8 },{ 7,8 } }; // рёбра варианта 4
        m = 12; // количество рёбер
        for (int i = 0; i < m; i++) // перебираем все рёбра
        {
            addEdge(edges[i][0], edges[i][1]); // добавляем ребро
        }
        break; // выходим из switch
    }
    case 0: // ввод графа с клавиатуры
    {
        cout << "Введите количество вершин и рёбер: ";
        cin >> n >> m;
        adj.assign(n + 1, vector<int>(n + 1, 0)); // создаём пустую матрицу смежности
        cout << "Введите рёбра (пары вершин от 1 до " << n << "):" << endl;
        for (int i = 0; i < m; i++) // читаем m рёбер
        {
            int u, v;
            cin >> u >> v;
            addEdge(u, v); // добавляем ребро
        }
        break; // выходим из switch
    }
    default: // введено что-то кроме 1 и 0
        cout << "Неверный выбор. Введите 1 или 0." << endl;
        system("PAUSE"); // пауза перед закрытием окна консоли
        return 0; // завершаем программу
    }

    buildLists(); // составляем списки смежности
    cout << "\nМатрица смежности графа:" << endl;
    printMatrix(); // выводим матрицу
    printInfo(); // выводим степени и проверяем достаточные условия

    vector<int> path; // найденный путь или цикл
    if (isConnected() && findCycle(path)) // ищем гамильтонов цикл перебором
    {
        cout << "\nГраф является гамильтоновым." << endl;
        cout << "Гамильтонов цикл: ";
        printPath(path, true); // выводим цикл
        vector<int> p(1, 1); vector<bool> used(n + 1, false); used[1] = true;
        cout << "Всего различных гамильтоновых циклов: " << countCycles(p, used) << endl;
        cout << "Гамильтонова цепь (цикл без последнего ребра): ";
        printPath(path, false); // цепь получается удалением одного ребра цикла
    }
    else
    {
        cout << "\nГамильтонова цикла нет - граф не является гамильтоновым." << endl;
        if (findPath(path)) // проверяем, является ли граф полугамильтоновым
        {
            cout << "Граф полугамильтонов. Гамильтонова цепь: ";
            printPath(path, false);
        }
        else
            cout << "Гамильтоновой цепи тоже нет." << endl;

        // Изменение графа: к вершинам v1..vp добавляем вершины u1..up и рёбра (vi, ui), (ui, vi+1)
        int p = n; // исходное число вершин
        n = 2 * p; // новых вершин столько же
        adj.assign(n + 1, vector<int>(n + 1, 0)); // заново строим матрицу (старые рёбра сохраняем ниже)
        cout << "\nИзменение графа: добавлены вершины v" << p + 1 << " - v" << n
            << " (u1 - u" << p << ") и рёбра (vi, ui), (ui, vi+1)" << endl;
        for (int v = 1; v <= p; v++) // восстанавливаем исходные рёбра
        {
            for (int u : L[v])
            {
                addEdge(v, u);
            }
        }
        for (int i = 1; i <= p; i++) // добавляем новые рёбра
        {
            int ui = p + i; // номер вершины ui
            addEdge(i, ui); // ребро (vi, ui)
            addEdge(ui, i % p + 1); // ребро (ui, vi+1), для vp следующей будет v1
        }
        buildLists(); // пересоставляем списки смежности
        printInfo(); // выводим новые степени
        if (findCycle(path)) // ищем цикл в изменённом графе
        {
            cout << "\nИзменённый граф является гамильтоновым." << endl;
            cout << "Гамильтонов цикл: ";
            printPath(path, true);
        }
    }


system("PAUSE");
return 0;

}