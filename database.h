#pragma once
#include <string>
#include <iostream>
#include <map>
#include <mutex>
#include <shared_mutex>

/**
 * @brief The table class - Класс таблица из БД
 */
class table
{
public: 
    table();
    table(std::string name);
    table(const table& tb);
    table& operator=(const table& tb);

    bool insert(int key, std::string value, std::string& errorMsg);
    bool truncate();
    std::map<int, std::string> getValues() const;
    std::string getName() const;

    std::mutex& getInsertTruncMutex();
    std::shared_mutex& getSharedMutex();

private:
    std::map<int, std::string> m_values;
    std::string m_name;
    std::mutex m_insertTruncMutex;
    std::shared_mutex m_sharedMutex;
};

/**
 * @brief The database class - Класс базы данных
 */
class database
{
public:
    
    database();
    bool insert(std::string table, int key, std::string value, std::string& errorMsg);
    bool truncate(std::string table, std::string& errorMsg);
    bool intersection(std::string& errorMsg);
    bool symmetric_difference(std::string& errorMsg);
    bool exec(std::string_view cmd, std::string& resultMsg);

private:
    std::map<std::string, table> m_tables;
};

