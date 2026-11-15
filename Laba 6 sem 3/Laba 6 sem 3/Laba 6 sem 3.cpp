#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <limits>
#include <cstdlib>
#ifdef _WIN32
#define NOMINMAX // иначе windows.h объявляет макрос max, и numeric_limits<streamsize>::max() не компилируется
#include <windows.h>
#endif

using namespace std;

const int CURRENT_YEAR{ 2026 }; // текущий год для проверки года издания

// ---------- Вспомогательные функции ввода ----------

// Выводит приглашение prompt и читает целое число.
// Если введено не число, сообщает об ошибке, очищает поток и спрашивает снова.
// Возвращает введённое число.
int readInt(string prompt)
{
    int x;
    while (true)
    {
        cout << prompt;
        if (cin >> x)
        {
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // убираем остаток строки
            return x; // число введено верно
        }
        cout << "  Ошибка: нужно ввести целое число." << endl;
        cin.clear(); // сбрасываем флаг ошибки потока
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // удаляем неверный ввод
    }
}

// То же, что readInt, но для вещественного числа (например, цены).
double readDouble(string prompt)
{
    double x;
    while (true)
    {
        cout << prompt;
        if (cin >> x)
        {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return x;
        }
        cout << "  Ошибка: нужно ввести число." << endl;
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

// Выводит приглашение prompt и читает целую строку (в ней могут быть пробелы).
string readLine(string prompt)
{
    string s;
    cout << prompt;
    getline(cin, s);
    return s;
}

// ---------- Интерфейсы: абстрактные классы без полей, с чистыми виртуальными функциями ----------

class IItem // общий интерфейс издания; наследуется ВИРТУАЛЬНО - виртуальный базовый класс
{
public:
    // Виртуальный деструктор: при delete через указатель на интерфейс
    // вызывается деструктор того класса, которому принадлежит объект.
    virtual ~IItem() = default;
    // Возвращает название издания.
    virtual string getTitle() const = 0;
    // Возвращает вид издания («Книга», «Периодическое издание», «Газета»).
    virtual string getType() const = 0;
    // Выводит сведения об издании.
    virtual void print() const = 0;
};

class ISellable : public virtual IItem // интерфейс 1: издание можно продать
{
public:
    // Возвращает цену одного экземпляра, руб.
    virtual double getPrice() const = 0;
};

class IReadable : public virtual IItem // интерфейс 2: издание можно прочитать
{
public:
    // Возвращает примерное время чтения в часах.
    virtual double readingTime() const = 0;
};

class ISubscribable : public virtual IItem // интерфейс 3: на издание можно подписаться
{
public:
    // Возвращает стоимость подписки на months месяцев.
    virtual double subscriptionCost(int months) const = 0;

    // Перегрузка функции: стоимость подписки на months месяцев для copies комплектов.
    // Вызывает виртуальную версию с одним параметром того класса, которому принадлежит объект.
    double subscriptionCost(int months, int copies) const
    {
        return subscriptionCost(months) * copies;
    }
};

// ---------- Абстрактный класс: печатное издание ----------

class PrintedEdition : public virtual IItem, public ISellable, public IReadable
{
public:
    // Конструктор: ставит значения по умолчанию, заменяет непустые название и издательство,
    // а год, страницы и цену задаёт через сеттеры, которые проверяют данные.
    PrintedEdition(string p_title, string p_publisher, int p_year, int p_pages, double p_price)
        : title{ "неизвестно" }, publisher{ "неизвестно" }, year{ CURRENT_YEAR }, pages{ 1 }, price{ 0 }
    {
        if (!p_title.empty())
            title = p_title;
        if (!p_publisher.empty())
            publisher = p_publisher;
        setYear(p_year); // числовые значения проверяются в сеттерах
        setPages(p_pages);
        setPrice(p_price);
    }

    // Возвращает название (реализация функции интерфейса IItem).
    string getTitle() const override
    {
        return title;
    }
    // Возвращает издательство.
    string getPublisher() const
    {
        return publisher;
    }
    // Возвращает год издания.
    int getYear() const
    {
        return year;
    }
    // Возвращает количество страниц.
    int getPages() const
    {
        return pages;
    }
    // Возвращает цену (реализация функции интерфейса ISellable).
    double getPrice() const override
    {
        return price;
    }

    // Задаёт год издания, если он от 1450 до текущего года, иначе оставляет старый.
    void setYear(int p_year)
    {
        if (p_year >= 1450 && p_year <= CURRENT_YEAR)
            year = p_year;
        else
            cout << "  Неверный год " << p_year << " (допустимо 1450-" << CURRENT_YEAR << "), оставлено: " << year << endl;
    }
    // Задаёт количество страниц, если оно больше 0, иначе оставляет старое.
    void setPages(int p_pages)
    {
        if (p_pages > 0)
            pages = p_pages;
        else
            cout << "  Число страниц должно быть больше 0, оставлено: " << pages << endl;
    }
    // Задаёт цену, если она не отрицательная, иначе оставляет старую.
    void setPrice(double p_price)
    {
        if (p_price >= 0)
            price = p_price;
        else
            cout << "  Цена не может быть отрицательной, оставлено: " << price << endl;
    }

    // Возвращает время чтения в часах при скорости 30 страниц в час
    // (реализация IReadable по умолчанию; производные классы переопределяют её).
    double readingTime() const override
    {
        return pages / 30.0;
    }

    // Выводит вид издания, название, издательство, год, страницы и цену.
    // Виртуальная функция: производные классы переопределяют её.
    void print() const override
    {
        cout << getType() << " \"" << title << "\"" << endl; // getType() вызывается виртуально
        cout << "  Издательство: " << publisher << ", год: " << year << endl;
        cout << "  Страниц: " << pages << ", цена: " << fixed << setprecision(2) << price << " руб." << endl;
    }

    // Выводит краткие сведения: название и год.
    // НЕ виртуальная функция: в производном классе её можно только скрыть.
    void printShort() const
    {
        cout << title << " (" << year << ")" << endl;
    }

    // Перегрузка оператора >: возвращает true, если текущее издание дороже edition.
    bool operator > (const PrintedEdition& edition) const
    {
        return getPrice() > edition.getPrice();
    }
    // Перегрузка оператора <: возвращает true, если текущее издание дешевле edition.
    bool operator < (const PrintedEdition& edition) const
    {
        return getPrice() < edition.getPrice();
    }

    // getType() не реализована - поэтому класс PrintedEdition абстрактный, создать его объект нельзя

private:
    string title;     // название
    string publisher; // издательство
    int year;         // год издания
    int pages;        // количество страниц
    double price;     // цена, руб.
};

// ---------- Книга ----------

class Book : public PrintedEdition
{
public:
    // Конструктор: общие поля передаёт конструктору PrintedEdition, затем задаёт автора и жанр.
    Book(string p_title, string p_publisher, int p_year, int p_pages, double p_price,
        string p_author, string p_genre)
        : PrintedEdition(p_title, p_publisher, p_year, p_pages, p_price), author{ p_author }, genre{ p_genre }
    {
    }

    // Возвращает автора.
    string getAuthor() const
    {
        return author;
    }
    // Возвращает жанр.
    string getGenre() const
    {
        return genre;
    }

    // Возвращает вид издания «Книга».
    string getType() const override
    {
        return "Книга";
    }

    // Возвращает время чтения: учебник читают медленнее (15 страниц в час),
    // остальные книги - по формуле базового класса (30 страниц в час).
    double readingTime() const override
    {
        if (genre == "учебник")
            return getPages() / 15.0;
        return PrintedEdition::readingTime(); // иначе - версия базового класса
    }

    // Выводит общие сведения (функцией базового класса), затем автора и жанр.
    void print() const override
    {
        PrintedEdition::print(); // вызываем функцию print из базового класса
        cout << "  Автор: " << author << ", жанр: " << genre << endl;
    }

    // Перегрузка оператора +: из двух книг получается сборник. Названия и авторы
    // соединяются, страницы и цены складываются, год берётся более новый.
    Book operator + (const Book& book) const
    {
        string newTitle = getTitle() + " / " + book.getTitle();
        string newAuthor = author + ", " + book.author;
        int newYear = getYear() > book.getYear() ? getYear() : book.getYear(); // год более новой книги
        return Book{ newTitle, getPublisher(), newYear, getPages() + book.getPages(), getPrice() + book.getPrice(),
                    newAuthor, "сборник" };
    }

private:
    string author; // автор
    string genre;  // жанр
};

// ---------- Периодика ----------

class Periodical : public PrintedEdition, public ISubscribable
{
public:
    // Конструктор: общие поля передаёт PrintedEdition, затем проверяет и задаёт
    // число выпусков в год (1-365) и номер выпуска (не меньше 1).
    Periodical(string p_title, string p_publisher, int p_year, int p_pages, double p_price,
        int p_issuesPerYear, int p_issueNumber)
        : PrintedEdition(p_title, p_publisher, p_year, p_pages, p_price), issuesPerYear{ 12 }, issueNumber{ 1 }
    {
        if (p_issuesPerYear >= 1 && p_issuesPerYear <= 365)
            issuesPerYear = p_issuesPerYear;
        else
            cout << "  Выпусков в год должно быть от 1 до 365, оставлено: 12" << endl;
        if (p_issueNumber >= 1)
            issueNumber = p_issueNumber;
        else
            cout << "  Номер выпуска должен быть больше 0, оставлено: 1" << endl;
    }

    // Возвращает число выпусков в год.
    int getIssuesPerYear() const
    {
        return issuesPerYear;
    }
    // Возвращает номер выпуска.
    int getIssueNumber() const
    {
        return issueNumber;
    }

    // Возвращает вид издания «Периодическое издание».
    string getType() const override
    {
        return "Периодическое издание";
    }

    // Возвращает время чтения: журнал просматривают быстрее - 60 страниц в час.
    double readingTime() const override
    {
        return getPages() / 60.0;
    }

    using ISubscribable::subscriptionCost; // чтобы перегрузка с двумя параметрами не скрывалась

    // Возвращает стоимость подписки на months месяцев со скидкой 10 %:
    // цена номера * число номеров за срок * 0,9.
    double subscriptionCost(int months) const override
    {
        return getPrice() * issuesPerYear * months / 12.0 * 0.9;
    }

    // Выводит общие сведения (функцией базового класса), затем номер выпуска и периодичность.
    void print() const override
    {
        PrintedEdition::print();
        cout << "  Выпуск №" << issueNumber << ", выходит " << issuesPerYear << " раз в год" << endl;
    }

private:
    int issuesPerYear; // выпусков в год
    int issueNumber;   // номер выпуска
};

// ---------- Газета ----------

class Newspaper : public Periodical
{
public:
    // Конструктор: поля периодики передаёт конструктору Periodical, затем проверяет
    // и задаёт формат (A2, A3, A4) и тираж (больше 0).
    Newspaper(string p_title, string p_publisher, int p_year, int p_pages, double p_price,
        int p_issuesPerYear, int p_issueNumber, string p_format, int p_circulation)
        : Periodical(p_title, p_publisher, p_year, p_pages, p_price, p_issuesPerYear, p_issueNumber),
        format{ "A3" }, circulation{ 1000 }
    {
        if (p_format == "A2" || p_format == "A3" || p_format == "A4")
            format = p_format;
        else
            cout << "  Формат должен быть A2, A3 или A4, оставлено: " << format << endl;
        if (p_circulation > 0)
            circulation = p_circulation;
        else
            cout << "  Тираж должен быть больше 0, оставлено: 1000" << endl;
    }

    // Возвращает true, если газета выходит 300 и более раз в год (ежедневная).
    bool isDaily() const
    {
        return getIssuesPerYear() >= 300;
    }

    // Возвращает вид издания «Газета».
    string getType() const override
    {
        return "Газета";
    }

    // Возвращает время чтения: газету читают по 3 минуты на полосу.
    double readingTime() const override
    {
        return getPages() * 3 / 60.0;
    }

    using Periodical::subscriptionCost; // перегрузка с двумя параметрами остаётся доступной

    // Возвращает стоимость подписки: для ежедневной газеты скидка 20 %,
    // иначе - как у периодики (функция базового класса).
    double subscriptionCost(int months) const override
    {
        if (isDaily())
            return getPrice() * getIssuesPerYear() * months / 12.0 * 0.8;
        return Periodical::subscriptionCost(months);
    }

    // Выводит сведения периодики (функцией базового класса), затем формат, тираж
    // и отметку «ежедневная».
    void print() const override
    {
        Periodical::print();
        cout << "  Формат: " << format << ", тираж: " << circulation << " экз.";
        if (isDaily())
            cout << ", ежедневная";
        cout << endl;
    }

    // Выводит краткие сведения о газете: название, номер выпуска и тираж.
    // Скрывает (а не переопределяет) функцию printShort базового класса.
    void printShort() const
    {
        cout << "Газета " << getTitle() << ", выпуск №" << getIssueNumber() << ", тираж " << circulation << " экз." << endl;
    }

private:
    string format;   // формат полосы (A2, A3, A4)
    int circulation; // тираж, экз.
};

// ---------- Клиентская программа ----------

// Вводит поля, общие для всех изданий: название, издательство, год, страницы и цену.
// Значения записываются в переменные вызывающей функции через ссылки.
void inputCommon(string& title, string& publisher, int& year, int& pages, double& price)
{
    title = readLine("  Название: ");
    publisher = readLine("  Издательство: ");
    year = readInt("  Год издания: ");
    pages = readInt("  Количество страниц: ");
    price = readDouble("  Цена, руб.: ");
}

// Вводит номер издания от 1 до size. Возвращает номер или 0, если номер неверный.
int readNumber(int size)
{
    int k = readInt("  Номер издания (1-" + to_string(size) + "): ");
    if (k < 1 || k > size)
    {
        cout << "Нет издания с таким номером." << endl;
        return 0;
    }
    return k;
}

// Главная функция: меню для работы с изданиями через интерфейсы и указатель на базовый класс.
int main()
{
    setlocale(LC_ALL, "rus");
#ifdef _WIN32
    SetConsoleCP(1251);       // ввод русских букв с клавиатуры
    SetConsoleOutputCP(1251); // вывод русских букв
#endif

    vector<PrintedEdition*> items; // все издания хранятся через указатель на абстрактный базовый класс

    int choice; // пункт меню
    do
    {
        cout << "\n===== Печатные издания (полиморфизм) =====" << endl;
        cout << "1 - добавить книгу" << endl;
        cout << "2 - добавить периодическое издание" << endl;
        cout << "3 - добавить газету" << endl;
        cout << "4 - показать все издания (IItem)" << endl;
        cout << "5 - стоимость покупки (ISellable)" << endl;
        cout << "6 - время чтения (IReadable)" << endl;
        cout << "7 - стоимость подписки (ISubscribable)" << endl;
        cout << "8 - изменить цену издания" << endl;
        cout << "9 - сравнить два издания по цене (операторы < и >)" << endl;
        cout << "10 - объединить две книги в сборник (оператор +)" << endl;
        cout << "11 - переопределение и скрытие функции" << endl;
        cout << "0 - выход" << endl;
        choice = readInt("Ваш выбор: ");

        switch (choice)
        {
        case 1: // ввод книги
        {
            string title, publisher, author, genre;
            int year, pages;
            double price;
            cout << "Ввод книги:" << endl;
            inputCommon(title, publisher, year, pages, price);
            author = readLine("  Автор: ");
            genre = readLine("  Жанр: ");
            items.push_back(new Book{ title, publisher, year, pages, price, author, genre }); // объект в динамической памяти
            cout << "Книга добавлена." << endl;
            break;
        }
        case 2: // ввод периодического издания
        {
            string title, publisher;
            int year, pages, issues, number;
            double price;
            cout << "Ввод периодического издания:" << endl;
            inputCommon(title, publisher, year, pages, price);
            issues = readInt("  Выпусков в год: ");
            number = readInt("  Номер выпуска: ");
            items.push_back(new Periodical{ title, publisher, year, pages, price, issues, number });
            cout << "Периодическое издание добавлено." << endl;
            break;
        }
        case 3: // ввод газеты
        {
            string title, publisher, format;
            int year, pages, issues, number, circulation;
            double price;
            cout << "Ввод газеты:" << endl;
            inputCommon(title, publisher, year, pages, price);
            issues = readInt("  Выпусков в год: ");
            number = readInt("  Номер выпуска: ");
            format = readLine("  Формат (A2/A3/A4): ");
            circulation = readInt("  Тираж, экз.: ");
            items.push_back(new Newspaper{ title, publisher, year, pages, price, issues, number, format, circulation });
            cout << "Газета добавлена." << endl;
            break;
        }
        case 4: // полиморфный вывод через интерфейс IItem
        {
            if (items.empty())
            {
                cout << "Список изданий пуст." << endl;
                break;
            }
            for (int i = 0; i < static_cast<int>(items.size()); i++)
            {
                IItem* item = items[i]; // благодаря виртуальному наследованию у объекта одна часть IItem
                cout << "\n" << i + 1 << ". ";
                item->print(); // вызывается print() нужного класса
            }
            break;
        }
        case 5: // работа через интерфейс ISellable
        {
            double total{};
            for (int i = 0; i < static_cast<int>(items.size()); i++)
            {
                ISellable* s = items[i];
                cout << "  " << s->getType() << " \"" << s->getTitle() << "\": "
                    << fixed << setprecision(2) << s->getPrice() << " руб." << endl;
                total += s->getPrice();
            }
            cout << "Итого: " << fixed << setprecision(2) << total << " руб." << endl;
            break;
        }
        case 6: // работа через интерфейс IReadable
        {
            for (int i = 0; i < static_cast<int>(items.size()); i++)
            {
                IReadable* r = items[i];
                cout << "  " << r->getType() << " \"" << r->getTitle() << "\": "
                    << fixed << setprecision(1) << r->readingTime() << " ч" << endl; // своя формула у каждого класса
            }
            break;
        }
        case 7: // работа через интерфейс ISubscribable
        {
            int months = readInt("На сколько месяцев оформить подписку: ");
            if (months < 1 || months > 12)
            {
                cout << "Срок подписки должен быть от 1 до 12 месяцев." << endl;
                break;
            }
            int copies = readInt("Количество комплектов: ");
            if (copies < 1)
            {
                cout << "Количество комплектов должно быть больше 0." << endl;
                break;
            }
            double total{};
            int found{}; // сколько изданий поддерживают подписку
            for (int i = 0; i < static_cast<int>(items.size()); i++)
            {
                ISubscribable* sub = dynamic_cast<ISubscribable*>(items[i]); // есть ли у объекта интерфейс подписки
                if (!sub) // то же, что sub == nullptr
                {
                    cout << "  " << items[i]->getType() << " \"" << items[i]->getTitle() << "\": подписка невозможна" << endl;
                    continue;
                }
                double c = sub->subscriptionCost(months, copies); // перегруженная функция с двумя параметрами
                cout << "  " << sub->getType() << " \"" << sub->getTitle() << "\": "
                    << fixed << setprecision(2) << c << " руб." << endl;
                total += c;
                found++;
            }
            cout << "Изданий с подпиской: " << found << ", итого за " << months << " мес.: "
                << fixed << setprecision(2) << total << " руб." << endl;
            break;
        }
        case 8: // изменение объекта через сеттер
        {
            if (items.empty())
            {
                cout << "Список изданий пуст." << endl;
                break;
            }
            int k = readNumber(static_cast<int>(items.size()));
            if (k == 0)
                break;
            double p = readDouble("Новая цена, руб.: ");
            items[k - 1]->setPrice(p); // при неверной цене сеттер оставит старое значение
            cout << "Цена \"" << items[k - 1]->getTitle() << "\": " << fixed << setprecision(2)
                << items[k - 1]->getPrice() << " руб." << endl;
            break;
        }
        case 9: // перегруженные операторы < и >
        {
            if (items.size() < 2)
            {
                cout << "Нужно хотя бы два издания." << endl;
                break;
            }
            int a = readNumber(static_cast<int>(items.size()));
            if (a == 0)
                break;
            int b = readNumber(static_cast<int>(items.size()));
            if (b == 0)
                break;
            const PrintedEdition& e1 = *items[a - 1];
            const PrintedEdition& e2 = *items[b - 1];
            if (e1 > e2)
                cout << "\"" << e1.getTitle() << "\" дороже, чем \"" << e2.getTitle() << "\"" << endl;
            else if (e1 < e2)
                cout << "\"" << e1.getTitle() << "\" дешевле, чем \"" << e2.getTitle() << "\"" << endl;
            else
                cout << "\"" << e1.getTitle() << "\" и \"" << e2.getTitle() << "\" стоят одинаково" << endl;
            break;
        }
        case 10: // перегруженный оператор +
        {
            if (items.size() < 2)
            {
                cout << "Нужно хотя бы два издания." << endl;
                break;
            }
            int a = readNumber(static_cast<int>(items.size()));
            if (a == 0)
                break;
            int b = readNumber(static_cast<int>(items.size()));
            if (b == 0)
                break;
            Book* b1 = dynamic_cast<Book*>(items[a - 1]);
            Book* b2 = dynamic_cast<Book*>(items[b - 1]);
            if (!b1 || !b2)
            {
                cout << "Объединять можно только книги." << endl;
                break;
            }
            Book collection = *b1 + *b2; // сборник из двух книг
            items.push_back(new Book{ collection });
            cout << "Создан сборник:" << endl;
            items.back()->print();
            break;
        }
        case 11: // разница между переопределением (virtual) и скрытием функции
        {
            Newspaper* news{ nullptr };
            for (int i = 0; i < static_cast<int>(items.size()); i++)
            {
                news = dynamic_cast<Newspaper*>(items[i]);
                if (news) // первая газета в списке
                    break;
            }
            if (!news)
            {
                cout << "Добавьте хотя бы одну газету." << endl;
                break;
            }
            PrintedEdition* p = news; // указатель на базовый класс, объект - газета
            cout << "Через указатель на PrintedEdition:" << endl;
            cout << "  print() (виртуальная) - функция класса Newspaper:" << endl;
            p->print();
            cout << "  printShort() (скрытая) - функция класса PrintedEdition: ";
            p->printShort();
            cout << "Через указатель на Newspaper:" << endl;
            cout << "  printShort() - функция класса Newspaper: ";
            news->printShort();
            break;
        }
        case 0:
            cout << "Работа программы завершена." << endl;
            break;
        default:
            cout << "Нет такого пункта меню." << endl;
        }
    } while (choice != 0);

    for (int i = 0; i < static_cast<int>(items.size()); i++)
        delete items[i]; // освобождаем память; благодаря виртуальному деструктору вызывается деструктор нужного класса

    system("PAUSE");
    return 0;
}
