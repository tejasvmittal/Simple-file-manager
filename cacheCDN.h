#ifndef __CACHECDN_H__
#define __CACHECDN_H__

#include <map>
#include <string>
namespace CACHE {
namespace CDN {

class Cache {
 private:
  unsigned int freshnessCount;
  std::map<std::string, unsigned int> filename_and_freshness;
  std::map<std::string, std::string*> textfilename_and_content;
  std::map<std::string, char*> binaryfilename_and_content;
  std::map<std::string, unsigned int> filename_and_requests;
  unsigned int disk_reads;

 public:
  explicit Cache(unsigned int freshnessCount);
  ~Cache();
  std::string getText(std::string filepath);
  char* getBinary(std::string filepath);
  bool isCached(std::string filepath);
  unsigned int getFreshness(std::string filepath);
  void markFileFresh(std::string filepath);
  void purgeCache();
  std::string topFile();
  std::string getStats();
};

}  // namespace CDN
}  // namespace CACHE

#endif  // __CACHECDN_H__
