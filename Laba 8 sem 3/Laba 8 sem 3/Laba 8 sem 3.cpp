// 8 лаба

#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <fstream>
#include <limits>
#include <cstdlib>
#ifdef _WIN32
#define NOMINMAX // иначе windows.h объявляет макрос max, и numeric_limits<streamsize>::max() не компилируется
#include <windows.h>
#endif

using namespace std;

const int INF{ 1000000 }; // «бесконечность»: билета между городами нет

// ---------- Вспомогательные функции ----------

// Выводит приглашение prompt и читает целое число.
// Если введено не число, сообщает об ошибке, очищает поток и спрашивает снова.
int readInt(string prompt)
{
    int x;
    while (true)
    {
        cout << prompt;
        if (cin >> x)
        {
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // убираем остаток строки
            return x;
        }
        cout << "  Ошибка: нужно ввести целое число." << endl;
        cin.clear(); // сбрасываем флаг ошибки потока
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // удаляем неверный ввод
    }
}

// Возвращает путь в виде строки «1 -> 2 -> 8».
string pathToString(const vector<int>& path)
{
    string s;
    for (int k = 0; k < static_cast<int>(path.size()); k++)
    {
        if (k > 0)
        {
            s += " -> ";
        }
        s += to_string(path[k]);
    }
    return s;
}

// ---------- Класс «Граф городов» ----------

class Graph
{
public:
    // Конструктор: граф из p_n городов без рёбер (вершины нумеруются с 1).
    Graph(int p_n) : n{ p_n }, weight(p_n + 1, vector<int>(p_n + 1, INF))
    {
        for (int i = 1; i <= n; i++)
        {
            weight[i][i] = 0;
        }
    }

    // Добавляет ребро a - b стоимостью p_cost (билет продаётся в обе стороны).
    void addEdge(int a, int b, int p_cost)
    {
        weight[a][b] = p_cost;
        weight[b][a] = p_cost;
    }

    int getN() const
    {
        return n;
    }
    // Возвращает стоимость прямого билета из a в b (INF - билета нет).
    int getWeight(int a, int b) const
    {
        return weight[a][b];
    }

    // Выводит матрицу стоимостей билетов; «-» - прямого билета нет.
    void print() const
    {
        cout << "     ";
        for (int j = 1; j <= n; j++)
        {
            cout << setw(4) << j;
        }
        cout << endl;
        for (int i = 1; i <= n; i++)
        {
            cout << setw(4) << i << " ";
            for (int j = 1; j <= n; j++)
            {
                if (weight[i][j] == INF)
                {
                    cout << setw(4) << "-";
                }
                else
                {
                    cout << setw(4) << weight[i][j];
                }
            }
            cout << endl;
        }
    }

private:
    int n;                      // число городов
    vector<vector<int>> weight; // матрица стоимостей билетов
};

// ---------- Алгоритм Дейкстры ----------

class Dijkstra
{
public:
    // Конструктор: сразу находит самые дешёвые маршруты из города p_start во все города.
    Dijkstra(const Graph& p_graph, int p_start) : graph{ p_graph }, start{ p_start }
    {
        run();
    }

    // Возвращает стоимость самого дешёвого маршрута из start в v (INF - маршрута нет).
    int getDist(int v) const
    {
        return dist[v];
    }

    // Восстанавливает маршрут из start в v по массиву предшественников prev.
    vector<int> getPath(int v) const
    {
        vector<int> path;
        if (dist[v] == INF)
        {
            return path;
        }
        for (int u = v; u != 0; u = prev[u])
        {
            path.insert(path.begin(), u); // добавляем город в начало маршрута
        }
        return path;
    }

