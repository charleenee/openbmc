// Copyright 2022-present Facebook. All Rights Reserved.
#include <sstream>
#include <sys/stat.h>
#if (__GNUC__ < 8)
#include <experimental/filesystem>
namespace std {
namespace filesystem = experimental::filesystem;
}
#else
#include <filesystem>
#endif

namespace rackmon {
class TempDirectory {
  std::string path_{};
  public:
  TempDirectory() {
    char mkdir_template[] = "/tmp/rackmonXXXXXX";
    path_.assign(mkdtemp(mkdir_template));
  }
  ~TempDirectory() {
    std::filesystem::remove_all(path_);
  }
  const std::string& path() const {
    return path_;
  }
};
} // namespace rackmon;
