#include "database.h"
#include <iostream>
#include <sstream>
#include <boost/iostreams/stream.hpp>
#include <vector>
#include <boost/algorithm/string.hpp>

table::table(){}
table::table(std::string name): m_name(name)
{
}
table::table(const table& tb)
{
  m_name = tb.getName();
  m_values = tb.getValues();
}
table& table::operator=(const table& tb)
{
  if(&tb != this)
  {
    m_name = tb.getName();
    m_values = tb.getValues();
  }
  return *this;
}

bool table::insert(int key, std::string value, std::string& errorMsg)
{
  const std::lock_guard<std::mutex> guard(m_insertTruncMutex);
  auto tableVal = m_values.find(key);
  if(tableVal != m_values.end())
  {
    errorMsg = "dublicate " + std::to_string(key) + '\n';
    return false;
  }
  m_values[key] = value;
  return true;
}

bool table::truncate()
{
  const std::scoped_lock lock(m_insertTruncMutex, m_sharedMutex);
  m_values.clear();
  return true;
}

std::map<int, std::string> table::getValues() const
{
  return m_values;
}

std::mutex& table::getInsertTruncMutex()
{
  return m_insertTruncMutex;
}
std::shared_mutex& table::getSharedMutex()
{
  return m_sharedMutex;
}
std::string table::getName() const
{
  return m_name;
}
database::database()
{
  m_tables["A"] =  table("A");
  m_tables["B"] =  table("B");
}

bool database::insert(std::string table, int key, std::string value, std::string& errorMsg)
{
  auto it = m_tables.find(table);
  if(it == m_tables.end())
  {
    errorMsg = "Таблица не найдена.";
    return false;
  }

  return it->second.insert(key, value, errorMsg);
}

bool database::truncate(std::string table, std::string& errorMsg)
{
  auto it = m_tables.find(table);
  if(it == m_tables.end())
  {
    errorMsg = "Таблица не найдена.";
    return false;
  }
  return it->second.truncate();
}

bool database::intersection(std::string& errorMsg)
{
  if(m_tables.size() < 2)
  {
    errorMsg = "Отсутствуют таблицы.";
    return false;
  }
  std::ostringstream result;
  auto A = m_tables["A"];
  auto B = m_tables["B"];
  
  std::shared_lock Alock(A.getSharedMutex(), std::defer_lock);
  std::shared_lock Block(B.getSharedMutex(), std::defer_lock);
  const std::scoped_lock lock(Alock, Block);

  std::map<int, std::string> aVals;
  std::map<int, std::string> bVals;
  {
    const std::scoped_lock lockSc(A.getInsertTruncMutex(), B.getInsertTruncMutex());
    aVals = A.getValues();
    bVals = B.getValues();
  }

  for(auto it = aVals.begin(); it != aVals.end(); it++)
  {
    auto bIt = bVals.find(it->first);
    if(bIt != bVals.end())
      result << it->first << ", " << it->second << ", " << bIt->second <<"\n";
  }
  result <<"OK\n";
  errorMsg = result.str();
  return true;
}

bool database::symmetric_difference(std::string& errorMsg)
{
  if(m_tables.size() < 2)
  {
    errorMsg = "Отсутствуют таблицы.";
    return false;
  }

  std::ostringstream result;
  auto A = m_tables["A"];
  auto B = m_tables["B"];
  
  std::shared_lock Alock(A.getSharedMutex(), std::defer_lock);
  std::shared_lock Block(B.getSharedMutex(), std::defer_lock);
  const std::scoped_lock lock(Alock, Block);

  std::map<int, std::string> aVals;
  std::map<int, std::string> bVals;
  {
    const std::scoped_lock lockSc(A.getInsertTruncMutex(), B.getInsertTruncMutex());
    aVals = A.getValues();
    bVals = B.getValues();
  }

  std::map<int, std::pair<std::string, std::string>> tmp;

  for(auto it = aVals.begin(); it != aVals.end(); it++)
  {
    auto bIt = bVals.find(it->first);
    if(bIt == bVals.end())
      tmp[it->first] = std::make_pair(it->second, "");
  }
  for(auto it = bVals.begin(); it != bVals.end(); it++)
  {
    auto aIt = aVals.find(it->first);
    if(aIt == aVals.end())
      tmp[it->first] = std::make_pair("", it->second);
  }

  for(auto it = tmp.begin(); it != tmp.end(); it++)
  {
    result << it->first << ", " << it->second.first << ", " << it->second.second <<"\n";
  }

  result <<"OK\n";
  errorMsg = result.str();
  return true;
}

bool database::exec(std::string_view cmd, std::string& resultMsg)
{
  boost::iostreams::stream<boost::iostreams::array_source> boostStream(cmd.data(), cmd.size());
  std::string str;
  std::vector<std::string> args;
  while(boostStream >> str)
    args.emplace_back(str);
  if(args.size() == 0)
  {
    resultMsg = "OK\n";
    return true;
  }
  const auto commandName = boost::to_upper_copy<std::string>(args.at(0));
  if(commandName == "INSERT")
  {
    if(args.size() < 4)
    {
      resultMsg = "ERR INSERT <Table> <Key> <Value>\n";
      return true;
    }
    std::string tableN = args.at(1);
    int key = std::stoi(args.at(2));
    std::string val = args.at(3);
    std::string curRes;
    if(insert(tableN, key, val, curRes))
      resultMsg = "OK\n";
    else
      resultMsg = "ERR " + curRes;
    return true;
  }
  else if(commandName == "TRUNCATE")
  {
    if(args.size() < 2)
    {
      resultMsg = "ERR TRUNCATE <Table> \n";
      return true;
    }
    std::string tableN = args.at(1);
    std::string curRes;
    if(truncate(tableN, curRes))
      resultMsg = "OK\n";
    else
      resultMsg = "ERR " + curRes;
    return true;
  }
  else if(commandName == "INTERSECTION")
  {
    std::string curRes;
    if(intersection(curRes))
      resultMsg = curRes;
    else
      resultMsg = "ERR " + curRes;
    return true;
  }
  else if(commandName == "SYMMETRIC_DIFFERENCE")
  {
    std::string curRes;
    if(symmetric_difference(curRes))
      resultMsg = curRes;
    else
      resultMsg = "ERR " + curRes;
    return true;
  }
  else
  {
    resultMsg = "ERR не найдена команда\n";
    return true;
  }
}