    // Выводит стоимости и маршруты из start во все остальные города.
    void print() const
    {
        for (int v = 1; v <= graph.getN(); v++)
        {
            if (v == start)
            {
                continue;
            }
            cout << "  " << start << " -> " << v << ": ";
            if (dist[v] == INF)
            {
                cout << "маршрута нет" << endl;
            }
            else
            {
                cout << setw(3) << dist[v] << " руб.   " << pathToString(getPath(v)) << endl;
            }
        }
    }

private:
    // Алгоритм Дейкстры (шаги 1-7 из методических указаний).
    void run()
    {
        int n = graph.getN();
        dist.assign(n + 1, INF);        // шаг 1: веса всех вершин - бесконечность,
        dist[start] = 0;                //         вес первой вершины - 0
        prev.assign(n + 1, 0);
        selected.assign(n + 1, false);  // шаг 2: все вершины не выделены
        int cur = start;                // шаг 3: первая вершина - текущая
        selected[cur] = true;
        while (cur != 0)                // cur = 0 означает: следующей вершины нет
        {
            // Шаг 4: пересчёт веса невыделенных вершин через текущую.
            for (int v = 1; v <= n; v++)
            {
                int w = graph.getWeight(cur, v);
                if (!selected[v] && w != INF && dist[cur] + w < dist[v])
                {
                    dist[v] = dist[cur] + w;
                    prev[v] = cur; // запоминаем, откуда пришли в v
                }
            }
            // Шаг 5: среди невыделенных вершин ищем вершину с минимальным конечным весом.
            int next = 0;
            for (int v = 1; v <= n; v++)
            {
                if (!selected[v] && dist[v] != INF && (next == 0 || dist[v] < dist[next]))
                {
                    next = v;
                }
            }
            cur = next;           // если next = 0, все оставшиеся вершины недостижимы - конец
            if (cur != 0)
            {
                selected[cur] = true; // шаг 5: вершина выделена; шаг 6 не нужен (ищем пути во все города); шаг 7 - к шагу 4
            }
        }
    }

    const Graph& graph;    // граф городов
    int start;             // город, из которого строятся маршруты
    vector<int> dist;      // вес вершины: стоимость маршрута из start
    vector<int> prev;      // предыдущий город на самом дешёвом маршруте (0 - нет)
    vector<bool> selected; // вершина выделена (стоимость окончательная)
};

// ---------- Алгоритм Флойда ----------

class Floyd
{
public:
    // Конструктор: сразу строит матрицы A (стоимости) и S (последовательность вершин).
    Floyd(const Graph& p_graph) : n{ p_graph.getN() }
    {
        a.assign(n + 1, vector<int>(n + 1, INF));
        s.assign(n + 1, vector<int>(n + 1, 0));
        // Шаг 0: A0 - матрица стоимостей билетов, S0[i][j] = j, на диагонали нули.
        for (int i = 1; i <= n; i++)
        {
            for (int j = 1; j <= n; j++)
            {
                a[i][j] = p_graph.getWeight(i, j);
                s[i][j] = (i == j) ? 0 : j;
            }
        }
        // Основной шаг k: строка и столбец k ведущие.
        for (int k = 1; k <= n; k++)
        {
            for (int i = 1; i <= n; i++)
            {
                for (int j = 1; j <= n; j++)
                {
                    if (i != k && j != k && i != j && a[i][k] != INF && a[k][j] != INF
                        && a[i][k] + a[k][j] < a[i][j])
                    {
                        a[i][j] = a[i][k] + a[k][j]; // путь i -> k -> j дешевле
                        s[i][j] = k;
                    }
                }
            }
        }
    }

    // Возвращает стоимость самого дешёвого маршрута из i в j.
    int getDist(int i, int j) const
    {
        return a[i][j];
    }

    // Восстанавливает маршрут из i в j по матрице S.
    vector<int> getPath(int i, int j) const
    {
        vector<int> path;
        if (a[i][j] == INF)
        {
            return path;
        }
        path.push_back(i);
        if (i != j)
        {
            addPath(i, j, path);
        }
        return path;
    }

    // Выводит итоговые матрицы A и S.
    void print() const
    {
        cout << "Матрица A (стоимость самого дешёвого маршрута):" << endl;
        printMatrix(a);
        cout << "Матрица S (через какой город ехать из i в j):" << endl;
        printMatrix(s);
    }

private:
    // Дописывает в path города маршрута i -> j без города i (рекурсивно).
    void addPath(int i, int j, vector<int>& path) const
    {
        int k = s[i][j];
        if (k == j)
        {
            path.push_back(j); // прямой билет
            return;
        }
        addPath(i, k, path); // сначала маршрут i -> k,
        addPath(k, j, path); // затем маршрут k -> j
    }

    // Выводит матрицу m с номерами строк и столбцов.
    void printMatrix(const vector<vector<int>>& m) const
    {
        cout << "     ";
        for (int j = 1; j <= n; j++)
        {
            cout << setw(4) << j;
        }
        cout << endl;
        for (int i = 1; i <= n; i++)
        {
            cout << setw(4) << i << " ";
            for (int j = 1; j <= n; j++)
            {
                if (m[i][j] == INF)
                {
                    cout << setw(4) << "-";
                }
                else
                {
                    cout << setw(4) << m[i][j];
                }
            }
            cout << endl;
        }
    }

    int n;                 // число городов
    vector<vector<int>> a; // матрица кратчайших стоимостей A
    vector<vector<int>> s; // матрица последовательности вершин S
};

