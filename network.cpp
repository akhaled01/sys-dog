#include "header.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <cstring>
#include <netinet/in.h>

/// @brief parses `/proc/net/dev` to get all network interfaces and return an appropriate struct
/// @return array of interfaces
std::vector<NetInterface> ParseNetInterfaces()
{
  std::vector<NetInterface> interfaces;
  std::ifstream file("/proc/net/dev");
  if (!file.is_open())
  {
    std::cerr << "Error opening /proc/net/dev\n";
    return interfaces;
  }

  std::string line;
  // Skip the first two lines since they are headers
  std::getline(file, line);
  std::getline(file, line);

  while (std::getline(file, line))
  {
    std::istringstream iss(line);
    std::string interfaceName;
    std::getline(iss, interfaceName, ':');
    interfaceName.erase(interfaceName.find_last_not_of(' ') + 1);

    NetInterface netInterface;
    netInterface.Name = interfaceName;

    // Parse the RX stats
    iss >> netInterface.recvStats.bytes >> netInterface.recvStats.packets >> netInterface.recvStats.errs >> netInterface.recvStats.drop >> netInterface.recvStats.fifo >> netInterface.recvStats.frame >> netInterface.recvStats.compressed >> netInterface.recvStats.multicast;

    // Parse the TX stats
    iss >> netInterface.transStats.bytes >> netInterface.transStats.packets >> netInterface.transStats.errs >> netInterface.transStats.drop >> netInterface.transStats.fifo >> netInterface.transStats.colls >> netInterface.transStats.carrier >> netInterface.transStats.compressed;

    interfaces.push_back(netInterface);
  }

  file.close();
  return interfaces;
}

/// @brief parses the arp table of the device from `/proc/net/arp` to get interfaces and their IPs
/// @return arr of the `IP4` struct
std::vector<IP4> GetIPv4Addrs()
{
  std::vector<IP4> interfaces;
  std::ifstream file("/proc/net/arp");
  if (!file.is_open())
  {
    std::cerr << "Error opening /proc/net/arp\n";
    return interfaces;
  }

  std::string line;
  // Skip the first line (headers)
  std::getline(file, line);

  while (std::getline(file, line))
  {
    IP4 inter;
    std::istringstream iss(line);
    std::string ipAddress, hwType, flags, hwAddress, mask, device;

    if (!(iss >> ipAddress >> hwType >> flags >> hwAddress >> mask >> device))
    {
      std::cerr << "Error parsing line: " << line << std::endl;
      continue;
    }

    inter.name = device;
    std::strncpy(inter.addressBuffer, ipAddress.c_str(), INET_ADDRSTRLEN - 1);
    inter.addressBuffer[INET_ADDRSTRLEN - 1] = '\0';

    interfaces.push_back(inter);
  }

  file.close();
  return interfaces;
}
