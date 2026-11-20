// 8 лаба
// Поездка Оли к друзьям (граф варианта 4: 8 городов, 12 рёбер).
// Оля живёт в городе 1, друзья - в городах 2-8; известна стоимость билетов между городами.
// Программа:
//   - находит самые дешёвые маршруты между городами алгоритмами Дейкстры и Флойда;
//   - перебирает все порядки посещения друзей (каждый друг один раз, затем возвращение домой),
//     выводит все варианты поездки и выделяет оптимальный (самый дешёвый);
//   - проверяет себя: результаты Дейкстры и Флойда для всех пар городов должны совпасть.

#include <iostream> // cin, cout - консольный ввод и вывод
#include <string>   // string, to_string - строки
#include <vector>   // vector - динамический массив (матрицы, маршруты)
#include <iomanip>  // setw - ширина поля при выводе таблиц
#include <fstream>  // ofstream - запись файла routes.txt
#include <limits>   // numeric_limits - очистка потока ввода
#include <cstdlib>  // system
#ifdef _WIN32
#define NOMINMAX // иначе windows.h объявляет макрос max, и numeric_limits<streamsize>::max() не компилируется
#include <windows.h> // SetConsoleCP, SetConsoleOutputCP - русские буквы в консоли
#endif

using namespace std;

// «Бесконечность»: билета между городами нет. Число намного больше любой стоимости поездки,
// но сумма двух таких чисел ещё помещается в int (переполнения нет).
const int INF{ 1000000 }; // «бесконечность»: билета между городами нет

// ---------- Вспомогательные функции ----------

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

// Возвращает путь в виде строки «1 -> 2 -> 8».
string pathToString(const vector<int>& path)
{
    string s;
    for (int k = 0; k < static_cast<int>(path.size()); k++)
    {
        if (k > 0) // перед всеми городами, кроме первого, ставим стрелку
        {
            s += " -> ";
        }
        s += to_string(path[k]); // номер города - в строку
    }
    return s;
}

// ---------- Класс «Граф городов» ----------

// Неориентированный взвешенный граф: вершины - города, рёбра - прямые билеты,
// вес ребра - стоимость билета. Хранится матрицей стоимостей weight.
// Вершины нумеруются с 1, как на рисунке варианта, поэтому строка и столбец 0 не используются.
class Graph
{
public:
    // Конструктор: граф из p_n городов без рёбер (вершины нумеруются с 1).
    // Все элементы матрицы - INF (билетов нет), на диагонали - 0 (из города в него самого ехать бесплатно).
    Graph(int p_n) : n{ p_n }, weight(p_n + 1, vector<int>(p_n + 1, INF))
    {
        for (int i = 1; i <= n; i++)
        {
            weight[i][i] = 0;
        }
    }

    // Добавляет ребро a - b стоимостью p_cost (билет продаётся в обе стороны).
    // Поэтому матрица симметрична: weight[a][b] = weight[b][a].
    void addEdge(int a, int b, int p_cost)
    {
        weight[a][b] = p_cost;
        weight[b][a] = p_cost;
    }

    // Возвращает число городов.
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
        // Строка заголовка: номера столбцов (городов назначения).
        cout << "     ";
        for (int j = 1; j <= n; j++)
        {
            cout << setw(4) << j;
        }
        cout << endl;
        // Строки матрицы: номер строки (город отправления), затем стоимости.
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

// Находит самые дешёвые маршруты из одного города (start) во все остальные.
// Каждой вершине приписан вес - стоимость лучшего известного маршрута из start.
// Выделенная вершина - вершина, вес которой уже окончательный (дешевле добраться нельзя).
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
    // Идём от v назад по prev до start (у start предшественника нет: prev[start] = 0),
    // каждый город добавляем в начало, поэтому маршрут получается в прямом порядке.
    vector<int> getPath(int v) const
    {
        vector<int> path; // если маршрута нет, путь останется пустым
        if (dist[v] != INF)
        {
            for (int u = v; u != 0; u = prev[u])
            {
                path.insert(path.begin(), u); // добавляем город в начало маршрута
            }
        }
        return path;
    }