// ---------- Поездка Оли: перебор всех порядков посещения друзей ----------

// Один вариант поездки: порядок городов и его стоимость.
struct Route
{
    vector<int> order; // 1, друзья в порядке посещения, 1
    int cost;          // стоимость поездки, руб.
};

class TripPlanner
{
public:
    // Конструктор: запоминает граф и матрицу Флойда, сразу перебирает все маршруты.
    TripPlanner(const Graph& p_graph, const Floyd& p_floyd, int p_home)
        : graph{ p_graph }, floyd{ p_floyd }, home{ p_home }, bestCost{ INF }, impossible{ 0 }
    {
        used.assign(graph.getN() + 1, false);
        used[home] = true;
        order.push_back(home);
        search(1);
    }

    int getBestCost() const
    {
        return bestCost;
    }
    int getCount() const
    {
        return static_cast<int>(routes.size());
    }

    // Выводит все возможные маршруты в поток out; оптимальные отмечены.
    void printAll(ostream& out) const
    {
        for (int r = 0; r < static_cast<int>(routes.size()); r++)
        {
            out << setw(5) << r + 1 << ".  " << pathToString(routes[r].order) << "   " << setw(4) << routes[r].cost << " руб.";
            if (routes[r].cost == bestCost)
            {
                out << "   <-- ОПТИМАЛЬНЫЙ";
            }
            out << endl;
        }
        if (impossible > 0)
        {
            out << "Невозможных порядков (нет маршрута между городами): " << impossible << endl;
        }
    }

    // Выводит оптимальные маршруты с пересадками и стоимостью каждого переезда.
    void printBest() const
    {
        if (bestCost == INF)
        {
            cout << "Объехать всех друзей и вернуться домой невозможно." << endl;
            return;
        }
        int number = 0;
        for (int r = 0; r < static_cast<int>(routes.size()); r++)
        {
            if (routes[r].cost != bestCost)
            {
                continue;
            }
            number++;
            cout << "Оптимальный маршрут " << number << " (вариант № " << r + 1 << "): "
                << pathToString(routes[r].order) << endl;
            vector<int> full; // маршрут со всеми пересадками
            full.push_back(home);
            for (int k = 1; k < static_cast<int>(routes[r].order.size()); k++)
            {
                int from = routes[r].order[k - 1];
                int to = routes[r].order[k];
                vector<int> leg = floyd.getPath(from, to);
                cout << "    " << from << " -> " << to << ": " << setw(3) << floyd.getDist(from, to)
                    << " руб.  (" << pathToString(leg) << ")" << endl;
                for (int t = 1; t < static_cast<int>(leg.size()); t++)
                {
                    full.push_back(leg[t]);
                }
            }
            cout << "    Поездка с пересадками: " << pathToString(full) << endl;
            cout << "    Итого: " << bestCost << " руб." << endl;
        }
    }

private:
    // Рекурсивный перебор: на место level ставится каждый ещё не посещённый друг.
    void search(int level)
    {
        int n = graph.getN();
        if (level == n) // все друзья расставлены - возвращаемся домой
        {
            order.push_back(home);
            int cost = 0;
            for (int k = 1; k < static_cast<int>(order.size()) && cost != INF; k++)
            {
                int d = floyd.getDist(order[k - 1], order[k]);
                cost = (d == INF) ? INF : cost + d;
            }
            if (cost == INF)
            {
                impossible++;
            }
            else
            {
                routes.push_back({ order, cost });
                if (cost < bestCost)
                {
                    bestCost = cost;
                }
            }
            order.pop_back();
            return;
        }
        for (int v = 1; v <= n; v++)
        {
            if (used[v])
            {
                continue;
            }
            used[v] = true;
            order.push_back(v);
            search(level + 1);
            order.pop_back(); // возврат: убираем друга, пробуем следующего
            used[v] = false;
        }
    }

    const Graph& graph;  // граф городов
    const Floyd& floyd;  // кратчайшие стоимости между всеми городами
    int home;            // город, где живёт Оля
    int bestCost;        // наименьшая стоимость поездки
    int impossible;      // число порядков, которые нельзя проехать
    vector<int> order;   // текущий порядок городов
    vector<bool> used;   // город уже есть в текущем порядке
    vector<Route> routes; // все возможные маршруты
};

// ---------- Клиентская программа ----------

