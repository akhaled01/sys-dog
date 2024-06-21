#include "header.h"
#include <iostream>
#include <array>
#include <memory>
#include <stdexcept>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <cstdio>

// Utility function to split a string by whitespace
std::vector<std::string> split(const std::string &s)
{
  std::istringstream iss(s);
  return std::vector<std::string>((istream_iterator<std::string>(iss)), istream_iterator<std::string>());
}

/// @brief parses `free -g` command to get the total amount of memory, and amount of memory available
/// @return `pair<long, long> {totalMemory, usedMemory}`
std::pair<long, long> getMemUsage()
{
  std::array<char, 128> buffer;
  std::string result;
  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen("free -g", "r"), pclose);
  if (!pipe)
  {
    throw std::runtime_error("popen() failed!");
  }
  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
  {
    result += buffer.data();
  }

  std::istringstream iss(result);
  std::string line;
  long totalMemory = 0;
  long usedMemory = 0;

  // Skip the first line which is the header
  std::getline(iss, line);
  // The second line contains the memory information
  std::getline(iss, line);
  std::istringstream memoryLine(line);
  std::string temp;

  memoryLine >> temp >> totalMemory >> usedMemory;
  return {totalMemory, usedMemory};
}

/// @brief parses `free -g` command to get the total amount of SWAP, and amount of SWAP free
/// @return `pair<long, long> {totalMemory, usedMemory}`
std::pair<long, long> getSWAPUsage()
{
  std::array<char, 128> buffer;
  std::string result;
  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen("free -g", "r"), pclose);
  if (!pipe)
  {
    throw std::runtime_error("popen() failed!");
  }
  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
  {
    result += buffer.data();
  }

  std::istringstream iss(result);
  std::string line;
  long totalSWAP = 0;
  long freeSWAP = 0;

  // Skip the first line which is the header
  std::getline(iss, line);
  // Skip the second line which is the mem usage
  std::getline(iss, line);
  // get third line
  std::getline(iss, line);
  std::istringstream SWAPline(line);
  std::string temp;
  long usedSWAP = 0;

  SWAPline >> temp >> totalSWAP >> usedSWAP >> freeSWAP;
  return {totalSWAP, freeSWAP};
}

/// @brief Parses the output of `df -h /` command to get disk usage information
/// @return pair<long, long> {usedStorage, availableStorage}
std::pair<long, long> parseDiskUsage()
{
  std::array<char, 128> buffer;
  std::string result;
  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen("df -h /", "r"), pclose);
  if (!pipe)
  {
    throw std::runtime_error("popen() failed!");
  }
  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
  {
    result += buffer.data();
  }

  std::istringstream iss(result);
  std::string line;

  // Skip the first line which is the header
  std::getline(iss, line);

  // The second line contains the disk usage information
  std::getline(iss, line);

  std::istringstream diskLine(line);
  std::string filesystem, size, used, available, usagePercent;
  diskLine >> filesystem >> size >> used >> available >> usagePercent;

  // Convert used and available strings to long
  long usedStorage = std::stol(used);
  long totalStorage = std::stol(size);

  return {usedStorage, totalStorage};
}

/// @brief parses the `/proc fs` and gets required data
/// @return Vector array of the `ProcessInfo` struct
std::vector<ProcessInfo> parseProcessInfo()
{
  std::vector<ProcessInfo> processes;
  DIR *dir = opendir("/proc");
  struct dirent *entry;

  while ((entry = readdir(dir)) != nullptr)
  {
    if (entry->d_type == DT_DIR)
    {
      std::string pidStr = entry->d_name;
      if (std::all_of(pidStr.begin(), pidStr.end(), ::isdigit))
      {
        int pid = std::stoi(pidStr);
        std::string statusPath = "/proc/" + pidStr + "/status";
        std::ifstream statusFile(statusPath);
        if (statusFile.is_open())
        {
          ProcessInfo proc;
          proc.pid = pid;
          std::string line;
          while (std::getline(statusFile, line))
          {
            std::istringstream iss(line);
            std::string key;
            if (std::getline(iss, key, ':'))
            {
              std::string value;
              if (std::getline(iss, value))
              {
                value.erase(0, value.find_first_not_of("\t "));
                if (key == "Name")
                {
                  proc.name = value;
                }
                else if (key == "State")
                {
                  proc.state = value;
                }
                else if (key == "VmRSS")
                {
                  long memoryUsageKb = std::stol(value.substr(0, value.find(' ')));
                  struct sysinfo sysInfo;
                  sysinfo(&sysInfo);
                  long totalMemoryKb = sysInfo.totalram / 1024;
                  proc.memoryUsage = (double)memoryUsageKb / totalMemoryKb * 100;
                }
              }
            }
          }
          statusFile.close();

          std::string statPath = "/proc/" + pidStr + "/stat";
          std::ifstream statFile(statPath);
          if (statFile.is_open())
          {
            std::string statLine;
            std::getline(statFile, statLine);
            std::istringstream iss(statLine);
            std::vector<std::string> statFields;
            std::string field;
            while (iss >> field)
            {
              statFields.push_back(field);
            }
            long utime = std::stol(statFields[13]);
            long stime = std::stol(statFields[14]);
            long total_time = utime + stime;
            long hertz = sysconf(_SC_CLK_TCK);
            double total_time_sec = (double)total_time / hertz;

            long uptime;
            std::ifstream uptimeFile("/proc/uptime");
            if (uptimeFile.is_open())
            {
              uptimeFile >> uptime;
              uptimeFile.close();
            }

            double seconds = uptime - (std::stol(statFields[21]) / hertz);
            proc.cpuUsage = 100.0 * ((total_time_sec / seconds));

            statFile.close();
          }

          processes.push_back(proc);
        }
      }
    }
  }
  closedir(dir);
  return processes;
}