    // Выводит стоимости и маршруты из start во все остальные города.
    void print() const
    {
        for (int v = 1; v <= graph.getN(); v++)
        {
            if (v != start) // маршрут из города в него самого не выводим
            {
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
    }

private:
    // Алгоритм Дейкстры (шаги 1-7 из методических указаний).
    void run()
    {
        int n = graph.getN();
        dist.assign(n + 1, INF);        // шаг 1: веса всех вершин - бесконечность,
        dist[start] = 0;                //         вес первой вершины - 0
        prev.assign(n + 1, 0);          // предшественников пока нет
        selected.assign(n + 1, false);  // шаг 2: все вершины не выделены
        int cur = start;                // шаг 3: первая вершина - текущая
        selected[cur] = true;
        while (cur != 0)                // cur = 0 означает: следующей вершины нет
        {
            // Шаг 4: пересчёт веса невыделенных вершин через текущую.
            // Если ехать в v через cur дешевле, чем известный маршрут в v, - запоминаем новый маршрут.
            for (int v = 1; v <= n; v++)
            {
                int w = graph.getWeight(cur, v); // стоимость билета cur -> v
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

// Находит самые дешёвые маршруты между всеми парами городов.
// Матрица A: A[i][j] - стоимость самого дешёвого маршрута из i в j.
// Матрица S: S[i][j] - город, через который нужно ехать из i в j
// (S[i][j] = j - ехать прямым билетом).
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
        // Для каждой пары (i, j) проверяем, не дешевле ли ехать через город k: i -> k -> j.
        // После шага k матрица A содержит самые дешёвые маршруты, проходящие через города 1..k.
        for (int k = 1; k <= n; k++)
        {
            for (int i = 1; i <= n; i++)
            {
                for (int j = 1; j <= n; j++)
                {
                    // i, j, k должны быть разными; маршруты i -> k и k -> j должны существовать.
                    if (i != k && j != k && i != j && a[i][k] != INF && a[k][j] != INF
                        && a[i][k] + a[k][j] < a[i][j])
                    {
                        a[i][j] = a[i][k] + a[k][j]; // путь i -> k -> j дешевле
                        s[i][j] = k;                 // ехать из i в j нужно через k
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
        vector<int> path; // если маршрута нет, путь останется пустым
        if (a[i][j] != INF)
        {
            path.push_back(i);       // маршрут начинается в i
            if (i != j)
            {
                addPath(i, j, path); // дописываем остальные города до j
            }
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
    // Если S[i][j] = k и k != j, то маршрут i -> j - это маршрут i -> k, а за ним маршрут k -> j;
    // каждую часть восстанавливаем тем же способом.
    void addPath(int i, int j, vector<int>& path) const
    {
        int k = s[i][j];
        if (k == j)
        {
            path.push_back(j); // прямой билет
        }
        else
        {
            addPath(i, k, path); // сначала маршрут i -> k,
            addPath(k, j, path); // затем маршрут k -> j
        }
    }

    // Выводит матрицу m с номерами строк и столбцов.
    void printMatrix(const vector<vector<int>>& m) const
    {
        // Строка заголовка: номера столбцов.
        cout << "     ";
        for (int j = 1; j <= n; j++)
        {
            cout << setw(4) << j;
        }
        cout << endl;
        // Строки матрицы; «-» - маршрута нет.
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

// Перебирает все порядки посещения друзей (все перестановки городов 2..n).
// Между соседними городами порядка Оля едет самым дешёвым маршрутом,
// поэтому стоимость переезда берётся из матрицы A алгоритма Флойда.
class TripPlanner
{
public:
    // Конструктор: запоминает граф и матрицу Флойда, сразу перебирает все маршруты.
    TripPlanner(const Graph& p_graph, const Floyd& p_floyd, int p_home)
        : graph{ p_graph }, floyd{ p_floyd }, home{ p_home }, bestCost{ INF }, impossible{ 0 }
    {
        used.assign(graph.getN() + 1, false);
        used[home] = true;     // домашний город в перестановку друзей не входит
        order.push_back(home); // поездка начинается из дома
        search(1);             // расставляем друзей на места 1, 2, ..., n - 1
    }

    // Возвращает наименьшую стоимость поездки (INF - объехать всех нельзя).
    int getBestCost() const
    {
        return bestCost;
    }
    // Возвращает число возможных вариантов поездки.
    int getCount() const
    {
        return static_cast<int>(routes.size());
    }

    // Выводит все возможные маршруты в поток out; оптимальные отмечены.
    // out - любой поток вывода: cout (консоль) или ofstream (файл routes.txt).
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
        }
        else
        {
            int number = 0; // номер оптимального маршрута (их может быть несколько с одной стоимостью)
            for (int r = 0; r < static_cast<int>(routes.size()); r++)
            {
                if (routes[r].cost == bestCost) // выводим только оптимальные маршруты
                {
                    number++;
                    cout << "Оптимальный маршрут " << number << " (вариант № " << r + 1 << "): "
                        << pathToString(routes[r].order) << endl;
                    vector<int> full; // маршрут со всеми пересадками
                    full.push_back(home);
                    // Каждый переезд между соседними городами порядка раскрываем
                    // в маршрут с пересадками по матрице S алгоритма Флойда.
                    for (int k = 1; k < static_cast<int>(routes[r].order.size()); k++)
                    {
                        int from = routes[r].order[k - 1];
                        int to = routes[r].order[k];
                        vector<int> leg = floyd.getPath(from, to); // переезд from -> to с пересадками
                        cout << "    " << from << " -> " << to << ": " << setw(3) << floyd.getDist(from, to)
                            << " руб.  (" << pathToString(leg) << ")" << endl;
                        // Первый город переезда (from) уже есть в full - добавляем остальные.
                        for (int t = 1; t < static_cast<int>(leg.size()); t++)
                        {
                            full.push_back(leg[t]);
                        }
                    }
                    cout << "    Поездка с пересадками: " << pathToString(full) << endl;
                    cout << "    Итого: " << bestCost << " руб." << endl;
                }
            }
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
            // Стоимость поездки - сумма стоимостей переездов между соседними городами порядка.
            // Если какой-то переезд невозможен (INF), считать дальше не нужно.
            int cost = 0;
            for (int k = 1; k < static_cast<int>(order.size()) && cost != INF; k++)
            {
                int d = floyd.getDist(order[k - 1], order[k]);
                cost = (d == INF) ? INF : cost + d;
            }
            if (cost == INF)
            {
                impossible++; // такой порядок проехать нельзя
            }
            else
            {
                routes.push_back({ order, cost }); // запоминаем вариант поездки
                if (cost < bestCost)
                {
                    bestCost = cost;               // найдена более дешёвая поездка
                }
            }
            order.pop_back(); // убираем дом из конца порядка - перебор продолжится
        }
        else // ставим на место level по очереди каждого ещё не поставленного друга
        {
            for (int v = 1; v <= n; v++)
            {
                if (!used[v]) // города v ещё нет в маршруте
                {
                    used[v] = true;
                    order.push_back(v);
                    search(level + 1); // перебор всех продолжений с городом v на месте level
                    order.pop_back(); // возврат: убираем друга, пробуем следующего
                    used[v] = false;
                }
            }
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
// Каждый вызов addEdge(a, b, c) - билет между городами a и b стоимостью c рублей.
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
// Алгоритмы работают по-разному, но ищут одно и то же - самые дешёвые маршруты,
// поэтому стоимости должны совпасть; расхождение означает ошибку в программе.
void checkAlgorithms(const Graph& graph, const Floyd& floyd)
{
    int n = graph.getN();
    int errors = 0; // число расхождений
    for (int i = 1; i <= n; i++)
    {
        Dijkstra dijkstra(graph, i); // маршруты из города i
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

    // Все вычисления выполняются один раз при запуске; пункты меню только выводят результаты.
    const int HOME{ 1 }; // Оля живёт в городе 1
    cout << "Поездка Оли к друзьям (граф варианта 4)" << endl;
    Graph graph = makeVariantGraph();          // граф городов варианта 4
    Floyd floyd(graph);                        // кратчайшие стоимости между всеми городами
    TripPlanner planner(graph, floyd, HOME);   // перебор всех маршрутов поездки

    // Меню повторяется, пока не выбран пункт 0 (условие проверяется в конце цикла).
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
        case 1: // матрица стоимостей билетов
            cout << "Стоимость билетов (- : прямого билета нет):" << endl;
            graph.print();
            break;
        case 2: // алгоритм Дейкстры из дома Оли
        {
            Dijkstra dijkstra(graph, HOME);
            cout << "Самые дешёвые маршруты из города " << HOME << " (алгоритм Дейкстры):" << endl;
            dijkstra.print();
            break;
        }
        case 3: // алгоритм Флойда: матрицы A и S и маршруты из дома Оли
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
        case 4: // все варианты поездки и наименьшая стоимость
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
            // Список длинный, поэтому дополнительно сохраняем его в файл.
            ofstream file("routes.txt");
            if (file)
            {
                planner.printAll(file);
                cout << "Список маршрутов также сохранён в файл routes.txt." << endl;
            }
            break;
        }
        case 5: // оптимальные маршруты с пересадками
            planner.printBest();
            break;
        case 6: // проверка: Дейкстра и Флойд должны дать одинаковые стоимости
            checkAlgorithms(graph, floyd);
            break;
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
