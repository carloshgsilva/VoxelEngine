#pragma once

#include <vulkan/vulkan.hpp>

namespace GraphicsUtil {
	static bool DoesFormatContainsDepth(vk::Format Format) {
        return
            Format == vk::Format::eD16Unorm ||
            Format == vk::Format::eD16UnormS8Uint ||
            Format == vk::Format::eD24UnormS8Uint ||
            Format == vk::Format::eD32Sfloat ||
            Format == vk::Format::eD32SfloatS8Uint;
	}

    static bool DoesFormatContainsStencil(vk::Format Format) {
        return
            Format == vk::Format::eD16UnormS8Uint ||
            Format == vk::Format::eD24UnormS8Uint ||
            Format == vk::Format::eD32SfloatS8Uint;
    }
}