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

// ---------- Базовый класс: печатное издание ----------

class PrintedEdition
{
public:
    // Конструктор с параметрами: увеличивает счётчик объектов, ставит значения
    // по умолчанию и задаёт поля через сеттеры, которые проверяют данные.
    PrintedEdition(string p_title, string p_publisher, int p_year, int p_pages, double p_price)
    {
        ++count; // при создании нового объекта увеличиваем счётчик
        title = "неизвестно";  // значения по умолчанию на случай неверных данных
        year = CURRENT_YEAR;
        pages = 1;
        price = 0;
        setTitle(p_title); // значения задаются через сеттеры, которые их проверяют
        setPublisher(p_publisher);
        setYear(p_year);
        setPages(p_pages);
        setPrice(p_price);
    }
    // Конструктор без параметров: вызывает конструктор с параметрами со значениями по умолчанию.
    PrintedEdition() : PrintedEdition("неизвестно", "неизвестно", CURRENT_YEAR, 1, 0) // делегирующий конструктор
    {
    }

    // Выводит заголовок «Печатное издание» и общие сведения об издании.
    void print() const
    {
        cout << "Печатное издание" << endl;
        printBase();
    }

    // Возвращает название издания.
    string getTitle() const
    {
        return title;
    }
    // Задаёт название; пустое название не принимается, остаётся старое.
    void setTitle(string p_title)
    {
        if (p_title.empty())
            cout << "  Название не может быть пустым, оставлено: " << title << endl;
        else
            title = p_title;
    }

    // Возвращает издательство (сеттер закрыт, поэтому снаружи его изменить нельзя).
    string getPublisher() const
    {
        return publisher;
    }

    // Возвращает год издания.
    int getYear() const
    {
        return year;
    }
    // Задаёт год издания, если он от 1450 до текущего года, иначе оставляет старый.
    void setYear(int p_year)
    {
        if (p_year >= 1450 && p_year <= CURRENT_YEAR) // книгопечатание появилось около 1450 г.
            year = p_year;
        else
            cout << "  Неверный год " << p_year << " (допустимо 1450-" << CURRENT_YEAR << "), оставлено: " << year << endl;
    }

    // Возвращает количество страниц.
    int getPages() const
    {
        return pages;
    }
    // Задаёт количество страниц, если оно больше 0, иначе оставляет старое.
    void setPages(int p_pages)
    {
        if (p_pages > 0)
            pages = p_pages;
        else
            cout << "  Число страниц должно быть больше 0, оставлено: " << pages << endl;
    }

    // Возвращает цену одного экземпляра.
    double getPrice() const
    {
        return price;
    }
    // Задаёт цену, если она не отрицательная, иначе оставляет старую.
    void setPrice(double p_price)
    {
        if (p_price >= 0)
            price = p_price;
        else
            cout << "  Цена не может быть отрицательной, оставлено: " << price << endl;
    }

    // Возвращает возраст издания в годах (текущий год минус год издания).
    int getAge() const // возраст вычисляется по году издания, установить его нельзя
    {
        return CURRENT_YEAR - year;
    }

    // Выводит, сколько всего создано печатных изданий; вызывается без объекта.
    static void print_count() // статическая функция: вызывается через имя класса
    {
        cout << "Создано объектов печатных изданий: " << count << endl;
    }

protected: // доступно в этом классе и в производных классах
    // Выводит общие сведения: название, издательство, год и возраст, страницы и цену.
    // Используется функциями print() производных классов.
    void printBase() const
    {
        cout << "  Название: " << title << endl;
        cout << "  Издательство: " << publisher << endl;
        cout << "  Год издания: " << year << " (изданию лет: " << getAge() << ")" << endl;
        cout << "  Страниц: " << pages << ", цена: " << fixed << setprecision(2) << price << " руб." << endl;
    }

private:
    // Задаёт издательство; пустая строка заменяется на «неизвестно».
    void setPublisher(string p_publisher) // закрытый сеттер: издательство задаётся только в конструкторе
    {
        if (p_publisher.empty())
            publisher = "неизвестно";
        else
            publisher = p_publisher;
    }

    string title;          // название
    string publisher;      // издательство
    int year;              // год издания
    int pages;             // количество страниц
    double price;          // цена, руб.
    static unsigned count; // статическое поле - счётчик объектов PrintedEdition
};

unsigned PrintedEdition::count{}; // определение статического поля (начальное значение 0)

// ---------- Производный класс: книга ----------

class Book : public PrintedEdition
{
public:
    // Конструктор: общие поля передаёт конструктору PrintedEdition, затем задаёт автора и жанр.
    Book(string p_title, string p_publisher, int p_year, int p_pages, double p_price,
        string p_author, string p_genre)
        : PrintedEdition(p_title, p_publisher, p_year, p_pages, p_price) // вызов конструктора базового класса
    {
        setAuthor(p_author);
        setGenre(p_genre);
    }
    // Конструктор без параметров: книга со значениями по умолчанию.
    Book() : Book("неизвестно", "неизвестно", CURRENT_YEAR, 1, 0, "неизвестно", "неизвестно")
    {
    }

