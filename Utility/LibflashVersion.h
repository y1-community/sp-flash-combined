#ifndef LIBFLASHVERSION_H
#define LIBFLASHVERSION_H

#include <string>
#include <vector>

// Single owner for the libflash version target: parse and compare
// releases, list what is installed, resolve a request. Empty resolves to
// the highest release; unmatched requests are invalid and fail loudly.
struct LibflashVersion {
    int parts[4] = {0, 0, 0, 0};
    int count = 0;
    static LibflashVersion parse(const std::string &text);
    bool isValid() const { return count > 0; }
    std::string toString() const;
    bool operator<(const LibflashVersion &other) const;
    bool operator==(const LibflashVersion &other) const;
};

class LibflashVersionTarget
{
public:
    // Installed releases, highest first. Today that is the single linked
    // engine; the list grows when side-by-side binaries are packaged.
    static std::vector<LibflashVersion> availableVersions();
    static LibflashVersion resolve(const std::string &requested);
    // resolve() as a recordable string; empty when unmatched.
    static std::string resolveToString(const std::string &requested);
};

#endif // LIBFLASHVERSION_H
