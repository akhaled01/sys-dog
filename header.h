// To make sure you don't declare the function more than once by including the header multiple times.
#ifndef header_H
#define header_H

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <dirent.h>
#include <vector>
#include <iostream>
#include <cmath>
// lib to read from file
#include <fstream>
// for the name of the computer and the logged in user
#include <unistd.h>
#include <limits.h>
// this is for us to get the cpu information
// mostly in unix system
// not sure if it will work in windows
#include <cpuid.h>
// this is for the memory usage and other memory visualization
// for linux gotta find a way for windows
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <sys/statvfs.h>
// for time and date
#include <ctime>
// ifconfig ip addresses
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <map>

using namespace std;

/// @brief representation of proccesses from the `/proc` fs
struct ProcessInfo
{
  int pid;
  std::string name;
  std::string state;
  double cpuUsage;
  double memoryUsage;
};

struct IP4
{
  std::string name;
  char addressBuffer[INET_ADDRSTRLEN];
};

/// @brief representation of the `tx` data from `/proc/net/dev`
struct RX
{
  long long bytes;
  int packets;
  int errs;
  int drop;
  int fifo;
  int frame;
  int compressed;
  int multicast;
};

/// @brief representation of the `rx` data from `/proc/net/dev`
struct TX
{
  long long bytes;
  int packets;
  int errs;
  int drop;
  int fifo;
  int colls;
  int carrier;
  int compressed;
};

// a low level representation of an interface
struct NetInterface
{
  std::string Name;
  RX recvStats;
  TX transStats;
};

/// @brief presentable interface format
struct PresentableNetInterface
{
  std::string Name;
  std::pair<long long, std::string> data;
};

string CPUinfo();
const char *getOsName();
char *hostname();
int getProcNum();
float getCPUUtil();
float getCPUTemp();

std::pair<long, long> getMemUsage();
std::pair<long, long> getSWAPUsage();
std::pair<long, long> parseDiskUsage();
std::vector<ProcessInfo> parseProcessInfo();

std::vector<NetInterface> ParseNetInterfaces();
std::vector<IP4> GetIPv4Addrs();

std::vector<PresentableNetInterface> SimplifyNetDataRX();
std::vector<PresentableNetInterface> SimplifyNetDataTX();
std::string fmtbytes(long long bytes);
std::string ltrim(const std::string &str);

#endif