    // Выводит «Книга», общие сведения, автора, жанр и время чтения при скорости 30 стр./ч.
    void print() const // скрывает функцию print базового класса
    {
        cout << "Книга" << endl;
        printBase(); // защищённая функция базового класса доступна в производном
        cout << "  Автор: " << author << ", жанр: " << genre << endl;
        cout << "  Время чтения (30 стр./ч): " << fixed << setprecision(1) << readingTime(30) << " ч" << endl;
    }

    // Возвращает автора.
    string getAuthor() const
    {
        return author;
    }
    // Задаёт автора; пустая строка заменяется на «неизвестно».
    void setAuthor(string p_author)
    {
        if (p_author.empty())
            author = "неизвестно";
        else
            author = p_author;
    }

    // Возвращает жанр.
    string getGenre() const
    {
        return genre;
    }
    // Задаёт жанр; пустая строка заменяется на «неизвестно».
    void setGenre(string p_genre)
    {
        if (p_genre.empty())
            genre = "неизвестно";
        else
            genre = p_genre;
    }

    // Возвращает время чтения книги в часах при скорости pagesPerHour страниц в час.
    double readingTime(int pagesPerHour) const
    {
        if (pagesPerHour <= 0)
            return 0; // скорость чтения должна быть положительной
        return static_cast<double>(getPages()) / pagesPerHour; // поле pages закрыто - используем геттер
    }

private:
    string author; // автор
    string genre;  // жанр
};

// ---------- Производный класс: периодика ----------

class Periodical : public PrintedEdition
{
public:
    // Конструктор: общие поля передаёт PrintedEdition, затем задаёт
    // число выпусков в год и номер выпуска через сеттеры.
    Periodical(string p_title, string p_publisher, int p_year, int p_pages, double p_price,
        int p_issuesPerYear, int p_issueNumber)
        : PrintedEdition(p_title, p_publisher, p_year, p_pages, p_price)
    {
        issuesPerYear = 12;
        issueNumber = 1;
        setIssuesPerYear(p_issuesPerYear);
        setIssueNumber(p_issueNumber);
    }
    // Конструктор без параметров: 12 выпусков в год, выпуск №1.
    Periodical() : Periodical("неизвестно", "неизвестно", CURRENT_YEAR, 1, 0, 12, 1)
    {
    }

    // Выводит общие сведения, номер выпуска, периодичность и стоимость подписки на 6 месяцев.
    void print() const
    {
        cout << "Периодическое издание" << endl;
        printBase();
        cout << "  Выпуск №" << issueNumber << ", выходит " << issuesPerYear << " раз в год (" << frequency() << ")" << endl;
        cout << "  Подписка на 6 месяцев: " << fixed << setprecision(2) << subscriptionCost(6) << " руб." << endl;
    }

    // Возвращает число выпусков в год.
    int getIssuesPerYear() const
    {
        return issuesPerYear;
    }
    // Задаёт число выпусков в год, если оно от 1 до 365, иначе оставляет старое.
    void setIssuesPerYear(int p_issuesPerYear)
    {
        if (p_issuesPerYear >= 1 && p_issuesPerYear <= 365)
            issuesPerYear = p_issuesPerYear;
        else
            cout << "  Выпусков в год должно быть от 1 до 365, оставлено: " << issuesPerYear << endl;
    }

    // Возвращает номер выпуска.
    int getIssueNumber() const
    {
        return issueNumber;
    }
    // Задаёт номер выпуска, если он не меньше 1, иначе оставляет старый.
    void setIssueNumber(int p_issueNumber)
    {
        if (p_issueNumber >= 1)
            issueNumber = p_issueNumber;
        else
            cout << "  Номер выпуска должен быть больше 0, оставлено: " << issueNumber << endl;
    }

    // Возвращает периодичность словами по числу выпусков в год.
    string frequency() const
    {
        if (issuesPerYear >= 300)
            return "ежедневно";
        if (issuesPerYear >= 100)
            return "несколько раз в неделю";
        if (issuesPerYear >= 50)
            return "еженедельно";
        if (issuesPerYear >= 12)
            return "ежемесячно";
        return "реже раза в месяц";
    }

    // Возвращает стоимость подписки на months месяцев: цена номера * число номеров за срок.
    double subscriptionCost(int months) const
    {
        return getPrice() * issuesPerYear * months / 12.0;
    }

private:
    int issuesPerYear; // выпусков в год
    int issueNumber;   // номер выпуска
};

// ---------- Производный класс второго уровня: газета ----------

