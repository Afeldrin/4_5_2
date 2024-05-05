#include <thread>
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <string>

std::condition_variable g_Bell;
std::condition_variable_any g_Door;

class Manager
{
public:
    void ComeToWork()
    {
        std::cout << "Hey security, please open the door!\n"; //фраза менеджера
        g_Bell.notify_one(); //Разблокировка одного из ожидающих потоков
        std::mutex mutex; // объявление mutex-переменной
        mutex.lock(); // блокировка мьютекса
        g_Door.wait(mutex); //присваивание мьютекса переменной g_Door
        mutex.unlock(); //разблокировка мьютекса
    }
};

class Security
{
    static bool m_SectorClear; // Объявление статик-переменной
    static std::mutex m_SectorMutex; // Объявление мьютекса
public:
    static bool SectorClear()
    {
        std::lock_guard<std::mutex> lock(m_SectorMutex); // Блокирует мьютекс при создании и разблокирует при выходе их области видимости
        return m_SectorClear;
    }
    void NotifyFellows()
    {
        std::lock_guard<std::mutex> lock(m_SectorMutex);
        m_SectorClear = false;
    }
    void WorkHard()
    {
        m_SectorClear = true; //По умолчнию всё "чисто", прогеры могут играть в старкрафт
        std::mutex mutex;
        std::unique_lock<std::mutex> lock(mutex);
        while (true)
        {
            if (g_Bell.wait_for(lock, std::chrono::seconds(5)) == std::cv_status::timeout) 
                std::this_thread::sleep_for(std::chrono::seconds(10)); //Вводит текущий поток в сон на 10 секунд
            else
            {
                NotifyFellows();
                g_Door.notify_one();
                std::cout << "Hello Great Manager, your slaves are ready to serve you!\n" << std::endl;
            }
        }
    }
};

bool Security::m_SectorClear;
std::mutex Security::m_SectorMutex;

class Programmer
{
public:
    void WorkHard()
    {
        std::cout << "Let's write some govnokod!\n" << std::endl;
        int i = 0;
        while (true)                    //Процесс выполнения "говнокода"
        {
            i++;
            i--;
        }
    }
    void PlayStarcraft()
    {
        while (Security::SectorClear()) //После получения сигнала от охраны что SectorClear == false начинают писать говнокод
            ;//Играем! :)
        WorkHard();// Работаем :(
    }
};



int main()
{
    Manager manager;
    Programmer programmer;
    Security security;

    

    auto managerCall = [&manager]()
        {
            manager.ComeToWork(); //ссылка на метод класса. По сути это нужно, чтобы передать метод класса как функцию в поток
        };
    auto programmerCall = [&programmer]()
        {
            programmer.PlayStarcraft();  
        };
    auto securityCall = [&security]()
        {
            security.WorkHard();
        };

    //Параллельно работает охрана, прогеры играют в старкрафт, менеджер приходит на работу спустя минуту

    std::thread securityThread(securityCall);
    std::thread programmerThread(programmerCall);
    std::this_thread::sleep_for(std::chrono::minutes(1));
    std::thread managerThread(managerCall);

    //Завершение потоков

    managerThread.join();
    programmerThread.join();
    securityThread.join();

    return 0;

} //Вывод: В коде создаются 3 класса: охрана, менеджер и прогер. Методы этих классов будут выполняться параллельно при помощи многопоточного программирования. \
при этом сообщение между потоками организованно через conditional variables, mutex. Охрана работает, проверяет не позвонил ли звонок в дверь, если нет, то спит, если да, то оповещает \
прогеров, что пора работать, открывает дверь, приветствует менеджера. Менеджер приходит и разблокирует звонок, таким образом охрана понимает, что менеджер пришел.