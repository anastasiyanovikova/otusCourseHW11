#include "commandHelper.h"

void commandHelper::addNewCommand(const std::string_view &newStr)
{
    m_currentBlock = newStr;
}
std::optional<std::string> commandHelper::getCommand()
{
    if(m_currentBlock.empty())
        return {};
    bool completer = true;
    auto endPos = m_currentBlock.find_first_of('\n');
    std::string_view newLine;
    if(endPos == std::string::npos)
    {
        newLine = m_currentBlock;
        m_currentBlock.remove_prefix(m_currentBlock.length());
        completer = false;
    }
    else
    {
        newLine = m_currentBlock.substr(0, endPos);
        m_currentBlock.remove_prefix(endPos + 1);
    }
    m_lastBlock += newLine;
    if(!completer)
        return {};
    std::string command = m_lastBlock;
    m_lastBlock.clear();
    return command;
}