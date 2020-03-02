#include <fstream>

#include <banner/util/debug.hpp>
#include <banner/util/file.hpp>

namespace ban {
std::vector<char> read_bytes_from_file(const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        debug::err("Failed to read shader file: %s", filename.c_str());
    }

    const size_t file_size = file.tellg();
    std::vector<char> buffer(file_size);
    file.seekg(0, std::ios::beg);
    file.read(buffer.data(), file_size);
    file.close();

    return buffer;
}
} // namespace ban