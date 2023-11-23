#pragma once

#include "Device.hpp"

namespace engine {

    struct TextureInfo{
        VkImageView imageView;
        VkSampler sampler;
    };

    class Image {
    public:
        Image(Device& device, const std::string& filePath);
        ~Image();

        // Not copyable or movable
        Image(const Image&) = delete;
        Image& operator=(const Image&) = delete;
        Image(Image&&) = delete;
        Image& operator=(Image&&) = delete;

        void createTextureImage(const std::string& filePath);

        VkImage image() { return m_image; }
        TextureInfo textureInfo() { return m_textureInfo; }
        static VkImageView  createImageView(Device& device, VkImage image, VkFormat format);

    private:
        void createImage(
            uint32_t width, 
            uint32_t height, 
            VkFormat format, 
            VkImageTiling tiling, 
            VkImageUsageFlags usage, 
            VkMemoryPropertyFlags properties);
        void createTextureImageView();
        void transitionImageLayout(VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
        void createTextureSampler();

        Device& m_device;
        VkImage m_image;
        TextureInfo m_textureInfo;
        VkDeviceMemory m_imageMemory;
    };

}  // namespace engine