// Создаёт граф варианта 4 (8 городов, 12 рёбер).
Graph makeVariantGraph()
{
    Graph g(8);
    g.addEdge(1, 2, 7);
    g.addEdge(1, 5, 6);
    g.addEdge(2, 4, 2);
    g.addEdge(2, 7, 4);
    g.addEdge(2, 8, 2);
    g.addEdge(3, 4, 3);
    g.addEdge(3, 7, 3);
    g.addEdge(4, 5, 5);
    g.addEdge(4, 6, 1);
    g.addEdge(6, 7, 4);
    g.addEdge(6, 8, 2);
    g.addEdge(7, 8, 5);
    return g;
}

// Сравнивает результаты Дейкстры (из каждого города) и Флойда для всех пар городов.
void checkAlgorithms(const Graph& graph, const Floyd& floyd)
{
    int n = graph.getN();
    int errors = 0;
    for (int i = 1; i <= n; i++)
    {
        Dijkstra dijkstra(graph, i);
        for (int j = 1; j <= n; j++)
        {
            if (dijkstra.getDist(j) != floyd.getDist(i, j))
            {
                errors++;
                cout << "  Расхождение для пары " << i << " - " << j << ": Дейкстра " << dijkstra.getDist(j)
                    << ", Флойд " << floyd.getDist(i, j) << endl;
            }
        }
    }
    cout << "Проверено пар городов: " << n * n << ", расхождений: " << errors << endl;
    if (errors == 0)
    {
        cout << "Проверка пройдена: алгоритмы Дейкстры и Флойда дали одинаковые стоимости." << endl;
    }
}

// Главная функция: построение графа варианта 4 и меню.
int main()
{
    setlocale(LC_ALL, "rus");
#ifdef _WIN32
    SetConsoleCP(1251);       // ввод русских букв с клавиатуры
    SetConsoleOutputCP(1251); // вывод русских букв
#endif

    const int HOME{ 1 }; // Оля живёт в городе 1
    cout << "Поездка Оли к друзьям (граф варианта 4)" << endl;
    Graph graph = makeVariantGraph();          // граф городов варианта 4
    Floyd floyd(graph);                        // кратчайшие стоимости между всеми городами
    TripPlanner planner(graph, floyd, HOME);   // перебор всех маршрутов поездки

    int choice; // пункт меню
    do
    {
        cout << "\n===== Поездка Оли (городов: " << graph.getN() << ", Оля живёт в городе " << HOME << ") =====" << endl;
        cout << "1 - матрица стоимостей билетов" << endl;
        cout << "2 - алгоритм Дейкстры: самые дешёвые маршруты из города 1" << endl;
        cout << "3 - алгоритм Флойда: самые дешёвые маршруты между всеми городами" << endl;
        cout << "4 - все маршруты поездки к друзьям и оптимальный" << endl;
        cout << "5 - оптимальный маршрут поездки с пересадками" << endl;
        cout << "6 - проверка: сравнение алгоритмов Дейкстры и Флойда" << endl;
        cout << "0 - выход" << endl;
        choice = readInt("Ваш выбор: ");

        switch (choice)
        {
        case 1:
            cout << "Стоимость билетов (- : прямого билета нет):" << endl;
            graph.print();
            break;
        case 2:
        {
            Dijkstra dijkstra(graph, HOME);
            cout << "Самые дешёвые маршруты из города " << HOME << " (алгоритм Дейкстры):" << endl;
            dijkstra.print();
            break;
        }
        case 3:
        {
            floyd.print();
            cout << "Маршруты из города " << HOME << " по матрице S:" << endl;
            for (int v = 1; v <= graph.getN(); v++)
            {
                if (v != HOME)
                {
                    cout << "  " << HOME << " -> " << v << ": ";
                    if (floyd.getDist(HOME, v) == INF)
                    {
                        cout << "маршрута нет" << endl;
                    }
                    else
                    {
                        cout << setw(3) << floyd.getDist(HOME, v) << " руб.   " << pathToString(floyd.getPath(HOME, v)) << endl;
                    }
                }
            }
            break;
        }
        case 4:
        {
            planner.printAll(cout);
            cout << "Всего маршрутов: " << planner.getCount() << endl;
            if (planner.getBestCost() == INF)
            {
                cout << "Объехать всех друзей невозможно." << endl;
            }
            else
            {
                cout << "Наименьшая стоимость поездки: " << planner.getBestCost() << " руб." << endl;
            }
            ofstream file("routes.txt");
            if (file)
            {
                planner.printAll(file);
                cout << "Список маршрутов также сохранён в файл routes.txt." << endl;
            }
            break;
        }
        case 5:
            planner.printBest();
            break;
        case 6:
            checkAlgorithms(graph, floyd);
            break;
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