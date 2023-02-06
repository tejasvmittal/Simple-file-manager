#include "cacheCDN.h"

#include <fstream>
#include <iomanip>
#include <map>
#include <sstream>
#include <string>
using namespace std;
using namespace CACHE::CDN;

Cache::Cache(unsigned int freshnessCount)
    : freshnessCount(freshnessCount), disk_reads(0) {}

Cache::~Cache() {
  for (auto p : textfilename_and_content) {
    delete p.second;
  }
}

std::string Cache::getText(std::string filepath) {
  auto entry = textfilename_and_content.find(filepath);
  if (entry == textfilename_and_content.end()) {
    filename_and_freshness[filepath] = 0;
    textfilename_and_content[filepath] = new string;
    filename_and_requests[filepath] = 0;
  }
  if (filename_and_freshness[filepath] == 0) {
    disk_reads += 1;
    ifstream inFile(filepath);
    string text_file;
    *textfilename_and_content[filepath] = "";
    while (getline(inFile, text_file)) {
      *textfilename_and_content[filepath] += text_file + "\n";
    }
    inFile.close();
    if (freshnessCount != 0)
      filename_and_freshness[filepath] = freshnessCount - 1;
    else
      filename_and_freshness[filepath] = freshnessCount;
  } else {
    filename_and_freshness[filepath] -= 1;
  }
  filename_and_requests[filepath] += 1;
  return *textfilename_and_content[filepath];
}

char* Cache::getBinary(std::string filepath) {
  auto entry = binaryfilename_and_content.find(filepath);
  streampos size;
  if (entry == binaryfilename_and_content.end()) {
    ifstream inFile(filepath, ios::binary | ios::in | ios::ate);
    if (inFile.is_open()) {
      size = inFile.tellg();
      filename_and_freshness[filepath] = 0;
      binaryfilename_and_content[filepath] = new char[size];
      filename_and_requests[filepath] = 0;
    }
    inFile.close();
  }

  if (filename_and_freshness[filepath] == 0) {
    ifstream inFile(filepath, ios::binary | ios::in | ios::ate);
    if (inFile.is_open()) {
      disk_reads += 1;
      size = inFile.tellg();
      binaryfilename_and_content[filepath] = new char[size];
      inFile.seekg(0, ios::beg);
      inFile.read(binaryfilename_and_content[filepath], size);
      inFile.close();
      if (freshnessCount != 0)
        filename_and_freshness[filepath] = freshnessCount - 1;
      else
        filename_and_freshness[filepath] = freshnessCount;
    }
  } else {
    filename_and_freshness[filepath] -= 1;
  }
  filename_and_requests[filepath] += 1;
  return binaryfilename_and_content[filepath];
}

bool Cache::isCached(std::string filepath) {
  auto textentry = textfilename_and_content.find(filepath);
  auto binaryentry = binaryfilename_and_content.find(filepath);
  return (textentry != textfilename_and_content.end() ||
          binaryentry != binaryfilename_and_content.end());
}

unsigned int Cache::getFreshness(std::string filepath) {
  auto entry = filename_and_freshness.find(filepath);
  if (entry == filename_and_freshness.end()) {
    return 0;
  } else {
    return filename_and_freshness[filepath];
  }
}

void Cache::markFileFresh(std::string filepath) {
  auto entry = filename_and_freshness.find(filepath);
  if (entry != filename_and_freshness.end()) {
    filename_and_freshness[filepath] = freshnessCount;
  }
}

void Cache::purgeCache() {
  for (auto p : textfilename_and_content) {
    delete p.second;
  }
  for (auto p = textfilename_and_content.begin();
       p != textfilename_and_content.end(); ++p) {
    p->second = new string;
  }
  for (auto p = binaryfilename_and_content.begin();
       p != binaryfilename_and_content.end(); ++p) {
    p->second = nullptr;
  }
  for (auto p = filename_and_freshness.begin();
       p != filename_and_freshness.end(); ++p) {
    p->second = 0;
  }
}

std::string Cache::topFile() {
  string filename = "";
  unsigned int request = 0;
  for (auto p : filename_and_requests) {
    if (request < p.second) {
      filename = p.first;
      request = p.second;
    }
  }
  return filename;
}

std::string Cache::getStats() {
  stringstream s;
  unsigned int requests = 0;
  unsigned int files_requested = 0;

  for (auto p : filename_and_requests) {
    requests += p.second;
  }
  for (auto p : filename_and_requests) {
    if (p.second > 0) {
      files_requested += 1;
    }
  }
  s << "Cache Stats\n"
    << "----------\n";
  s << "Total requests: " << requests << "\n";
  s << "Total files requested: " << files_requested << "\n";
  if (files_requested == 0) {
    s << "Average requests per file: " << fixed << setprecision(2) << 0.00
      << "\n";
  } else {
    s << "Average requests per file: " << fixed << setprecision(2)
      << ((double)requests / (double)files_requested) << "\n";
  }
  s << "Top file: " << Cache::topFile() << " ("
    << filename_and_requests[Cache::topFile()] << " requests)"
    << "\n";
  s << "Total times read from disk: " << disk_reads << "\n";
  s << "----------\n";
  return s.str();
}
