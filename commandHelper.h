#pragma once

#include <optional>
#include <string_view>
#include <string>

/**
 * @brief The commandHelper class - Класс разбора команд от клиента
 */
class commandHelper
{
public:
    void addNewCommand(const std::string_view &newStr);
    std::optional<std::string> getCommand();

private:
    std::string m_lastBlock;
    std::string_view m_currentBlock;
};