class Newspaper final : public Periodical // final - от газеты наследовать нельзя
{
public:
    // Конструктор: поля периодики передаёт конструктору Periodical,
    // затем задаёт формат и тираж через сеттеры.
    Newspaper(string p_title, string p_publisher, int p_year, int p_pages, double p_price,
        int p_issuesPerYear, int p_issueNumber, string p_format, int p_circulation)
        : Periodical(p_title, p_publisher, p_year, p_pages, p_price, p_issuesPerYear, p_issueNumber)
    {
        format = "A3";
        circulation = 1000;
        setFormat(p_format);
        setCirculation(p_circulation);
    }
    // Конструктор без параметров: ежедневная газета формата A3 тиражом 1000 экз.
    Newspaper() : Newspaper("неизвестно", "неизвестно", CURRENT_YEAR, 1, 0, 365, 1, "A3", 1000)
    {
    }

    // Выводит «Газета», общие сведения, периодичность, формат, тираж и стоимость подписки на 6 месяцев.
    void print() const
    {
        cout << "Газета" << endl;
        printBase();
        cout << "  Выпуск №" << getIssueNumber() << ", выходит " << getIssuesPerYear() << " раз в год (" << frequency() << ")";
        if (isDaily())
            cout << ", ежедневная газета";
        cout << endl;
        cout << "  Формат: " << format << ", тираж: " << circulation << " экз." << endl;
        cout << "  Подписка на 6 месяцев: " << fixed << setprecision(2) << subscriptionCost(6) << " руб." << endl;
    }

    // Возвращает формат полосы.
    string getFormat() const
    {
        return format;
    }
    // Задаёт формат, если это A2, A3 или A4, иначе оставляет старый.
    void setFormat(string p_format)
    {
        if (p_format == "A2" || p_format == "A3" || p_format == "A4")
            format = p_format;
        else
            cout << "  Формат должен быть A2, A3 или A4, оставлено: " << format << endl;
    }

    // Возвращает тираж.
    int getCirculation() const
    {
        return circulation;
    }
    // Задаёт тираж, если он больше 0, иначе оставляет старый.
    void setCirculation(int p_circulation)
    {
        if (p_circulation > 0)
            circulation = p_circulation;
        else
            cout << "  Тираж должен быть больше 0, оставлено: " << circulation << endl;
    }

    // Возвращает true, если газета выходит 300 и более раз в год (ежедневная).
    bool isDaily() const
    {
        return getIssuesPerYear() >= 300;
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

// Главная функция: меню для добавления изданий, вывода списка и расчёта подписки.
int main()
{
    setlocale(LC_ALL, "rus");
#ifdef _WIN32
    SetConsoleCP(1251);       // ввод русских букв с клавиатуры
    SetConsoleOutputCP(1251); // вывод русских букв
#endif

    vector<Book> books;             // список книг
    vector<Periodical> periodicals; // список периодических изданий
    vector<Newspaper> newspapers;   // список газет

    int choice; // пункт меню
    do
    {
        cout << "\n===== Печатные издания =====" << endl;
        cout << "1 - добавить книгу" << endl;
        cout << "2 - добавить периодическое издание" << endl;
        cout << "3 - добавить газету" << endl;
        cout << "4 - показать все издания" << endl;
        cout << "5 - стоимость подписки на периодику и газеты" << endl;
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
            books.push_back(Book{ title, publisher, year, pages, price, author, genre }); // создаём объект
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
            periodicals.push_back(Periodical{ title, publisher, year, pages, price, issues, number });
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
            newspapers.push_back(Newspaper{ title, publisher, year, pages, price, issues, number, format, circulation });
            cout << "Газета добавлена." << endl;
            break;
        }
        case 4: // вывод всех изданий
        {
            cout << endl;
            PrintedEdition::print_count(); // статическая функция вызывается через имя класса
            int k{ 1 }; // порядковый номер
            for (int i = 0; i < static_cast<int>(books.size()); i++)
            {
                cout << "\n" << k << ". ";
                books[i].print();
                k++;
            }
            for (int i = 0; i < static_cast<int>(periodicals.size()); i++)
            {
                cout << "\n" << k << ". ";
                periodicals[i].print();
                k++;
            }
            for (int i = 0; i < static_cast<int>(newspapers.size()); i++)
            {
                cout << "\n" << k << ". ";
                newspapers[i].print();
                k++;
            }
            if (k == 1)
                cout << "Список изданий пуст." << endl;
            break;
        }
        case 5: // подписка
        {
            int months = readInt("На сколько месяцев оформить подписку: ");
            if (months < 1 || months > 12)
            {
                cout << "Срок подписки должен быть от 1 до 12 месяцев." << endl;
                break;
            }
            double total{}; // общая стоимость
            for (int i = 0; i < static_cast<int>(periodicals.size()); i++)
            {
                double c = periodicals[i].subscriptionCost(months);
                cout << "  " << periodicals[i].getTitle() << ": " << fixed << setprecision(2) << c << " руб." << endl;
                total += c;
            }
            for (int i = 0; i < static_cast<int>(newspapers.size()); i++)
            {
                double c = newspapers[i].subscriptionCost(months); // функция унаследована от Periodical
                cout << "  " << newspapers[i].getTitle() << ": " << fixed << setprecision(2) << c << " руб." << endl;
                total += c;
            }
            cout << "Итого за " << months << " мес.: " << fixed << setprecision(2) << total << " руб." << endl;
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
