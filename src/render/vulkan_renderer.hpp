#include <vector>
#include <vulkan/vulkan.hpp>
#include "../common/types.hpp"

namespace tde {

class platform;

class vulkan_renderer
{
public:
    explicit vulkan_renderer(platform* platform);
    ~vulkan_renderer();

private:
    platform* _platform;
};

} // namespace tde
