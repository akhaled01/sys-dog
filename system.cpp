#include "header.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sstream>
#include <errno.h>
#include <numeric>
#include <thread>
#include <chrono>

/// @brief parses info from __cpuid and returns cpu brand string
/// @return cpu brand string
string CPUinfo()
{
  char CPUBrandString[0x40];
  unsigned int CPUInfo[4] = {0, 0, 0, 0};

  __cpuid(0x80000000, CPUInfo[0], CPUInfo[1], CPUInfo[2], CPUInfo[3]);
  unsigned int nExIds = CPUInfo[0];

  memset(CPUBrandString, 0, sizeof(CPUBrandString));

  for (unsigned int i = 0x80000000; i <= nExIds; ++i)
  {
    __cpuid(i, CPUInfo[0], CPUInfo[1], CPUInfo[2], CPUInfo[3]);

    if (i == 0x80000002)
      memcpy(CPUBrandString, CPUInfo, sizeof(CPUInfo));
    else if (i == 0x80000003)
      memcpy(CPUBrandString + 16, CPUInfo, sizeof(CPUInfo));
    else if (i == 0x80000004)
      memcpy(CPUBrandString + 32, CPUInfo, sizeof(CPUInfo));
  }
  string str(CPUBrandString);
  return str;
}

/// @brief implements Macros to return sys OS
/// @return system OS name
const char *getOsName()
{
#ifdef _WIN32
  return "Windows 32-bit";
#elif _WIN64
  return "Windows 64-bit";
#elif __APPLE__ || __MACH__
  return "Mac OSX";
#elif __linux__
  return "Linux";
#elif __FreeBSD__
  return "FreeBSD";
#elif __unix || __unix__
  return "Unix";
#else
  return "Other";
#endif
}

/// @brief returns hostname with dynamic malloc
/// @return hostname
char *hostname()
{
  size_t size = 255;
  char *a = (char *)malloc(size);
  if (a == NULL)
  {
    perror("malloc");
    return NULL;
  }

  if (gethostname(a, size) != 0)
  {
    perror("gethostname");
    free(a);
    return NULL;
  }

  return a;
}

/// @brief return total number of tasks/processes that are currently running, sleeping, uninterruptible, zombie, traced/stopped or interrupted.
/// @return total number of tasks
int getProcNum()
{
  const char *command = "ps aux --no-header | wc -l";
  FILE *fp = popen(command, "r");
  if (fp == NULL)
  {
    perror("popen");
    return -1;
  }

  char output[128];
  if (fgets(output, sizeof(output), fp) != NULL)
  {
    pclose(fp);
    return atoi(output) - 1;
  }
  else
  {
    pclose(fp);
    return -1;
  }
}

CpuUsage getCpuUsage()
{
  std::ifstream file("/proc/stat");
  std::string line;
  CpuUsage usage;

  if (std::getline(file, line))
  {
    istringstream ss(line);
    std::string cpu;
    ss >> cpu >> usage.user >> usage.nice >> usage.system >> usage.idle >> usage.iowait >> usage.irq >> usage.softirq >> usage.steal;
  }

  return usage;
}

double calcCPUUtil(const CpuUsage &prev, const CpuUsage &curr)
{
  unsigned long long prevIdle = prev.idle + prev.iowait;
  unsigned long long currIdle = curr.idle + curr.iowait;

  unsigned long long prevNonIdle = prev.user + prev.nice + prev.system + prev.irq + prev.softirq + prev.steal;
  unsigned long long currNonIdle = curr.user + curr.nice + curr.system + curr.irq + curr.softirq + curr.steal;

  unsigned long long prevTotal = prevIdle + prevNonIdle;
  unsigned long long currTotal = currIdle + currNonIdle;

  unsigned long long totalDiff = currTotal - prevTotal;
  unsigned long long idleDiff = currIdle - prevIdle;

  return (static_cast<double>(totalDiff - idleDiff) / totalDiff) * 100.0;
}

double getCurrentCpuUtil(float sleeptime)
{
  CpuUsage prevUsage = getCpuUsage();
  std::this_thread::sleep_for(std::chrono::duration<float>(sleeptime));
  CpuUsage currUsage = getCpuUsage();

  return calcCPUUtil(prevUsage, currUsage);
}

/// @brief gets cpu temp from `/sys/class/thermal/thermal_zone0`
/// @return cpu temp in degrees celsius
float getCPUTemp()
{
  FILE *pipe = popen("cat /sys/class/thermal/thermal_zone0/temp", "r");
  if (!pipe)
  {
    std::cerr << "Failed to open pipe." << std::endl;
    return -1.0f;
  }

  char buffer[128];
  std::string result = "";
  while (!feof(pipe))
  {
    if (fgets(buffer, 128, pipe) != nullptr)
      result += buffer;
  }
  pclose(pipe);

  // Convert the output to an integer and divide by 1000
  int temp = std::atoi(result.c_str());
  return static_cast<float>(temp) / 1000.0f;
}
