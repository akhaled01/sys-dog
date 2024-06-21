#include "header.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

/// @brief fmt bytes to required format
/// @param bytes the bytes needed
/// @return formatted bytes str
std::string fmtbytes(long long bytes)
{
  const int KB = 1024;
  const int MB = KB * 1024;
  const int GB = MB * 1024;

  std::ostringstream result;

  if (bytes >= GB)
  {
    result << std::fixed << setprecision(2) << static_cast<double>(bytes) / GB << " GB";
  }
  else if (bytes >= MB)
  {
    result << std::fixed << setprecision(2) << static_cast<double>(bytes) / MB << " MB";
  }
  else if (bytes >= KB)
  {
    result << std::fixed << setprecision(2) << static_cast<double>(bytes) / KB << " KB";
  }
  else
  {
    result << bytes << " bytes";
  }

  return result.str();
}

/// @brief simplifies `NetInterface` RX data to a presentable format
/// @return Array of `PresentableNetInterface`
std::vector<PresentableNetInterface> SimplifyNetDataRX()
{
  std::vector<PresentableNetInterface> presentableinterfaces;
  for (const auto &interface : ParseNetInterfaces())
  {
    PresentableNetInterface pinterface;
    pinterface.Name = interface.Name;
    pinterface.data.first = interface.recvStats.bytes;
    pinterface.data.second = fmtbytes(interface.recvStats.bytes);
    presentableinterfaces.push_back(pinterface);
  }
  return presentableinterfaces;
}

/// @brief simplifies `NetInterface` TX data to a presentable format
/// @return Array of `PresentableNetInterface`
std::vector<PresentableNetInterface> SimplifyNetDataTX()
{
  std::vector<PresentableNetInterface> presentableinterfaces;
  for (const auto &interface : ParseNetInterfaces())
  {
    PresentableNetInterface pinterface;
    pinterface.Name = interface.Name;
    pinterface.data.first = interface.transStats.bytes;
    pinterface.data.second = fmtbytes(interface.transStats.bytes);
    presentableinterfaces.push_back(pinterface);
  }
  return presentableinterfaces;
}

/// @brief Trim leading whitespaces from a string
/// @param str The input string
/// @return A new string with leading whitespaces removed
std::string ltrim(const std::string &str)
{
  std::size_t start = 0;
  while (start < str.size() && std::isspace(static_cast<unsigned char>(str[start])))
  {
    ++start;
  }
  return str.substr(start);
}
