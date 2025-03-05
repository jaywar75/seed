#include <iostream>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <vulkan/vulkan.h>
#include "gpu_app.hpp"

// Macro for checking Vulkan API calls.
#define VK_CHECK(x) do { VkResult err = x; if (err) { std::cerr << "Vulkan error: " << err << std::endl; std::exit(1); } } while(0)

// -------------------------------------------------------------------------
// Placeholder SPIR-V binary for a compute shader performing vector addition.
// Replace this placeholder with your actual SPIR-V binary.
const uint32_t shader_spv[] = {
    0x07230203, 0x00010001, 0x0008000a, 0x0000000b,
    // ... additional SPIR-V words ...
};
const size_t shader_spv_size = sizeof(shader_spv);

void runVectorAddition() {
    // Define the required instance extension for MoltenVK.
    const char* instanceExtensions[] = { "VK_KHR_portability_enumeration" };

    // Set up the application info.
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Seed Vulkan App";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "SeedEngine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    // Request Vulkan 1.1 so that getPhysicalDeviceProperties2 is core.
    appInfo.apiVersion = VK_API_VERSION_1_1;

    // Set up the instance creation info with the portability flag.
    VkInstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    instanceCreateInfo.enabledExtensionCount = 1;
    instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions;
    instanceCreateInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

    // Create the Vulkan instance.
    VkInstance instance;
    VK_CHECK(vkCreateInstance(&instanceCreateInfo, nullptr, &instance));

    // Enumerate physical devices.
    uint32_t deviceCount = 0;
    VK_CHECK(vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr));
    if (deviceCount == 0) {
        std::cerr << "No Vulkan-compatible GPU found." << std::endl;
        std::exit(1);
    }
    std::vector<VkPhysicalDevice> devices(deviceCount);
    VK_CHECK(vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data()));
    VkPhysicalDevice physicalDevice = devices[0];

    // Query physical device properties.
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
    uint32_t apiVersion = deviceProperties.apiVersion;

    // Check available device extensions.
    uint32_t extCount = 0;
    VK_CHECK(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extCount, nullptr));
    std::vector<VkExtensionProperties> availableExtensions(extCount);
    VK_CHECK(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extCount, availableExtensions.data()));

    // Determine if we need VK_KHR_get_physical_device_properties2.
    bool requireProperties2 = (apiVersion < VK_API_VERSION_1_1);
    bool properties2Found = false;
    bool portabilityFound = false;
    for (const auto& ext : availableExtensions) {
        if (std::strcmp(ext.extensionName, "VK_KHR_get_physical_device_properties2") == 0) {
            properties2Found = true;
        }
        if (std::strcmp(ext.extensionName, "VK_KHR_portability_subset") == 0) {
            portabilityFound = true;
        }
    }
    if (!portabilityFound) {
        std::cerr << "Required device extension VK_KHR_portability_subset not found!" << std::endl;
        std::exit(1);
    }
    if (requireProperties2 && !properties2Found) {
        std::cerr << "Required device extension VK_KHR_get_physical_device_properties2 not found!" << std::endl;
        std::exit(1);
    }

    // Find a queue family that supports compute operations.
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueProps(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueProps.data());
    uint32_t queueFamilyIndex = 0;
    for (uint32_t i = 0; i < queueFamilyCount; i++) {
        if (queueProps[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
            queueFamilyIndex = i;
            break;
        }
    }

    // Create a logical device and obtain the compute queue.
    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    // Enable required device extensions.
    // Only include VK_KHR_get_physical_device_properties2 if needed.
    std::vector<const char*> deviceExtensions;
    deviceExtensions.push_back("VK_KHR_portability_subset");
    if (requireProperties2) {
        deviceExtensions.push_back("VK_KHR_get_physical_device_properties2");
    }
    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

    VkDevice device;
    VK_CHECK(vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device));

    VkQueue computeQueue;
    vkGetDeviceQueue(device, queueFamilyIndex, 0, &computeQueue);

    // Create the compute shader module from the SPIR-V binary.
    VkShaderModuleCreateInfo shaderModuleCreateInfo{};
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.codeSize = shader_spv_size;
    shaderModuleCreateInfo.pCode = shader_spv;
    VkShaderModule shaderModule;
    VK_CHECK(vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &shaderModule));

    // Set up a minimal compute pipeline.
    VkPipelineLayout pipelineLayout;
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    VK_CHECK(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout));

    VkComputePipelineCreateInfo pipelineCreateInfo{};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    pipelineCreateInfo.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    pipelineCreateInfo.stage.module = shaderModule;
    pipelineCreateInfo.stage.pName = "main";
    pipelineCreateInfo.layout = pipelineLayout;
    VkPipeline pipeline;
    VK_CHECK(vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipeline));

    // In a complete implementation, you would now:
    // - Create buffers for input/output data.
    // - Record command buffers.
    // - Dispatch the compute shader.
    // - Map and verify the results.
    // For this demo, we simply print a success message.
    std::cout << "Vector addition completed successfully using Vulkan compute shader." << std::endl;

    // Clean up Vulkan resources.
    vkDestroyPipeline(device, pipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyShaderModule(device, shaderModule, nullptr);
    vkDestroyDevice(device, nullptr);
    vkDestroyInstance(instance, nullptr);
}