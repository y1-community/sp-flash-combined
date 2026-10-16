#include "LibflashVersion.h"

#include <algorithm>
#include <cctype>
#include <sstream>

#include "version.h"

namespace {

std::string trim(const std::string &text)
{
    std::string::size_type first = 0;
    while (first < text.size() && std::isspace((unsigned char)text[first])) {
        ++first;
    }
    std::string::size_type last = text.size();
    while (last > first && std::isspace((unsigned char)text[last - 1])) {
        --last;
    }
    return text.substr(first, last - first);
}

bool isNumericField(const std::string &field)
{
    if (field.empty()) {
        return false;
    }
    for (std::string::size_type i = 0; i < field.size(); ++i) {
        if (!std::isdigit((unsigned char)field[i])) {
            return false;
        }
    }
    return true;
}

} // namespace

LibflashVersion LibflashVersion::parse(const std::string &text)
{
    LibflashVersion version;
    std::string item;
    std::istringstream stream(trim(text));
    while (std::getline(stream, item, '.') && version.count < 4) {
        if (!isNumericField(item)) {
            break;
        }
        version.parts[version.count++] = std::atoi(item.c_str());
    }
    return version;
}

std::string LibflashVersion::toString() const
{
    if (!isValid()) {
        return std::string();
    }
    int shown = count;
    while (shown > 2 && parts[shown - 1] == 0) {
        --shown;
    }
    std::ostringstream stream;
    stream << parts[0];
    for (int i = 1; i < shown; ++i) {
        stream << '.' << parts[i];
    }
    return stream.str();
}

bool LibflashVersion::operator<(const LibflashVersion &other) const
{
    for (int i = 0; i < 4; ++i) {
        if (parts[i] != other.parts[i]) {
            return parts[i] < other.parts[i];
        }
    }
    return false;
}

bool LibflashVersion::operator==(const LibflashVersion &other) const
{
    return !(*this < other) && !(other < *this);
}

std::vector<LibflashVersion> LibflashVersionTarget::availableVersions()
{
    std::vector<LibflashVersion> versions;
    const LibflashVersion built_in = LibflashVersion::parse(ToolInfo::VersionNum());
    if (built_in.isValid()) {
        versions.push_back(built_in);
    }
    std::sort(versions.begin(), versions.end());
    std::reverse(versions.begin(), versions.end());
    return versions;
}

std::string LibflashVersionTarget::resolveToString(const std::string &requested)
{
    return resolve(requested).toString();
}

LibflashVersion LibflashVersionTarget::resolve(const std::string &requested)
{
    const std::vector<LibflashVersion> versions = availableVersions();
    if (versions.empty()) {
        return LibflashVersion();
    }
    if (trim(requested).empty()) {
        return versions.front();
    }
    const LibflashVersion want = LibflashVersion::parse(requested);
    if (!want.isValid()) {
        return LibflashVersion();
    }
    for (std::vector<LibflashVersion>::const_iterator it = versions.begin();
         it != versions.end(); ++it) {
        bool match = true;
        for (int i = 0; match && i < want.count; ++i) {
            match = (it->parts[i] == want.parts[i]);
        }
        if (match) {
            return *it;
        }
    }
    return LibflashVersion();
}
