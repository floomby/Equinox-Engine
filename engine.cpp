// This is basically the "vulkan" file plus the "glfw" file ie. most of the rendering and input handling
// definately not good organization

#define LOG_GUI_TEXTURES

#ifndef DONT_PRINT_MEMORY_LOG
#define VMA_DEBUG_LOG(format, ...) do { \
    printf(format, ##__VA_ARGS__); \
    printf("\n"); \
} while(false)
#endif

#define VMA_IMPLEMENTATION
#include "libs/vk_mem_alloc.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "libs/stb_image_write.h"
#include "libs/stb_image.h"

#include "econf.h"

#include "engine.hpp"
#include "entity.hpp"
#include "sound.hpp"

#include <codecvt>
#include <chrono>
#include <csignal>
#include <cstring>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <iterator>
#include <locale>
#include <numeric>
#include <map>
#include <sstream>
#include <thread>

#define BOOST_STACKTRACE_USE_ADDR2LINE
#include <boost/stacktrace.hpp>

#ifdef LOG_GUI_TEXTURES
static std::ofstream textureLog("textureLog", std::ofstream::out);
#endif

// void vkDestroyImageViewLog(VkDevice device, VkImageView imageView, const VkAllocationCallbacks *pAllocator) {
//     std::ostringstream ss;
//     ss << std::hex << "Destroying " << imageView << " " << std::this_thread::get_id() << std::endl;
//     ss << boost::stacktrace::stacktrace() << std::endl;
//     textureLog << ss.str();
//     vkDestroyImageView(device, imageView, pAllocator);
// }

// The memory from the debugging is not cleaned up correctly
// Debuging stuff
// #define DEPTH_DEBUG_IMAGE_USAGE VK_IMAGE_USAGE_TRANSFER_SRC_BIT
#define DEPTH_DEBUG_IMAGE_USAGE 0

// TODO, do this same thing for vma errors as well
static std::map<int, const char *> vulkanErrors = {
    { VK_SUCCESS, "VK_SUCCESS" },
    { VK_NOT_READY, "VK_NOT_READY" },
    { VK_TIMEOUT, "VK_TIMEOUT" },
    { VK_EVENT_SET, "VK_EVENT_SET" },
    { VK_EVENT_RESET, "VK_EVENT_RESET" },
    { VK_INCOMPLETE, "VK_INCOMPLETE" },
    { VK_ERROR_OUT_OF_HOST_MEMORY, "VK_ERROR_OUT_OF_HOST_MEMORY" },
    { VK_ERROR_OUT_OF_DEVICE_MEMORY, "VK_ERROR_OUT_OF_DEVICE_MEMORY" },
    { VK_ERROR_INITIALIZATION_FAILED, "VK_ERROR_INITIALIZATION_FAILED" },
    { VK_ERROR_DEVICE_LOST, "VK_ERROR_DEVICE_LOST" },
    { VK_ERROR_MEMORY_MAP_FAILED, "VK_ERROR_MEMORY_MAP_FAILED" },
    { VK_ERROR_LAYER_NOT_PRESENT, "VK_ERROR_LAYER_NOT_PRESENT" },
    { VK_ERROR_EXTENSION_NOT_PRESENT, "VK_ERROR_EXTENSION_NOT_PRESENT" },
    { VK_ERROR_FEATURE_NOT_PRESENT, "VK_ERROR_FEATURE_NOT_PRESENT" },
    { VK_ERROR_INCOMPATIBLE_DRIVER, "VK_ERROR_INCOMPATIBLE_DRIVER" },
    { VK_ERROR_TOO_MANY_OBJECTS, "VK_ERROR_TOO_MANY_OBJECTS" },
    { VK_ERROR_FORMAT_NOT_SUPPORTED, "VK_ERROR_FORMAT_NOT_SUPPORTED" },
    { VK_ERROR_FRAGMENTED_POOL, "VK_ERROR_FRAGMENTED_POOL" },
    { VK_ERROR_UNKNOWN, "VK_ERROR_UNKNOWN" },
    { VK_ERROR_OUT_OF_POOL_MEMORY, "VK_ERROR_OUT_OF_POOL_MEMORY" },
    { VK_ERROR_INVALID_EXTERNAL_HANDLE, "VK_ERROR_INVALID_EXTERNAL_HANDLE" },
    { VK_ERROR_FRAGMENTATION, "VK_ERROR_FRAGMENTATION" },
    { VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS, "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS" },
    { VK_ERROR_SURFACE_LOST_KHR, "VK_ERROR_SURFACE_LOST_KHR" },
    { VK_ERROR_NATIVE_WINDOW_IN_USE_KHR, "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR" },
    { VK_SUBOPTIMAL_KHR, "VK_SUBOPTIMAL_KHR" },
    { VK_ERROR_OUT_OF_DATE_KHR, "VK_ERROR_OUT_OF_DATE_KHR" },
    { VK_ERROR_INCOMPATIBLE_DISPLAY_KHR, "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR" },
    { VK_ERROR_VALIDATION_FAILED_EXT, "VK_ERROR_VALIDATION_FAILED_EXT" },
    { VK_ERROR_INVALID_SHADER_NV, "VK_ERROR_INVALID_SHADER_NV" },
    { VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT, "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT" },
    { VK_ERROR_NOT_PERMITTED_EXT, "VK_ERROR_NOT_PERMITTED_EXT" },
    { VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT, "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT" },
    { VK_THREAD_IDLE_KHR, "VK_THREAD_IDLE_KHR" },
    { VK_THREAD_DONE_KHR, "VK_THREAD_DONE_KHR" },
    { VK_OPERATION_DEFERRED_KHR, "VK_OPERATION_DEFERRED_KHR" },
    { VK_OPERATION_NOT_DEFERRED_KHR, "VK_OPERATION_NOT_DEFERRED_KHR" },
    { VK_PIPELINE_COMPILE_REQUIRED_EXT, "VK_PIPELINE_COMPILE_REQUIRED_EXT" },
    { VK_ERROR_OUT_OF_POOL_MEMORY_KHR, "VK_ERROR_OUT_OF_POOL_MEMORY_KHR" },
    { VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR, "VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR" },
    { VK_ERROR_FRAGMENTATION_EXT, "VK_ERROR_FRAGMENTATION_EXT" },
    { VK_ERROR_INVALID_DEVICE_ADDRESS_EXT, "VK_ERROR_INVALID_DEVICE_ADDRESS_EXT" },
    { VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR, "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR" },
    { VK_ERROR_PIPELINE_COMPILE_REQUIRED_EXT, "VK_ERROR_PIPELINE_COMPILE_REQUIRED_EXT" }
};

void vulkanErrorGuard_(VkResult result, const char *errorMessage, const char *file, int line) {
    if (result != VK_SUCCESS) {
        std::stringstream message;
        auto errorName = vulkanErrors.find(result);
        if (errorName == vulkanErrors.end()) {
            message << errorMessage << "\n\tUnknow vulkan error: " << std::dec << result << "   " << file << ":" << line << std::endl;
        } else {
            message << errorMessage << "\n\t" << "Vulkan error: " << std::dec << result << "   " << file << ":" << line << std::endl;
        }
        throw std::runtime_error(message.str());
    }
}

#define vulkanErrorGuard(result, message) vulkanErrorGuard_(result, message, __FILE__, __LINE__)

std::vector<char> Utilities::readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        std::stringstream msg;
        msg << "Failed to open file: " << filename;
        throw std::runtime_error(msg.str());
    }

    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();
    return buffer;
}

static VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger) {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        else return VK_ERROR_EXTENSION_NOT_PRESENT;
}

static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) func(instance, debugMessenger, pAllocator);
}

static const std::vector<const char*> requiredDeviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    // VK_NV_FRAMEBUFFER_MIXED_SAMPLES_EXTENSION_NAME,
    VK_KHR_MAINTENANCE3_EXTENSION_NAME,
    VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
    // My card does not support cubic filtering??? (I am in disbelief, but we can do it in the fragment shader anyways)
    // VK_EXT_FILTER_CUBIC_EXTENSION_NAME
    // I really want to use this, but I can't because nvidia drivers don't support it
    // VK_EXT_LINE_RASTERIZATION_EXTENSION_NAME
};

static const std::vector<const char*> requiredInstanceExtensions = {
    VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
};

Engine::Engine(EngineSettings engineSettings)
: client(std::make_shared<Networking::Client>(net.io, "127.0.0.1", Config::portStr, &authState)) {
    this->engineSettings = engineSettings;
    Api::context = this;
}

void Engine::send(const ApiProtocol& data) {
    client->writeData(data);
}

void Engine::send(ApiProtocol&& data) {
    client->writeData(data);
}

void Engine::quit() {
    drawTooltip = false;
    glfwSetWindowShouldClose(window, true);
}

std::set<std::string> Engine::getSupportedInstanceExtensions() {
    uint32_t count;
    vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
    std::vector<VkExtensionProperties> extensions(count);
    vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data());
    std::set<std::string> results;

    for (auto & extension : extensions) {
        results.insert(extension.extensionName);
    }

    return results;
}

void Engine::initWidow()
{
    if (engineSettings.extremelyVerbose) {
        std::cout << "Supported extensions:" << std::endl;
        for(const auto& name : getSupportedInstanceExtensions()) {
            std::cout << "\t" << name << std::endl;
        }
    }

    // TODO Get the monitor size and scalling from glfw to help for deciding how big to make the text
    glfwInit();
    GuiTextures::initFreetype2(this);
    createCursors();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window = glfwCreateWindow(engineSettings.width, engineSettings.height, "Vulkan Toy", nullptr, nullptr);
    framebufferSize.width = engineSettings.width;
    framebufferSize.height = engineSettings.height;
    glfwSetWindowUserPointer(window, this);

    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    for (int i = 0; i < 8; i++) mouseButtonsPressed[i] = false;
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    scrollAmount = 0.0f;
    glfwSetScrollCallback(window, scrollCallback);
    memset(&keysPressed, 0, sizeof(keysPressed));
    glfwSetKeyCallback(window, keyCallback);
    glfwSetCharCallback(window, charCallback);
}

void Engine::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto engine = reinterpret_cast<Engine *>(glfwGetWindowUserPointer(window));
    engine->framebufferResized = true;
    engine->framebufferSize.width = width;
    engine->framebufferSize.height = height;
    // std::cout << "Thread resize callback run on thred: " << gettid() << std::endl;
    if (engine->gui) {
        engine->guiOutOfDate = true; // dont draw the gui until after it has been rebuilt
        GuiCommandData *what = new GuiCommandData();
        // what->childIndices.push(0); // Don't actually push anything rn since we have no root node by default
        what->size.height.asInt = height;
        what->size.width.asInt = width;
        engine->gui->submitCommand({ Gui::GUI_RESIZE, what });
    }
}

void Engine::mouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
    auto engine = reinterpret_cast<Engine *>(glfwGetWindowUserPointer(window));
    engine->mouseButtonsPressed[button] = action == GLFW_PRESS;
    double x, y;
    glfwGetCursorPos(engine->window, &x, &y);
    engine->mouseInput.push({ action, button, mods, (float)x, (float)y });
    // I guess the tids are the same as the engine main thread and we can just do this???
    // Glfw docs have me confoozzled in this reguard (entirely possible I just can't read)
    // std::cout << "Mouse handler tid: " << gettid() << std::endl;
    if (action == GLFW_PRESS) {
        auto what = new GuiCommandData();
        what->id = engine->gui->pushConstant.guiID;
        what->position.x.asFloat = engine->lastMousePosition.normedX;
        what->position.y.asFloat = engine->lastMousePosition.normedY;
        what->flags = mods;
        engine->gui->submitCommand({ Gui::GUI_CLICK, what });
    }
    if (engine->canAttack) {
        auto mode = InsertionMode::OVERWRITE;
        if (mods & GLFW_MOD_SHIFT) mode = InsertionMode::BACK;
        if (mods & GLFW_MOD_ALT) mode = InsertionMode::FRONT;
        engine->apiLocks[APIL_SELECTION].lock();
        for (auto id : engine->idsSelected)
            Api::cmd_setTargetID(id, engine->mousedOverId, mode, true);
        engine->apiLocks[APIL_SELECTION].unlock();
    }
}

void Engine::scrollCallback(GLFWwindow *window, double xoffset, double yoffset) {
    auto engine = reinterpret_cast<Engine *>(glfwGetWindowUserPointer(window));
    engine->scrollAmount -= yoffset;
}

void Engine::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    auto engine = reinterpret_cast<Engine *>(glfwGetWindowUserPointer(window));
    if (key == GLFW_KEY_UNKNOWN) {
        std::cerr << "Unknown key (ignoring)" << std::endl;
        return;
    }
    engine->keyboardInput.push({ action, key, mods });
    if (engine->keyboardCaptured &&
        (key == GLFW_KEY_ENTER || key == GLFW_KEY_BACKSPACE) &&
        (action == GLFW_REPEAT || action == GLFW_PRESS)) {
        auto what = new GuiCommandData();
        what->action = key;
        engine->gui->submitCommand({ Gui::GUI_KEY_INPUT, what });
    }
}

void Engine::charCallback(GLFWwindow *window, unsigned int codepoint) {
    auto engine = reinterpret_cast<Engine *>(glfwGetWindowUserPointer(window));
    if (engine->keyboardCaptured) {
        auto what = new GuiCommandData();
        what->action = codepoint;
        engine->gui->submitCommand({ Gui::GUI_CODEPOINT_INPUT, what });
    }
}

std::vector<const char *> Engine::getUnsupportedLayers() {
    std::vector<const char *> ret;

    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    // if (engineSettings.extremelyVerbose) {
        std::cout << "Supported layers:" << std::endl;
        for(const auto& layer : availableLayers) {
            std::cout << "\t" << layer.layerName << std::endl;
        }
    // }

    for (const char* layerName: engineSettings.validationLayers) {
        bool foundLayer = false;

        for(const auto& layerProperties : availableLayers) {
            if (!strcmp(layerName, layerProperties.layerName)) {
                foundLayer = true;
                break;
            }
        }
        if (!foundLayer) ret.push_back(layerName);
    }
    return ret;
}

void Engine::enableValidationLayers() {
    if (engineSettings.validationLayers.size()) {
        auto unsupportedLayers = getUnsupportedLayers();
        if (unsupportedLayers.size() > 0) {
            std::stringstream msg;
            msg << "Requested layers (" << engineSettings.validationLayers.size() << "):" << std::endl;
            for (const auto& layerName : engineSettings.validationLayers) msg << '\t' << layerName << std::endl;
            msg << "but the following layers are unsupported (" << unsupportedLayers.size() << "):" << std::endl;
            for (const char* layerName : unsupportedLayers) msg << '\t' << layerName << std::endl;

            throw std::runtime_error(msg.str());
        }
    }
}

std::vector<const char *> Engine::getRequiredExtensions() {
    uint32_t glfwExtensionsCount = 0;
    const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);

    std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionsCount);

    if (engineSettings.validationLayers.size()) extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    return extensions;
}

VKAPI_ATTR VkBool32 VKAPI_CALL Engine::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {
    if (messageSeverity > VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

void Engine::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& debugCreateInfo){
    debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debugCreateInfo.pfnUserCallback = debugCallback;
}

void Engine::initInstance() {
    VkApplicationInfo appInfo {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = engineSettings.applicationName;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.ppEnabledExtensionNames = glfwGetRequiredInstanceExtensions(&createInfo.enabledExtensionCount);

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo {};
    VkValidationFeaturesEXT features = {};

    if (engineSettings.validationLayers.size()) {
        createInfo.enabledLayerCount = engineSettings.validationLayers.size();
        createInfo.ppEnabledLayerNames = engineSettings.validationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);

        if (engineSettings.validationExtentions.size()) {
            features.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
            features.enabledValidationFeatureCount = engineSettings.validationExtentions.size();
            features.pEnabledValidationFeatures = engineSettings.validationExtentions.data();
            debugCreateInfo.pNext = &features;
        }
        createInfo.pNext = &debugCreateInfo;
    } else {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    auto extensions = getRequiredExtensions();
    for (const char *name : requiredInstanceExtensions) {
        extensions.push_back(name);
    }
    createInfo.enabledExtensionCount = extensions.size();
    createInfo.ppEnabledExtensionNames = extensions.data();

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
        throw std::runtime_error("Failed to create vulkan instance.");

    if (engineSettings.validationLayers.size()) {
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
        populateDebugMessengerCreateInfo(debugCreateInfo);
        debugCreateInfo.pUserData = nullptr;
        debugCreateInfo.pNext = nullptr;
        debugCreateInfo.flags = 0;

        if (CreateDebugUtilsMessengerEXT(instance, &debugCreateInfo, nullptr, &debugMessenger) != VK_SUCCESS)
            throw std::runtime_error("Failed to create debug messenger.");
    }

    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) throw std::runtime_error("Failed to create window surface.");
}

std::set<uint32_t> Engine::QueueFamilyIndices::families() {
    return std::set<uint32_t> { graphicsFamily.value(), presentFamily.value(), transferFamily.value() };
}

bool Engine::QueueFamilyIndices::isSupported() {
    return graphicsFamily.has_value() && presentFamily.has_value() && transferFamily.has_value();
}

std::vector<const char *> Engine::QueueFamilyIndices::whatIsMissing() {
    std::vector<const char *> missingFamilies;
    if (!graphicsFamily.has_value()) missingFamilies.push_back("Graphics family queue");
    if (!presentFamily.has_value()) missingFamilies.push_back("Present family queue");
    if (!transferFamily.has_value()) missingFamilies.push_back("Transfer family queue");
    return missingFamilies;
}

std::string Engine::QueueFamilyIndices::info() {
    std::stringstream output;
    output << std::hex << "Present family: " << (presentFamily.has_value() ? std::to_string(presentFamily.value()) : "None") << std::endl;
    output << "Graphics family: " << (graphicsFamily.has_value() ? std::to_string(graphicsFamily.value()) : "None") << std::endl;
    output << "Transfer family: " << (transferFamily.has_value() ? std::to_string(transferFamily.value()) : "None") << std::endl;
    return output.str();
}

bool Engine::deviceSupportsExtensions(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(requiredDeviceExtensions.begin(), requiredDeviceExtensions.end());

    for (const auto& extension : availableExtensions) requiredExtensions.erase(extension.extensionName);

    return requiredExtensions.empty();
}

// TODO support fallback if missing a dedicated transfer queue family
Engine::QueueFamilyIndices Engine::findQueueFamilyIndices(VkPhysicalDevice device, VkSurfaceKHR surface, bool useConcurrentTransferQueue) {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
            if (!useConcurrentTransferQueue) indices.transferFamily = i;
        }
        if (!(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) && queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) indices.transferFamily = i;

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
        if (presentSupport) indices.presentFamily = i;

        if (indices.isSupported()) break;
        i++;
    }

    return indices;
}

Engine::SwapChainSupportDetails Engine::querySwapChainSupport(VkPhysicalDevice device) {
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

bool Engine::deviceSupportsSwapChain(VkPhysicalDevice device) {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
    return !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
}

static inline int invalidateScore(int score) { return score < 0 ? score : -score; }

void Engine::pickPhysicalDevice() {
    uint32_t deviceCount = 0;

    vulkanErrorGuard(vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr), "Unable to enemerate physical devices");
    if (deviceCount == 0) throw std::runtime_error("Unable to find GPU with vulkan support.");

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vulkanErrorGuard(vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data()), "Unable to enemerate physical devices");

    // Do we really need the device features?
    std::multimap<int, std::tuple<VkPhysicalDevice, VkPhysicalDeviceProperties, VkPhysicalDeviceFeatures, QueueFamilyIndices>> deviceInfos;
    std::transform(devices.begin(), devices.end(), std::inserter(deviceInfos, deviceInfos.begin()),
        [this](VkPhysicalDevice device) -> std::pair<int, std::tuple<VkPhysicalDevice, VkPhysicalDeviceProperties, VkPhysicalDeviceFeatures, QueueFamilyIndices>> {
            QueueFamilyIndices indices = findQueueFamilyIndices(device, surface, engineSettings.useConcurrentTransferQueue);
            VkPhysicalDeviceProperties deviceProperties;
            VkPhysicalDeviceFeatures deviceFeatures;
            vkGetPhysicalDeviceProperties(device, &deviceProperties);
            vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
            int score = deviceProperties.limits.maxImageDimension2D;
            if (!indices.isSupported() /*|| !deviceSupportsExtensions(device)*/ ||
                !deviceSupportsSwapChain(device) || !deviceFeatures.samplerAnisotropy) score = invalidateScore(score);
            return std::make_pair(score, std::make_tuple(device, deviceProperties, deviceFeatures, indices));
        });

    if (engineSettings.verbose) for(const auto& [score, details] : deviceInfos) std::cout << score << " - " << std::get<1>(details).deviceName
        << " - " << std::get<1>(details).apiVersion << std::endl;

    if (deviceInfos.rbegin()->first < 0)
        throw std::runtime_error("No suitable gpu was found on this system. (Missing feature queues, extensions, or swap support).");

    const int deviceListOffset = 0;
    physicalDevice = std::get<0>(std::next(deviceInfos.rbegin(), deviceListOffset)->second);
    physicalDeviceIndices = std::get<3>(std::next(deviceInfos.rbegin(), deviceListOffset)->second);
    // for right now just run at max availible anisotrpic filtering level for textures
    maxSamplerAnisotropy = std::get<1>(std::next(deviceInfos.rbegin(), deviceListOffset)->second).limits.maxSamplerAnisotropy;
    // same with multisampling
    msaaSamples = getMaxUsableSampleCount(std::get<1>(std::next(deviceInfos.rbegin(), deviceListOffset)->second));
    minUniformBufferOffsetAlignment = std::get<1>(std::next(deviceInfos.rbegin(), deviceListOffset)->second).limits.minUniformBufferOffsetAlignment;

    // TODO We should act on these values in case it doesn't support line width and make an error message rather than just try and initalize the device anyways
    lineWidthRange = {
        std::get<1>(std::next(deviceInfos.rbegin(), deviceListOffset)->second).limits.lineWidthRange[0],
        std::get<1>(std::next(deviceInfos.rbegin(), deviceListOffset)->second).limits.lineWidthRange[1]
    };
    lineWidthGranularity = std::get<1>(std::next(deviceInfos.rbegin(), deviceListOffset)->second).limits.lineWidthGranularity;

    if (engineSettings.verbose) std::cout << physicalDeviceIndices.info();
}

VkSampleCountFlagBits Engine::getMaxUsableSampleCount(const VkPhysicalDeviceProperties& physicalDeviceProperties) {
    VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
    if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
    if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
    if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
    if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
    if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
    if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

    return VK_SAMPLE_COUNT_1_BIT;
}

void Engine::setupLogicalDevice() {
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    float queuePriorities[] = { 1.0f, 0.0f, 0.1f };

    for(uint32_t queueFamily : physicalDeviceIndices.families() ) {
        VkDeviceQueueCreateInfo queueCreateInfo {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = queueFamily == physicalDeviceIndices.graphicsFamily.value() ? 3 : 1;
        queueCreateInfo.pQueuePriorities = queuePriorities;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures {};
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    deviceFeatures.wideLines = VK_TRUE;
    deviceFeatures.sampleRateShading = VK_TRUE;
    // Vulkan 1.2 feature that I might want to use

    // VkPhysicalDeviceDescriptorIndexingFeatures indexingFeatures {};
    // indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
    // indexingFeatures.runtimeDescriptorArray = VK_TRUE;

    VkPhysicalDeviceVulkan12Features otherFeatures {};
    otherFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    otherFeatures.pNext = nullptr;
    otherFeatures.drawIndirectCount = VK_TRUE;
    otherFeatures.descriptorIndexing = VK_TRUE;
    // otherFeatures.runtimeDescriptorArray = VK_TRUE;

    VkDeviceCreateInfo createInfo {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = queueCreateInfos.size();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = requiredDeviceExtensions.size();
    createInfo.ppEnabledExtensionNames = requiredDeviceExtensions.data();
    if (engineSettings.validationLayers.size()) {
        createInfo.enabledLayerCount = engineSettings.validationLayers.size();
        createInfo.ppEnabledLayerNames = engineSettings.validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    vulkanErrorGuard(vkCreateDevice(physicalDevice, &createInfo, nullptr, &device), "Failed to create logical device.");

    vkGetDeviceQueue(device, physicalDeviceIndices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, physicalDeviceIndices.graphicsFamily.value(), 1, &guiGraphicsQueue);
    vkGetDeviceQueue(device, physicalDeviceIndices.graphicsFamily.value(), 2, &computeQueue);
    vkGetDeviceQueue(device, physicalDeviceIndices.presentFamily.value(), 0, &presentQueue);
    vkGetDeviceQueue(device, physicalDeviceIndices.transferFamily.value(), 0, &transferQueue);
    // if (engineSettings.verbose) std::cout << std::hex << "Presentation queue: " << presentQueue << "  Graphics queue: " << graphicsQueue << "  Transfer queue: " << transferQueue << std::endl;

    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = physicalDevice;
    allocatorInfo.device = device;
    allocatorInfo.instance = instance;
    if (vmaCreateAllocator(&allocatorInfo, &memoryAllocator) != VK_SUCCESS)
        throw std::runtime_error("Unable to create memory allocator.");
}

VkSurfaceFormatKHR Engine::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats)
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return availableFormat;

    return availableFormats[0];
}

VkPresentModeKHR Engine::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes)
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            return availablePresentMode;

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Engine::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

void Engine::createSwapChain() {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    concurrentFrames = swapChainSupport.capabilities.minImageCount + 1;

    if (swapChainSupport.capabilities.maxImageCount > 0 && concurrentFrames > swapChainSupport.capabilities.maxImageCount)
        concurrentFrames = swapChainSupport.capabilities.maxImageCount;

    VkSwapchainCreateInfoKHR createInfo {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = concurrentFrames;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t queueFamilyIndices[] = { physicalDeviceIndices.graphicsFamily.value(), physicalDeviceIndices.presentFamily.value() };
    if (queueFamilyIndices[0] != queueFamilyIndices[1]) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = VK_NULL_HANDLE;

    vulkanErrorGuard(vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain), "Failed to create swap chain.");

    vkGetSwapchainImagesKHR(device, swapChain, &concurrentFrames, nullptr);
    swapChainImages.resize(concurrentFrames);
    vkGetSwapchainImagesKHR(device, swapChain, &concurrentFrames, swapChainImages.data());

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;

    concurrentFrames = 2;
}

VkImageView Engine::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
    VkImageViewCreateInfo viewInfo {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    // The docs said you could just do this, I don't know if there is a downside (might be slower or something)
    viewInfo.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

    VkImageView imageView;
    vulkanErrorGuard(vkCreateImageView(device, &viewInfo, nullptr, &imageView), "Unable to create texture image view.");

    return imageView;
}

// These views are for the swap chain in case that wasn't obvious...
void Engine::createImageViews() {
    subpassImages.resize(swapChainImages.size());
    subpassImageAllocations.resize(swapChainImages.size());
    subpassImageViews.resize(swapChainImages.size());
    bgSubpassImages.resize(swapChainImages.size());
    bgSubpassImageAllocations.resize(swapChainImages.size());
    bgSubpassImageViews.resize(swapChainImages.size());
    swapChainImageViews.resize(swapChainImages.size());
    static bool notFirstCall;

    for (int i = 0; i < swapChainImages.size(); i++) {
        VkImageCreateInfo imageInfo {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = swapChainExtent.width;
        imageInfo.extent.height = swapChainExtent.height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        // I am not sure about this one
        imageInfo.format = swapChainImageFormat;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        // imageInfo.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
        imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo imageAllocCreateInfo = {};
        imageAllocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        if (vmaCreateImage(memoryAllocator, &imageInfo, &imageAllocCreateInfo, &subpassImages[i], &subpassImageAllocations[i], nullptr) != VK_SUCCESS)
            throw std::runtime_error("Unable to create attachment images.");

        if (vmaCreateImage(memoryAllocator, &imageInfo, &imageAllocCreateInfo, &bgSubpassImages[i], &bgSubpassImageAllocations[i], nullptr) != VK_SUCCESS)
            throw std::runtime_error("Unable to create attachment images.");


        // I cant even do this here because with have no commandbuffers yet
        // transitionImageLayout(subpassImages[i], swapChainImageFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        if (notFirstCall) {
            VkCommandBuffer commandBuffer = beginSingleTimeCommands();

            VkImageMemoryBarrier subpassBarrier {};
            subpassBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            subpassBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            subpassBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            subpassBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            subpassBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            subpassBarrier.image = subpassImages[i];
            subpassBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            subpassBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            subpassBarrier.subresourceRange.baseMipLevel = 0;
            subpassBarrier.subresourceRange.levelCount = 1;
            subpassBarrier.subresourceRange.baseArrayLayer = 0;
            subpassBarrier.subresourceRange.layerCount = 1;
            subpassBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            subpassBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            VkImageMemoryBarrier bgBarrier {};
            bgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            bgBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            bgBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            bgBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            bgBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            bgBarrier.image = bgSubpassImages[i];
            bgBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            bgBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            bgBarrier.subresourceRange.baseMipLevel = 0;
            bgBarrier.subresourceRange.levelCount = 1;
            bgBarrier.subresourceRange.baseArrayLayer = 0;
            bgBarrier.subresourceRange.layerCount = 1;
            bgBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            bgBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            std::array<VkImageMemoryBarrier, 2> barriers = { subpassBarrier, bgBarrier };

            vkCmdPipelineBarrier(
                commandBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                0,
                0, nullptr,
                0, nullptr,
                barriers.size(), barriers.data()
            );

            endSingleTimeCommands(commandBuffer);
        }

        VkImageViewCreateInfo imageViewInfo {};

        subpassImageViews[i] = createImageView(subpassImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
        bgSubpassImageViews[i] = createImageView(bgSubpassImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
        swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
    }
    notFirstCall = true;
}

void Engine::destroyImageViews() {
    for(int i = 0; i < subpassImageViews.size(); i++) {
        vkDestroyImageView(device, subpassImageViews[i], nullptr);
        vmaDestroyImage(memoryAllocator, subpassImages[i], bgSubpassImageAllocations[i]);
        vkDestroyImageView(device, bgSubpassImageViews[i], nullptr);
        vmaDestroyImage(memoryAllocator, bgSubpassImages[i], subpassImageAllocations[i]);
    }
    for (auto imageView : swapChainImageViews) {
        vkDestroyImageView(device, imageView, nullptr);
    }
}

VkFormat Engine::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
    for (VkFormat format : candidates) {
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &properties);

        if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features) return format;
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features) return format;
    }
    throw std::runtime_error("Failed to find supported format.");
}

VkFormat Engine::findDepthFormat() {
    return findSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
        VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

// This is confussing to me at this point, stuff is named poorly
void Engine::createRenderPass() {
    VkAttachmentDescription colorAttachment {};
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription depthAttachment {};
    depthAttachment.format = findDepthFormat();
    depthAttachment.samples = msaaSamples;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depthGuiAttachment {};
    depthGuiAttachment.format = findDepthFormat();
    depthGuiAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthGuiAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthGuiAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthGuiAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthGuiAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthGuiAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthGuiAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription subpassAttachment {};
    subpassAttachment.format = swapChainImageFormat;
    subpassAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    subpassAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    subpassAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    subpassAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    subpassAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    subpassAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    subpassAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkAttachmentDescription bgAttachment {};
    bgAttachment.format = swapChainImageFormat;
    bgAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    bgAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    bgAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    bgAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    bgAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    bgAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    bgAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkAttachmentDescription colorAAAttachment {};
    colorAAAttachment.format = swapChainImageFormat;
    colorAAAttachment.samples = msaaSamples;
    colorAAAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAAAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAAAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAAAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAAAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAAAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference subpass0color = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
    VkAttachmentReference subpass0depth = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
    VkAttachmentReference colorAA = { 4, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
    VkAttachmentReference guiDepthReference { 5, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

    std::array<VkSubpassDescription, 3> subpasses {};
    subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpasses[0].colorAttachmentCount = 1;
    subpasses[0].pColorAttachments = &colorAA;
    subpasses[0].pDepthStencilAttachment = &subpass0depth;
    subpasses[0].pResolveAttachments = &subpass0color;

    VkAttachmentReference subpass1color = { 3, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

    subpasses[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpasses[1].colorAttachmentCount = 1;
    subpasses[1].pColorAttachments = &subpass1color;
    subpasses[1].pDepthStencilAttachment = nullptr;
    subpasses[1].pResolveAttachments = nullptr;
    subpasses[1].inputAttachmentCount = 0;
    subpasses[1].pInputAttachments = nullptr;

    VkAttachmentReference subpass2color = {2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
    VkAttachmentReference subpass2inputs[2] = {
        { 0, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL },
        { 3, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }
    };

    subpasses[2].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpasses[2].colorAttachmentCount = 1;
    subpasses[2].pColorAttachments = &subpass2color;
    subpasses[2].pDepthStencilAttachment = &guiDepthReference;
    subpasses[2].pResolveAttachments = nullptr;
    subpasses[2].inputAttachmentCount = 2;
    subpasses[2].pInputAttachments = subpass2inputs;
    // Can we just reuse the depth attachment?

    std::array<VkAttachmentDescription, 6> attachments = {
        subpassAttachment,
        depthAttachment,
        colorAttachment,
        bgAttachment,
        colorAAAttachment,
        depthGuiAttachment
    };
    VkRenderPassCreateInfo renderPassInfo {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = attachments.size();
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = subpasses.size();;
    renderPassInfo.pSubpasses = subpasses.data();

    std::array<VkSubpassDependency, 4> dependencies {};

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependencies[0].srcAccessMask = 0;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    dependencies[1].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].dstSubpass = 1;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependencies[1].srcAccessMask = 0;
    dependencies[1].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    dependencies[2].srcSubpass = 0;
    dependencies[2].dstSubpass = 2;
    dependencies[2].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependencies[2].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependencies[2].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[2].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    dependencies[3].srcSubpass = 1;
    dependencies[3].dstSubpass = 2;
    dependencies[3].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependencies[3].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependencies[3].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[3].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    renderPassInfo.dependencyCount = dependencies.size();
    renderPassInfo.pDependencies = dependencies.data();

    vulkanErrorGuard(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass), "Failed to create render pass.");
}

VkSampler TextResource::sampler_;

void Engine::createDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding uboLayoutBinding {};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding samplerLayoutBinding {};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = ECONF_MAX_ENTITY_TEXTURES;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding shadowMapBinding {};
    shadowMapBinding.binding = 2;
    shadowMapBinding.descriptorCount = 1;
    shadowMapBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    shadowMapBinding.pImmutableSamplers = nullptr;
    shadowMapBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding lightingBinding {};
    lightingBinding.binding = 3;
    lightingBinding.descriptorCount = 1;
    lightingBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    lightingBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    lightingBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding skyboxSamplerBinding {};
    skyboxSamplerBinding.binding = 4;
    skyboxSamplerBinding.descriptorCount = 1;
    skyboxSamplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    skyboxSamplerBinding.pImmutableSamplers = nullptr;
    skyboxSamplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 5> bindings = { uboLayoutBinding, samplerLayoutBinding, shadowMapBinding, lightingBinding, skyboxSamplerBinding };
    VkDescriptorSetLayoutCreateInfo layoutInfo {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = bindings.size();
    layoutInfo.pBindings = bindings.data();

    vulkanErrorGuard(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &mainPass.descriptorSetLayout), "Failed to create descriptor set layout.");

    // for line drawing all we need is one ubo
    layoutInfo.bindingCount = 1;
    vulkanErrorGuard(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &mainPass.lineDescriptorLayout), "Failed to create descriptor set layout.");

    VkDescriptorSetLayoutBinding hudColorInput {};
    hudColorInput.binding = 0;
    hudColorInput.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    hudColorInput.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    hudColorInput.descriptorCount = 1;

    VkDescriptorSetLayoutBinding hudIconInput {};
    hudIconInput.binding = 1;
    hudIconInput.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    hudIconInput.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    hudIconInput.descriptorCount = 1;

    VkSamplerCreateInfo samplerInfo {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;

    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    // Anisotropic filtering is irrelevant on this as we view it straight on every time
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = maxSamplerAnisotropy;

    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.maxLod = 1.0f;
    samplerInfo.mipLodBias = 0.0f;

    vulkanErrorGuard(vkCreateSampler(device, &samplerInfo, nullptr, &TextResource::sampler_), "Failed to create texture sampler.");

    std::vector<VkSampler> samplers(ECONF_MAX_HUD_TEXTURES);
    fill(samplers.begin(), samplers.end(), TextResource::sampler_);

    VkDescriptorSetLayoutBinding hudTextures {};
    hudTextures.binding = 2;
    hudTextures.descriptorCount = ECONF_MAX_HUD_TEXTURES;
    hudTextures.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    hudTextures.pImmutableSamplers = samplers.data();
    hudTextures.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding tooltipTexture {};
    tooltipTexture.binding = 3;
    tooltipTexture.descriptorCount = 1;
    tooltipTexture.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    tooltipTexture.pImmutableSamplers = &TextResource::sampler_;
    tooltipTexture.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 4> hudBindings = { hudColorInput, hudIconInput, hudTextures, tooltipTexture };
    VkDescriptorSetLayoutCreateInfo hudLayout {};
    hudLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    hudLayout.bindingCount = hudBindings.size();
    hudLayout.pBindings = hudBindings.data();

    vulkanErrorGuard(vkCreateDescriptorSetLayout(device, &hudLayout, nullptr, &hudDescriptorLayout), "Failed to create hud descriptor set layout.");
}

VkShaderModule Engine::createShaderModule(const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    vulkanErrorGuard(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule), "Failed to create shader module.");

    return shaderModule;
}

void Engine::createGraphicsPipelines() {
    auto vertShaderCode = Utilities::readFile("shaders/vert.spv");
    auto fragShaderCode = Utilities::readFile("shaders/frag.spv");
    auto lineVertShaderCode = Utilities::readFile("shaders/line_vert.spv");
    auto lineFragShaderCode = Utilities::readFile("shaders/line_frag.spv");
    auto hudVertShaderCode = Utilities::readFile("shaders/hud_vert.spv");
    auto hudFragShaderCode = Utilities::readFile("shaders/hud_frag.spv");

    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);
    VkShaderModule lineVertShaderModule = createShaderModule(lineVertShaderCode);
    VkShaderModule lineFragShaderModule = createShaderModule(lineFragShaderCode);
    VkShaderModule hudVertShaderModule = createShaderModule(hudVertShaderCode);
    VkShaderModule hudFragShaderModule = createShaderModule(hudFragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo {};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo {};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo lineVertShaderStageInfo {};
    lineVertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    lineVertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    lineVertShaderStageInfo.module = lineVertShaderModule;
    lineVertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo lineFragShaderStageInfo {};
    lineFragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    lineFragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    lineFragShaderStageInfo.module = lineFragShaderModule;
    lineFragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo hudVertShaderStageInfo {};
    hudVertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    hudVertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    hudVertShaderStageInfo.module = hudVertShaderModule;
    hudVertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo hudFragShaderStageInfo {};
    hudFragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    hudFragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    hudFragShaderStageInfo.module = hudFragShaderModule;
    hudFragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };
    VkPipelineShaderStageCreateInfo hudShaders[] = { hudVertShaderStageInfo, hudFragShaderStageInfo };
    VkPipelineShaderStageCreateInfo lineShaders[] = { lineVertShaderStageInfo, lineFragShaderStageInfo };

    VkPipelineVertexInputStateCreateInfo vertexInputInfo {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    auto bindingDescription = Utilities::Vertex::getBindingDescription();
    auto attributeDescriptions = Utilities::Vertex::getAttributeDescriptions();

    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapChainExtent.width);
    viewport.height = static_cast<float>(swapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor {};
    scissor.offset = { 0, 0 };
    scissor.extent = swapChainExtent;

    VkPipelineViewportStateCreateInfo viewportState {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;

    VkPipelineMultisampleStateCreateInfo multisampling {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = msaaSamples;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &mainPass.descriptorSetLayout;

    VkPushConstantRange pushConstantRange;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(PushConstants);
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    vulkanErrorGuard(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout), "Failed to create pipeline layout.");

    // pushConstantRange.size = sizeof(ViewProj);
    pipelineLayoutInfo.pSetLayouts = &mainPass.lineDescriptorLayout;

    vulkanErrorGuard(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &linePipelineLayout), "Failed to create pipeline layout.");

    VkPipelineDepthStencilStateCreateInfo depthStencil {};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f;
    depthStencil.maxDepthBounds = 1.0f;
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {};
    depthStencil.back = {};

    VkGraphicsPipelineCreateInfo pipelineInfo {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;

    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = nullptr;

    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    pipelineInfo.pDepthStencilState = &depthStencil;

    vulkanErrorGuard(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineInfo, nullptr, &graphicsPipelines[GP_WORLD]), "Failed to create graphics pipeline.");

    rasterizer.cullMode = VK_CULL_MODE_NONE;
    colorBlendAttachment.blendEnable = VK_FALSE;
    pipelineInfo.subpass = 1;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    vulkanErrorGuard(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineInfo, nullptr, &graphicsPipelines[GP_BG]), "Failed to create graphics pipeline.");

    colorBlendAttachment.blendEnable = VK_FALSE;
    pipelineInfo.pStages = lineShaders;
    pipelineInfo.subpass = 0;
    multisampling.rasterizationSamples = msaaSamples;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;

    // I am going to use this soon for drawing pathing lines and stuff in the world
    VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_LINE_WIDTH
    };

    VkPipelineDynamicStateCreateInfo dynamicState {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 1;
    dynamicState.pDynamicStates = dynamicStates;

    pipelineInfo.pDynamicState = &dynamicState;

    pipelineInfo.layout = linePipelineLayout;
    colorBlendAttachment.blendEnable = VK_TRUE;
    vulkanErrorGuard(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineInfo, nullptr, &graphicsPipelines[GP_LINES]),
        "Failed to create graphics pipeline.");
    colorBlendAttachment.blendEnable = VK_FALSE;

    pipelineInfo.pDynamicState = nullptr;

    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    pipelineInfo.subpass = 2;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineVertexInputStateCreateInfo hudVertexInputInfo {};
    hudVertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    auto hudBindingDescription = GuiVertex::getBindingDescription();
    auto hudAttributeDescriptions = GuiVertex::getAttributeDescriptions();

    hudVertexInputInfo.vertexBindingDescriptionCount = 1;
    hudVertexInputInfo.vertexAttributeDescriptionCount = hudAttributeDescriptions.size();
    hudVertexInputInfo.pVertexBindingDescriptions = &hudBindingDescription;
    hudVertexInputInfo.pVertexAttributeDescriptions = hudAttributeDescriptions.data();

    pipelineInfo.pVertexInputState = &hudVertexInputInfo;

    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = hudShaders;

    VkPipelineLayoutCreateInfo hudLayoutInfo {};
    hudLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    hudLayoutInfo.setLayoutCount = 1;
    hudLayoutInfo.pSetLayouts = &hudDescriptorLayout;

    // This way I don't have to worry about which is the front side of the triangles for the hud, we wouldnt want anything culled anyways
    rasterizer.cullMode = VK_CULL_MODE_NONE;

    VkPushConstantRange hudPushConstantRange;
    hudPushConstantRange.offset = 0;
    hudPushConstantRange.size = sizeof(GuiPushConstant);
    // Why need fragment stage access here???? (I don't for my needs, but the validation is complaining, (it is like mixing up the subpasses or something))
    hudPushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    hudLayoutInfo.pushConstantRangeCount = 1;
    hudLayoutInfo.pPushConstantRanges = &hudPushConstantRange;

    vulkanErrorGuard(vkCreatePipelineLayout(device, &hudLayoutInfo, nullptr, &hudPipelineLayout), "Failed to create pipeline layout.");

    pipelineInfo.layout = hudPipelineLayout;
    colorBlendAttachment.blendEnable = VK_TRUE;

    // depthStencil.depthTestEnable = VK_FALSE;
    // depthStencil.depthWriteEnable = VK_FALSE;
    // depthStencil.depthCompareOp = VK_COMPARE_OP_GREATER_OR_EQUAL;

    // vulkanErrorGuard(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineInfo, nullptr, &graphicsPipelines[1]), "Failed to create graphics pipeline.");
    multisampling.sampleShadingEnable = VK_TRUE;
    multisampling.minSampleShading = .2f;
    
    vulkanErrorGuard(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineInfo, nullptr, &graphicsPipelines[GP_HUD]), "Failed to create graphics pipeline.");

    vkDestroyShaderModule(device, fragShaderModule, nullptr);
    vkDestroyShaderModule(device, vertShaderModule, nullptr);
    vkDestroyShaderModule(device, lineFragShaderModule, nullptr);
    vkDestroyShaderModule(device, lineVertShaderModule, nullptr);
    vkDestroyShaderModule(device, hudFragShaderModule, nullptr);
    vkDestroyShaderModule(device, hudVertShaderModule, nullptr);
}

void Engine::createComputeResources() {
    std::array<VkDescriptorSetLayoutBinding, 4> layoutBindings;

    layoutBindings[0].binding = 0;
    layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    layoutBindings[0].descriptorCount = ECONF_MAX_GLYPH_TEXTURES;
    layoutBindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    layoutBindings[1].binding = 1;
    layoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    layoutBindings[1].descriptorCount = 1;
    layoutBindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    layoutBindings[2].binding = 2;
    layoutBindings[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layoutBindings[2].descriptorCount = 1;
    layoutBindings[2].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    layoutBindings[3].binding = 3;
    layoutBindings[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    layoutBindings[3].descriptorCount = 1;
    layoutBindings[3].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkDescriptorSetLayoutCreateInfo setLayoutCreateInfo {};
    setLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    setLayoutCreateInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
    setLayoutCreateInfo.pBindings = layoutBindings.data();

    vulkanErrorGuard(vkCreateDescriptorSetLayout(device, &setLayoutCreateInfo, nullptr, &computeSetLayouts[CP_SDF_BLIT]),
        "Failed to create descriptor set layout.");

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo {};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = 1;
    pipelineLayoutCreateInfo.pSetLayouts = &computeSetLayouts[CP_SDF_BLIT];

    vulkanErrorGuard(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &computePipelineLayouts[CP_SDF_BLIT]),
        "Failed to create pipeline layout.");

    auto sdfBlitCode = Utilities::readFile("shaders/sdf_blit.spv");
    VkShaderModule sdfBlitShader = createShaderModule(sdfBlitCode);

    VkComputePipelineCreateInfo pipelineCreateInfo{};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    pipelineCreateInfo.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    pipelineCreateInfo.stage.module = sdfBlitShader;

    pipelineCreateInfo.stage.pName = "main";
    pipelineCreateInfo.layout = computePipelineLayouts[CP_SDF_BLIT];

    vulkanErrorGuard(vkCreateComputePipelines(device, pipelineCache, 1, &pipelineCreateInfo, nullptr, &computePipelines[CP_SDF_BLIT]),
        "Failed to create compute pipeline.");

    vkDestroyShaderModule(device, sdfBlitShader, nullptr);

    std::array<VkDescriptorPoolSize, 4> poolSizes;
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    poolSizes[0].descriptorCount = ECONF_MAX_GLYPH_TEXTURES;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    poolSizes[1].descriptorCount = 1;
    poolSizes[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[2].descriptorCount = 1;
    poolSizes[3].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[3].descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolInfo {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = poolSizes.size();
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = concurrentFrames;

    vulkanErrorGuard(vkCreateDescriptorPool(device, &poolInfo, nullptr, &computePools[CP_SDF_BLIT]), "Failed to create descriptor pool.");

    VkDescriptorSetAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = computePools[CP_SDF_BLIT];
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &computeSetLayouts[CP_SDF_BLIT];

    vulkanErrorGuard(vkAllocateDescriptorSets(device, &allocInfo, &computeSets[CP_SDF_BLIT]), "Failed to allocate descriptor sets.");

    VkFenceCreateInfo sdfFenceInfo {};
    sdfFenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    vulkanErrorGuard(vkCreateFence(device, &sdfFenceInfo, nullptr, &sdfBlitFence), "Failed to create compute fence.");

    VkBufferCreateInfo sdfUBOInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    sdfUBOInfo.size = sizeof(GlyphInfoUBO);
    sdfUBOInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    VmaAllocationCreateInfo sdfUBOAllocationInfo = {};
    sdfUBOAllocationInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
    sdfUBOAllocationInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

    if (vmaCreateBuffer(memoryAllocator, &sdfUBOInfo, &sdfUBOAllocationInfo, &sdfUBOBuffer, &sdfUBOAllocation, nullptr) != VK_SUCCESS)
        throw std::runtime_error("Unable to allocate memory for sdf uniform buffers.");

    VkDescriptorBufferInfo sdfUBOBufferInfo {};
    sdfUBOBufferInfo.buffer = sdfUBOBuffer;
    sdfUBOBufferInfo.offset = 0;
    sdfUBOBufferInfo.range = sizeof(GlyphInfoUBO);

    VkWriteDescriptorSet sdfUBOdescriptorWrite {};

    sdfUBOdescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    sdfUBOdescriptorWrite.dstSet = computeSets[CP_SDF_BLIT];
    sdfUBOdescriptorWrite.dstBinding = 2;
    sdfUBOdescriptorWrite.dstArrayElement = 0;
    sdfUBOdescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    sdfUBOdescriptorWrite.descriptorCount = 1;
    sdfUBOdescriptorWrite.pBufferInfo = &sdfUBOBufferInfo;

    vkUpdateDescriptorSets(device, 1, &sdfUBOdescriptorWrite, 0, nullptr);
}

void Engine::destroyComputeResources() {
    for (auto &dp : computePools) {
        vkDestroyDescriptorPool(device, dp, nullptr);
    }
    for (auto& pl : computePipelines) {
        vkDestroyPipeline(device, pl, nullptr);
    }
    for (auto& pll : computePipelineLayouts) {
        vkDestroyPipelineLayout(device, pll, nullptr);
    }
    for (auto& dsl : computeSetLayouts) {
        vkDestroyDescriptorSetLayout(device, dsl, nullptr);
    }
    vkDestroyFence(device, sdfBlitFence, nullptr);
    vmaDestroyBuffer(memoryAllocator, sdfUBOBuffer, sdfUBOAllocation);
}

// Remember that all the command buffers from a pool must be accessed from the same thread
void Engine::createCommandPools() {
    VkCommandPoolCreateInfo poolInfo {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = physicalDeviceIndices.graphicsFamily.value();
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    vulkanErrorGuard(vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool), "Failed to create command pool.");
    // The gui pool is exactly the same, but it is just accessed from another thread so we need another pool
    vulkanErrorGuard(vkCreateCommandPool(device, &poolInfo, nullptr, &guiCommandPool), "Failed to create command pool.");
    vulkanErrorGuard(vkCreateCommandPool(device, &poolInfo, nullptr, &computeCommandPool), "Failed to create command pool.");

    if (engineSettings.useConcurrentTransferQueue) {
        poolInfo.flags = 0;
        poolInfo.queueFamilyIndex = physicalDeviceIndices.transferFamily.value();

        vulkanErrorGuard(vkCreateCommandPool(device, &poolInfo, nullptr, &transferCommandPool), "Failed to create transfer command pool.");
    }
}

uint32_t Engine::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    if(engineSettings.extremelyVerbose)
        std::cout << "Most calls to this function should be elliminated in favor of using the vma allocator." << std::endl
            << boost::stacktrace::stacktrace() << std::endl;

    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) if (typeFilter & (1 << i) &&
        (memProperties.memoryTypes[i].propertyFlags & properties) == properties) return i;

    throw std::runtime_error("Failed to find suitable memory type.");
}

// TODO Really all calls to this should be replaced with vmaCreateImage
void Engine::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
    VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory, uint32_t mipLevels) {

    // std::cout << std::dec << "Creating image with mip levels of " << mipLevels << std::endl;
    VkImageCreateInfo imageInfo {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = mipLevels;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    // should the transfer queue transfer it?
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    vulkanErrorGuard(vkCreateImage(device, &imageInfo, nullptr, &image), "Failed to create image.");

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    vulkanErrorGuard(vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory), "Failed to allocate image memory.");

    vkBindImageMemory(device, image, imageMemory, 0);
}

// This creates a command buffer for the graphics queue in the default command pool
VkCommandBuffer Engine::beginSingleTimeCommands() {
    return beginSingleTimeCommands(commandPool);
}

// We can use this to create a command buffer on other queues (namely the tranfer queue) in the default command buffer
VkCommandBuffer Engine::beginSingleTimeCommands(VkCommandPool pool) {
    VkCommandBufferAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = pool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

// Using the fenced versions requires manual freeing of the commandbuffer via the returned lambda
std::function<void (void)> Engine::endSingleTimeCommands(VkCommandBuffer commandBuffer, VkFence fence) {
    return endSingleTimeCommands(commandBuffer, commandPool, graphicsQueue, fence);
}

std::function<void (void)> Engine::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
    return endSingleTimeCommands(commandBuffer, commandPool, graphicsQueue);
}

std::function<void (void)> Engine::endSingleTimeCommands(VkCommandBuffer commandBuffer, VkCommandPool pool, VkQueue queue) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);

    vkFreeCommandBuffers(device, pool, 1, &commandBuffer);

    return [](){};
}

// Using the fenced versions requires manual freeing of the commandbuffer via the returned lambda
std::function<void (void)> Engine::endSingleTimeCommands(VkCommandBuffer commandBuffer, VkCommandPool pool, VkQueue queue, VkFence fence) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(queue, 1, &submitInfo, fence);

    return [this, pool, commandBuffer](){ vkFreeCommandBuffers(device, pool, 1, &commandBuffer); };
}

bool Engine::hasStencilComponent(VkFormat format) {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

// Note that this should not be done with the tranfer queue (the validation layer complains that you are using the queue incorrectly)
void Engine::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels) {
    // std::cout << "Using transition mip levels of " << mipLevels << std::endl;
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkImageMemoryBarrier barrier {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;

    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    barrier.image = image;

    if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if (hasStencilComponent(format)) {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    } else {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    } else throw std::invalid_argument("Unsupported layout transition.");

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    endSingleTimeCommands(commandBuffer);
}

void Engine::createDepthResources() {
    depthGuiImages.resize(swapChainImages.size());
    depthGuiImageAllocations.resize(swapChainImages.size());
    depthGuiImageViews.resize(swapChainImages.size());
    depthImages.resize(swapChainImages.size());
    depthImageAllocations.resize(swapChainImages.size());
    depthImageViews.resize(swapChainImages.size());

    VkFormat depthFormat = findDepthFormat();

    depthDump.makeDump = false;
    depthDump.writePending = false;
    if (DEPTH_DEBUG_IMAGE_USAGE) {

        VkBufferCreateInfo debuggingBufferInfo = {};
        debuggingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        debuggingBufferInfo.size = swapChainExtent.width * swapChainExtent.height * 4;
        debuggingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;

        VmaAllocationCreateInfo debuggingAllocationInfo = {};
        debuggingAllocationInfo.usage = VMA_MEMORY_USAGE_GPU_TO_CPU;
        debuggingAllocationInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

        VmaAllocationInfo debuggingBufferAl = {};

        if (vmaCreateBuffer(memoryAllocator, &debuggingBufferInfo, &debuggingAllocationInfo, &depthDump.buffer, &depthDump.allocation, &depthDump.allocationInfo) != VK_SUCCESS)
            throw std::runtime_error("Unable to allocate memory for main depth dump buffer.");
    }

    for (int i = 0; i < swapChainImages.size(); i++) {
        VkImageCreateInfo imageInfo {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = swapChainExtent.width;
        imageInfo.extent.height = swapChainExtent.height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = depthFormat;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | DEPTH_DEBUG_IMAGE_USAGE;
        imageInfo.samples = msaaSamples;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo imageAllocCreateInfo = {};
        imageAllocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        vmaCreateImage(memoryAllocator, &imageInfo, &imageAllocCreateInfo, &depthImages[i], &depthImageAllocations[i], nullptr);

        depthImageViews[i] = createImageView(depthImages[i], depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
        transitionImageLayout(depthImages[i], depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        vmaCreateImage(memoryAllocator, &imageInfo, &imageAllocCreateInfo, &depthGuiImages[i], &depthGuiImageAllocations[i], nullptr);

        depthGuiImageViews[i] = createImageView(depthGuiImages[i], depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
        transitionImageLayout(depthGuiImages[i], depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    }
}

void Engine::createColorResources() {
    colorImages.resize(swapChainImages.size());
    colorImageAllocations.resize(swapChainImages.size());
    colorImageViews.resize(swapChainImages.size());

    for (int i = 0; i < swapChainImages.size(); i++) {
        VkImageCreateInfo imageInfo {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = swapChainExtent.width;
        imageInfo.extent.height = swapChainExtent.height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = swapChainImageFormat;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        imageInfo.samples = msaaSamples;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo imageAllocCreateInfo = {};
        imageAllocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        vmaCreateImage(memoryAllocator, &imageInfo, &imageAllocCreateInfo, &colorImages[i], &colorImageAllocations[i], nullptr);
        // vmaCreateImage(memoryAllocator, &imageInfo, &imageAllocCreateInfo, &colorImages[2 * i + 1], &colorImageAllocations[2 * i + 1], nullptr);

        VkImageViewCreateInfo viewInfo {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = colorImages[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = swapChainImageFormat;
        viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        // The docs said you could just do this, I don't know if there is a downside (might be slower or something)
        viewInfo.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

        vulkanErrorGuard(vkCreateImageView(device, &viewInfo, nullptr, &colorImageViews[i]), "Unable to create texture image view.");
        // viewInfo.image = colorImages[2 * i + 1];
        // vulkanErrorGuard(vkCreateImageView(device, &viewInfo, nullptr, &colorImageViews[2 * i + 1]), "Unable to create texture image view.");
        // depthImageViews[i] = createImageView(depthImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
        // transitionImageLayout(depthImages[i], depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    }
}

void Engine::dumpDepthBuffer(const VkCommandBuffer& buffer, int index) {
    // I really dont want to garble up the buffer (I should only ever hit this due to a programming error)
    assert(!depthDump.writePending);

    VkImageMemoryBarrier barrier {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = depthImages[index];

    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    // not sure about src mask ????
    barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    // I do know that the dst access mask is right
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

    // also no clue about stage masks??? I am continually confused by these things
    vkCmdPipelineBarrier(
        buffer,
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    VkBufferImageCopy region = {};
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    region.imageSubresource.layerCount = 1;
    region.imageExtent.width = swapChainExtent.width;
    region.imageExtent.height = swapChainExtent.height;
    region.imageExtent.depth = 1;

    vkCmdCopyImageToBuffer(buffer, depthImages[index], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, depthDump.buffer, 1, &region);

    // I don't think we have any reason to need this memory barrier in place (we get the dump and the end of the render pass and then we just dont care anymore)
    // barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    // barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

    // barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    // barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

    // vkCmdPipelineBarrier(
    //     buffer,
    //     VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
    //     VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
    //     0,
    //     0, nullptr,
    //     0, nullptr,
    //     1, &barrier
    // );

    depthDump.writePending = true;
    depthDump.makeDump = false;
}

std::array<int, 256>& Engine::logDepthBufferToFile(std::array<int, 256>& depthHistogram, const char *filename) {
    assert(depthDump.writePending);

    memset(depthHistogram.data(), 0, sizeof(int) * depthHistogram.size());

    // idk about single channle png images?
    unsigned char *buf = new unsigned char[swapChainExtent.width * swapChainExtent.height];
    float *data = static_cast<float *>(depthDump.allocation->GetMappedData());

    for (int y = 0; y < swapChainExtent.height; y++) {
        for (int x = 0; x < swapChainExtent.width; x++) {
            unsigned char value = static_cast<unsigned char>(data[y * swapChainExtent.width + x] * 255.0f);
            buf[y * swapChainExtent.width + x] = value;
            depthHistogram[value] += 1;
        }
    }

    stbi_write_png(filename, swapChainExtent.width, swapChainExtent.height, 1, buf, swapChainExtent.width);

    delete buf;

    depthDump.writePending = false;

    return depthHistogram;
}

void Engine::destroyDepthResources() {
    for (int i = 0; i < depthImages.size(); i++) {
        vkDestroyImageView(device, depthImageViews[i], nullptr);
        vmaDestroyImage(memoryAllocator, depthImages[i], depthImageAllocations[i]);
        vkDestroyImageView(device, depthGuiImageViews[i], nullptr);
        vmaDestroyImage(memoryAllocator, depthGuiImages[i], depthGuiImageAllocations[i]);
    }
}

void Engine::destroyColorResources() {
    for (int i = 0; i < colorImages.size(); i++) {
        vkDestroyImageView(device, colorImageViews[i], nullptr);
        vmaDestroyImage(memoryAllocator, colorImages[i], colorImageAllocations[i]);
    }
}

void Engine::createFramebuffers() {
    swapChainFramebuffers.resize(swapChainImageViews.size());

    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
        std::array<VkImageView, 6> attachments = {
            subpassImageViews[i],
            depthImageViews[i],
            swapChainImageViews[i],
            bgSubpassImageViews[i],
            colorImageViews[i],
            depthGuiImageViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = attachments.size();
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        vulkanErrorGuard(vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]), "Failed to create framebuffer.");
    }
}

// TODO Eliminate this in favor of using vmaCreateBuffer
void Engine::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer,
    VkDeviceMemory& bufferMemory, bool concurrent, std::vector<uint32_t> queues) {

    VkBufferCreateInfo bufferInfo {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = concurrent ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
    if (concurrent) {
        bufferInfo.queueFamilyIndexCount = queues.size();
        bufferInfo.pQueueFamilyIndices = queues.data();
    }

    vulkanErrorGuard(vkCreateBuffer(device, &bufferInfo, nullptr, &buffer), "Failed to create buffer.");

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    vulkanErrorGuard(vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory), "Failed to allocate buffer memory.");

    vkBindBufferMemory(device, buffer, bufferMemory, 0);

    // std::cout << std::hex << "Buffer: " << buffer << "\n" << boost::stacktrace::stacktrace() << std::endl;
}

void Engine::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
    VkCommandBuffer commandBuffer = engineSettings.useConcurrentTransferQueue ? beginSingleTimeCommands(transferCommandPool) : beginSingleTimeCommands();

    VkBufferImageCopy region {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = { width, height, 1 };

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    if (engineSettings.useConcurrentTransferQueue) endSingleTimeCommands(commandBuffer, transferCommandPool, transferQueue);
    else endSingleTimeCommands(commandBuffer);
}

void Engine::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkCommandPool pool, VkQueue queue) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(pool);

    VkBufferCopy copyRegion {};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(commandBuffer, pool, queue);
}

void Engine::createModelBuffers() {
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    // Vertex buffer
    VkDeviceSize bufferSize = currentScene->vertexBuffer.size() * sizeof(Utilities::Vertex);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, currentScene->vertexBuffer.data(), bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory,
        engineSettings.useConcurrentTransferQueue, { physicalDeviceIndices.graphicsFamily.value(), physicalDeviceIndices.transferFamily.value() });

    copyBuffer(stagingBuffer, vertexBuffer, bufferSize,
        engineSettings.useConcurrentTransferQueue ? transferCommandPool : commandPool,
        engineSettings.useConcurrentTransferQueue ? transferQueue : graphicsQueue);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);

    // Index buffer
    bufferSize = currentScene->indexBuffer.size() * sizeof(uint32_t);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer, stagingBufferMemory);

    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, currentScene->indexBuffer.data(), bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory,
        engineSettings.useConcurrentTransferQueue, { physicalDeviceIndices.graphicsFamily.value(), physicalDeviceIndices.transferFamily.value() });

    copyBuffer(stagingBuffer, indexBuffer, bufferSize,
        engineSettings.useConcurrentTransferQueue ? transferCommandPool : commandPool,
        engineSettings.useConcurrentTransferQueue ? transferQueue : graphicsQueue);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void Gui::createBuffers(size_t initialSize) {
    VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufferInfo.size = initialSize * sizeof(GuiVertex);
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    VmaAllocationCreateInfo bufferAllocationInfo = {};
    bufferAllocationInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
    bufferAllocationInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

    for (int i = 0; i < 2; i++) {
        if (vmaCreateBuffer(context->memoryAllocator, &bufferInfo, &bufferAllocationInfo, &gpuBuffers[i], &gpuAllocations[i], nullptr) !=VK_SUCCESS)
            throw std::runtime_error("Unable to allocate memory for hud buffer.");
    }
}

void Gui::destroyBuffer(int index) {
    vmaDestroyBuffer(context->memoryAllocator, gpuBuffers[index], gpuAllocations[index]);
}

void Gui::reallocateBuffer(int index, size_t newSize) {
    std::cout << "Reallocating hud buffer" << std::endl;

    destroyBuffer(index);

    VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufferInfo.size = newSize * sizeof(GuiVertex);
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    VmaAllocationCreateInfo bufferAllocationInfo = {};
    bufferAllocationInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
    bufferAllocationInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

    if (vmaCreateBuffer(context->memoryAllocator, &bufferInfo, &bufferAllocationInfo, &gpuBuffers[index], &gpuAllocations[index], nullptr) !=VK_SUCCESS)
        throw std::runtime_error("Unable to allocate memory for hud buffer.");
}

// This one just searches the vertex buffer oblivious to the gui component tree, this means we can run it directly on the buffer
uint32_t Gui::idUnderPoint(float x, float y) {
    std::scoped_lock l(dataMutex);
    uint32_t ret = 0; // the root has id 0 and will get all clicks that are orphans
    float maxLayer = 1.0;
    for(int i = Gui::dummyVertexCount; i < usedSizes[whichBuffer]; i += 6){
        GuiVertex *vertex = static_cast<GuiVertex *>(gpuAllocations[whichBuffer]->GetMappedData()) + i;
        if(
            vertex->pos.z < maxLayer &&
            vertex->pos.x < x &&
            vertex->pos.y < y &&
            (vertex + 1)->pos.x > x &&
            (vertex + 1)->pos.y > y) {
            // std::cout << buffer[i].pos.z << std::endl;
            ret = vertex->guiID;
            maxLayer = vertex->pos.z;
        }
    }
    return ret;
}

void Gui::rebuildBuffer(bool texturesDirty) {
    std::vector<std::shared_ptr<GuiTexture>> buildingTextures;
    int idx = 0;
    std::vector<GuiVertex> buildingVertices;
    std::map<uint32_t, uint> componentIdToBufferMap;
    uint otherIdx = dummyCompomentCount;

    root->mapTextures(buildingTextures, idx);
    root->buildVertexBuffer(buildingVertices, componentIdToBufferMap, otherIdx);

    std::scoped_lock lock(dataMutex);
    whichBuffer ^= 1;
    needTextureSync = texturesDirty;
    textures = buildingTextures;
    idToBuffer = componentIdToBufferMap;
    usedSizes[whichBuffer] = Gui::dummyVertexCount + buildingVertices.size();
    if (buildingVertices.size() + Gui::dummyVertexCount > gpuSizes[whichBuffer]) {
        reallocateBuffer(whichBuffer, buildingVertices.size() + Gui::dummyVertexCount);
        gpuSizes[whichBuffer] = Gui::dummyVertexCount + buildingVertices.size();
    }

    memcpy(static_cast<GuiVertex *>(gpuAllocations[whichBuffer]->GetMappedData()) + Gui::dummyVertexCount, buildingVertices.data(),
        buildingVertices.size() * sizeof(GuiVertex));

    rebuilt = true;
}

void Engine::initVulkan() {
    enableValidationLayers();
    initInstance();
    pickPhysicalDevice();
    setupLogicalDevice();


    VkPipelineCacheCreateInfo pipelineCacheCI = {};
    pipelineCacheCI.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    // pipelineCacheCI.flags = VK_PIPELINE_CACHE_CREATE_EXTERNALLY_SYNCHRONIZED_BIT_EXT;
    pipelineCacheCI.flags = 0;
    pipelineCacheCI.pNext = nullptr;
    pipelineCacheCI.pInitialData = nullptr;
    pipelineCacheCI.initialDataSize = 0;

    vulkanErrorGuard(vkCreatePipelineCache(device, &pipelineCacheCI, nullptr, &pipelineCache), "Unable to create pipeline cache.");

    createSwapChain();
    framebufferResized = false;
    createImageViews();
    createRenderPass();

    createComputeResources();
    createDescriptorSetLayout();
    createGraphicsPipelines();
    createCommandPools();

    // this needs to match the size of the swapchain rather than the command buffer actually
    // Lets try using the same descriptor layout as the main render pass to make the code more simple
    createShadowResources(true);

    createDepthResources();
    createColorResources();
    createFramebuffers();

    // this sets the currentScene pointer
    loadDefaultScene();
}

void Engine::init() {
    initWidow();
    initVulkan();
    sound = new Sound();
}

// TODO Name this better (What is this calculated value called anyways?)
template<typename T> static float whichSideOfPlane(const T& n, float d, const T& x) {
    return glm::dot(n, x) + d;
}

// Note that this is signed and N HAS TO BE NORMALIZED
static float distancePointToPlane(const glm::vec3& n, float d, const glm::vec3& x) {
    return (glm::dot(n, x) + d) / glm::l2Norm(n);
}

// creates the frustum, but not actually a frustum, bounding planes (passing the straffing vector saves having to recalculate it)
static std::array<std::pair<glm::vec3, float>, 5> boundingPlanes(const glm::vec3& pos, const glm::vec3& strafing, const glm::vec3& normedPointing,
    const glm::vec3& r1, const glm::vec3& r2) {
    std::array<std::pair<glm::vec3, float>, 5> ret;

    auto n = normalize(cross(strafing, r1));
    ret[0] = { n, -distancePointToPlane(n, 0, pos) };

    n = normalize(cross(cross(strafing, normedPointing), r1));
    ret[1] = { n, -distancePointToPlane(n, 0, pos) };

    n = normalize(cross(strafing, r2));
    ret[2] = { n, -distancePointToPlane(n, 0, pos) };

    n = normalize(cross(cross(strafing, normedPointing), r2));
    ret[3] = { n, -distancePointToPlane(n, 0, pos) };

    ret[4] = { normedPointing, -distancePointToPlane(normedPointing, 0, pos) };

    return ret;
}

inline std::pair<float, float> Engine::normedDevice(float x, float y) {
    return { x / framebufferSize.width * 2.0f - 1.0, y / framebufferSize.height * 2.0f - 1.0f };
}

inline glm::vec3 Engine::raycast(float x, float y, glm::mat4 inverseProjection, glm::mat4 inverseView) {
    return raycastDevice(normedDevice(x, y), inverseProjection, inverseView);
}

glm::vec3 Engine::raycastDevice(float normedDeviceX, float normedDeviceY, glm::mat4 inverseProjection, glm::mat4 inverseView) {
    auto homogeneousRay = glm::vec4(-normedDeviceX, -normedDeviceY, 1.0f, 1.0f);
    auto eyeRay = inverseProjection * homogeneousRay;
    eyeRay.z = 1.0f;
    eyeRay.w = 0.0f;
    auto worldRay = inverseView * eyeRay;
    glm::vec3 ray = { worldRay.x, worldRay.y, worldRay.z };
    return -ray;
}

inline glm::vec3 Engine::raycastDevice(std::pair<float, float> normedDevice, glm::mat4 inverseProjection, glm::mat4 inverseView) {
    return raycastDevice(normedDevice.first, normedDevice.second, inverseProjection, inverseView);
}

void LineHolder::addCircle(const glm::vec3& center, const glm::vec3& normal, const float radius, const uint segmentCount, const glm::vec4& color) {
    auto theta = 2.0f * M_PI / (float)segmentCount;
    auto rot = toMat3(rotationVector({ 0.0f, 0.0f, 1.0f }, normal));
    glm::vec3 prev, curr;
    for (uint i = 0; i <= segmentCount; i++) {
        if (i == 0) {
            prev = center + rot * glm::vec3({ cosf((float)i * theta), sinf((float)i * theta), 0.0f });
        } else {
            curr = center + rot * glm::vec3({ cosf((float)i * theta), sinf((float)i * theta), 0.0f });;
            lines.push_back({ prev, curr, color, color });
            prev = std::move(curr);
        }
    }
}

void LineHolder::addAxes(float length, const glm::vec4& orginColor, const glm::vec4& peripheryColor) {
    lines.push_back({{ 0.0f, 0.0f, 0.0f }, {  length, 0.0f, 0.0f }, orginColor, peripheryColor });
    lines.push_back({{ 0.0f, 0.0f, 0.0f }, { -length, 0.0f, 0.0f }, orginColor, peripheryColor });
    lines.push_back({{ 0.0f, 0.0f, 0.0f }, { 0.0f,  length, 0.0f }, orginColor, peripheryColor });
    lines.push_back({{ 0.0f, 0.0f, 0.0f }, { 0.0f, -length, 0.0f }, orginColor, peripheryColor });
    lines.push_back({{ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f,  length }, orginColor, peripheryColor });
    lines.push_back({{ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -length }, orginColor, peripheryColor });
}

namespace GuiTextures {
    static std::shared_ptr<GuiTexture> defaultTexture;
    static int maxGlyphHeight;

    static std::mutex textureCacheMutex;
    // Idk if I should build garbage collecting into this so it does not tie up gpu resources by keeping them cached indefinately
    // It is pretty easy to do
    static std::map<std::string, std::shared_ptr<GuiTexture>> textureCache;
}

void Engine::setTooltip(std::string&& str) {
    assert(!tooltipJob);
    ComputeOp op { ComputeKind::TEXT, new std::string(std::move(str)) };
    op.deleteData = true;
    tooltipJob = manager->submitJob(std::move(op));
}

void Engine::setTooltip(const std::string& str) {
    assert(!tooltipJob);
    ComputeOp op { ComputeKind::TEXT, new std::string(std::move(str)) };
    op.deleteData = true;
    tooltipJob = manager->submitJob(std::move(op));
}


// So many silly lines of code in this function
// TODO I need to rewrite this as traversals of a state graph (I am putting off doing this until maintaining this function becomes absolutely terrible)
void Engine::handleInput() {
    bool didCameraMovement = false;

    assert(!wasZPlacing || !wasZMoving);
    if (wasZMoving && placingStructure) {
        wasZMoving = false;
        wasZPlacing = true;
    }

    static bool notFirstTime;
    cursorLines->lines.clear();
    std::vector<uint32_t> idsSelected = this->idsSelected;

    Gui::GuiMessage message;
    while (gui->guiMessages.pop(message)) {
        if (message.signal == Gui::ENG_SETTOOLTIP) {
            if (!tooltipJob) setTooltip(message.data->str);
            delete message.data;
            drawTooltip = true;
        } else if (message.signal == Gui::ENG_CAPTURE) {
            keyboardCaptured = message.data->bl;
            delete message.data;
        }
    }

    double x, y;
    glfwGetCursorPos(window, &x, &y);
    float deltaX = (lastMousePosition.x - x) / framebufferSize.width;
    float deltaY = (lastMousePosition.y - y) / framebufferSize.height;

    static std::chrono::steady_clock::time_point lastMouseMovementTime;
    static bool emittedHover;
    static uint hoveredId;
    if (!notFirstTime) hoveredId = UINT32_MAX;

    if (lastMousePosition.x - x == 0 && lastMousePosition.y - y == 0) {
        if (!emittedHover && std::chrono::steady_clock::now() - lastMouseMovementTime > std::chrono::seconds(1)) {
            auto what = new GuiCommandData();
            what->id = gui->pushConstant.guiID;
            what->position.x.asFloat = lastMousePosition.normedX;
            what->position.y.asFloat = lastMousePosition.normedY;
            what->flags = 0;
            gui->submitCommand({ Gui::GUI_HOVER, what });
            emittedHover = true;
            hoveredId = what->id;
        }
    } else {
        lastMouseMovementTime = std::chrono::steady_clock::now();
        if (gui->pushConstant.guiID != hoveredId) emittedHover = false;
    }

    auto inverseProjection = glm::inverse(pushConstants.projection);
    auto inverseView = glm::inverse(pushConstants.view);
    auto mouseNormed = normedDevice(x, y);
    auto mouseRay = raycast(x, y, inverseProjection, inverseView);
    auto mouseRayNormed = normalize(mouseRay);
    // hudConstants.dragBox[1] = { mouseNormed.first, mouseNormed.second };

    lastMousePosition.normedX = mouseNormed.first;
    lastMousePosition.normedY = mouseNormed.second;

    gui->pushConstant.tooltipBox[0] = { mouseNormed.first + 0.05, mouseNormed.second + 0.04 };
    // whatever, just pick one
    auto tooltipWideness = tooltipResource ? tooltipResource->widenessRatio : 0.0f;
    gui->pushConstant.tooltipBox[1] = { mouseNormed.first + 0.05 + 0.13 * tooltipWideness, mouseNormed.second + 0.17 };

    // Do I want this?
    for (int i = 0; i < 8; i++) {
        if (mouseButtonsPressed[i]) {
            mouseButtonsPressed[i] = false;
        }
    }
    KeyEvent keyEvent;
    while(keyboardInput.pop(keyEvent)) {
        if (!keyboardCaptured && (keyEvent.action == GLFW_PRESS || keyEvent.action == GLFW_REPEAT)) {
            keysPressed[keyEvent.key] = true;
            if (keyEvent.key == GLFW_KEY_RIGHT_BRACKET) {
                quit();
                return;
            } else if (keyEvent.key == GLFW_KEY_B) {
                std::raise(SIGTRAP);
            } else if (keyEvent.key == GLFW_KEY_S) {
                InsertionMode mode = InsertionMode::OVERWRITE;
                if (keyEvent.mods & GLFW_MOD_SHIFT) mode = InsertionMode::BACK;
                for ( const auto id : idsSelected) Api::cmd_stop(id, mode);
            } else if (keyEvent.key == GLFW_KEY_ESCAPE) {
                apiLocks[APIL_CURSOR_INSTANCE].lock();
                setCursorInstance(Instance());
                apiLocks[APIL_CURSOR_INSTANCE].unlock();
            }
            apiLocks[APIL_KEYBINDINGS].lock();
            bool callLuaHandler = luaKeyBindings.contains(keyEvent.key);
            apiLocks[APIL_KEYBINDINGS].unlock();
            if (callLuaHandler) {
                GuiCommandData *what = new GuiCommandData();
                what->id = keyEvent.key;
                what->flags = keyEvent.mods;
                gui->submitCommand({ Gui::GUI_KEYBINDING, what });
            }
        }
        if (keyEvent.action == GLFW_RELEASE) {
            keysPressed[keyEvent.key] = false;
        }
    }
    // TODO This whole thing could be more effeciently calculated (when I feel like being clever I will come back and fix it)
    const auto pointingLength = length(camera.position - camera.target);

    if (!notFirstTime) {
        camera.pointing = normalize(camera.position - camera.target);
        camera.strafing = normalize(cross(camera.pointing, { 0.0f, 0.0f, 1.0f }));
        camera.fowarding = normalize(cross(camera.strafing, { 0.0f, 0.0f, 1.0f }));
        camera.heading = normalize(cross(camera.pointing, camera.strafing));
        sound->setCameraPosition(camera.position, camera.heading, camera.pointing, { 0.0f, 0.0f, 0.0f });
    }

    const float planeIntersectionDenominator = glm::dot(mouseRay, { 0.0f, 0.0f, 1.0f });
    float planeDist;
    glm::vec3 planeIntersection;
    if (planeIntersectionDenominator == 0) {
        planeIntersection = glm::vec3(0.0);
    } else {
        planeDist = - (glm::dot(camera.position, { 0.0f, 0.0f, 1.0f }) + 0) / planeIntersectionDenominator;
        planeIntersection = camera.position + mouseRay * planeDist;
    }

    if (mouseAction == MOUSE_DRAGGING) {
        gui->setDragBox(dragStartDevice, mouseNormed);
    }

    Instance *mousedOver = nullptr;
    canAttack = false;
    mousedOverId = 0;
    float minDistance = std::numeric_limits<float>::max();

    for(int i = 1; i < currentScene->state.instances.size(); i++) {
        if (!currentScene->state.instances[i]->inPlay) continue;
        float distance;
        if(currentScene->state.instances[i]->rayIntersects(camera.position, mouseRayNormed, distance)) {
            if (distance < minDistance) mousedOver = currentScene->state.instances[i];
        }
        // instance.highlight() = false;
    }

    if(mousedOver) {
        // mousedOver->highlight() = true;
        mousedOverId = mousedOver->id;
        glfwSetCursor(window, cursors[CursorIndex::CURSOR_SELECT]);
        bool tooltipStatus = true;
        for (const auto d : tooltipDirty) if (d) tooltipStatus = false;
        if (tooltipStatus) {
            std::ostringstream tooltipText;
            tooltipText << std::fixed << std::setprecision(2) << mousedOver->position << "\n RUs: " << mousedOver->resources << "\nID: " 
                << std::dec << mousedOver->id << " <- " << mousedOver->parentID;
            if (!tooltipJob) setTooltip(tooltipText.str());
        }
        drawTooltip = true;
    } else {
        glfwSetCursor(window, cursors[CursorIndex::CURSOR_DEFAULT]);
        if (!emittedHover) drawTooltip = false;
    }
    if (planeIntersectionDenominator != 0 && planeDist > 0 && mouseAction == MOUSE_NONE && !idsSelected.empty()) {
        cursorLines->addCircle(planeIntersection, { 0.0f, 0.0f, 1.0f }, 1.0f, 20, glm::vec4({ 0.3f, 1.0f, 0.3f, 1.0f }));
        apiLocks[APIL_CURSOR_INSTANCE].lock();
        cursorInstance.position = planeIntersection;
        apiLocks[APIL_CURSOR_INSTANCE].unlock();
    }

    static glm::vec3 zPlaneNormal;

    bool idsSelectedChanged = false;
    MouseEvent mouseEvent;
    while (mouseInput.pop(mouseEvent)) {
        if (mouseEvent.action == GLFW_PRESS && mouseEvent.mods & GLFW_MOD_SHIFT && mouseEvent.button == GLFW_MOUSE_BUTTON_MIDDLE) {
            if (mouseAction == MOUSE_MOVING_Z) {
                wasZMoving = true;
            }
            if (mouseAction == MOUSE_PLACING_Z) {
                wasZPlacing = true;
            }
            mouseAction = MOUSE_PANNING;
        } else if (mouseEvent.action == GLFW_PRESS && mouseEvent.button == GLFW_MOUSE_BUTTON_MIDDLE) {
            if (mouseAction == MOUSE_MOVING_Z) {
                wasZMoving = true;
            }
            if (mouseAction == MOUSE_PLACING_Z) {
                wasZPlacing = true;
            }
            mouseAction = MOUSE_ROTATING;
        }
        if (mouseEvent.action == GLFW_RELEASE && mouseEvent.button == GLFW_MOUSE_BUTTON_MIDDLE) {
            if (wasZMoving) {
                mouseAction = MOUSE_MOVING_Z;
                wasZMoving = false;
            } else {
                mouseAction = MOUSE_NONE;
            }
            if (wasZPlacing) {
                mouseAction = MOUSE_PLACING_Z;
                wasZPlacing = false;
            } else {
                mouseAction = MOUSE_NONE;
            }
        }
        if (mouseAction == MOUSE_NONE && mouseEvent.action == GLFW_PRESS && mouseEvent.button == GLFW_MOUSE_BUTTON_LEFT && gui->pushConstant.guiID == 0) {
            mouseAction = MOUSE_DRAGGING;
            // Unless the framerate is crap this should be almost identical to the mouseRay
            // dragStartRay = raycast(mouseEvent.x, mouseEvent.y, inverseProjection, inverseView);
            dragStartRay = mouseRayNormed;
            dragStartDevice = mouseNormed;
            // hudConstants.dragBox[0] = { mouseNormed.first, mouseNormed.second };
        }
        if (mouseAction == MOUSE_DRAGGING && mouseEvent.action == GLFW_RELEASE && mouseEvent.button == GLFW_MOUSE_BUTTON_LEFT) {
            gui->setDragBox({ 0.0f, 0.0f }, { 0.0f, 0.0f });
            mouseAction = MOUSE_NONE;
            // std::cout << "Dragged box (" << dragStartDevice.first << ":" << dragStartDevice.second << " - "
            //     << mouseNormed.first << ":" << mouseNormed.second << "):" << std::endl;
            auto planes = boundingPlanes(camera.position, camera.strafing, camera.pointing, dragStartRay, mouseRayNormed);
            // auto planes = boundingPlanes(camera.position, strafing, pointing, dragStartRay, raycast(mouseEvent.x, mouseEvent.y, inverseProjection, inverseView));
            // for(const auto& plane : planes) {
            //     std::cout << "\t" << plane.first << " --- " << plane.second << std::endl;
            // }
            idsSelected.clear();
            selectionCanAttack = false;
            idsSelectedChanged = true;
            for (int i = 0; i < currentScene->state.instances.size(); i++) {
                if (!currentScene->state.instances[i]->valid || !currentScene->state.instances[i]->inPlay || !currentScene->state.instances[i]->entity->isUnit) continue;
                if (engineSettings.teamIAm.id && engineSettings.teamIAm.id != currentScene->state.instances[i]->team) continue;
                if (whichSideOfPlane(planes[4].first, planes[4].second - Config::Camera.minClip, currentScene->state.instances[i]->position) < 0 &&
                    whichSideOfPlane(planes[0].first, planes[0].second, currentScene->state.instances[i]->position) < 0 !=
                    whichSideOfPlane(planes[2].first, planes[2].second, currentScene->state.instances[i]->position) < 0 &&
                    whichSideOfPlane(planes[1].first, planes[1].second, currentScene->state.instances[i]->position) < 0 !=
                    whichSideOfPlane(planes[3].first, planes[3].second, currentScene->state.instances[i]->position) < 0 &&
                    whichSideOfPlane(planes[4].first, planes[4].second + Config::Camera.maxClip, currentScene->state.instances[i]->position) > 0
                    // To only select things as far as the camera drawing distance
                ) {
                    currentScene->state.instances[i]->highlight = true;
                    idsSelected.push_back(currentScene->state.instances[i]->id);
                    if (!selectionCanAttack) selectionCanAttack = currentScene->state.instances[i]->canAttack();
                } else {
                    currentScene->state.instances[i]->highlight = false;
                }
            }
            if (mousedOver && engineSettings.teamIAm.id &&
                engineSettings.teamIAm.id == mousedOver->team) {
                idsSelected.push_back(mousedOver->id);
                if (!selectionCanAttack) selectionCanAttack = mousedOver->canAttack();
            }
        }
        if (mouseEvent.action == GLFW_PRESS && mouseEvent.button == GLFW_MOUSE_BUTTON_RIGHT) {
            if (planeIntersectionDenominator != 0 && mouseAction == MOUSE_NONE && planeDist > 0 && !idsSelected.empty()) {
                if (cursorInstance.valid) {
                    mouseAction = MOUSE_PLACING_Z;
                } else {
                    mouseAction = MOUSE_MOVING_Z;
                }
                movingTo.x = planeIntersection.x;
                movingTo.y = planeIntersection.y;
                zPlaneNormal = normalize(cross({ 0.0f, 0.0f, 1.0f }, cross({ 0.0f, 0.0f, 1.0f }, mouseRayNormed)));
            }
        } else if (mouseEvent.action == GLFW_RELEASE && mouseEvent.button == GLFW_MOUSE_BUTTON_RIGHT && (mouseAction == MOUSE_MOVING_Z || mouseAction == MOUSE_PLACING_Z)) {
            for (const auto id : idsSelected) {
                auto mode = InsertionMode::OVERWRITE;
                if (mouseEvent.mods & GLFW_MOD_SHIFT) mode = InsertionMode::BACK;
                if (mouseEvent.mods & GLFW_MOD_ALT) mode = InsertionMode::FRONT;
                if (mouseAction == MOUSE_MOVING_Z) {
                    Api::cmd_move(id, { movingTo.x, movingTo.y, movingTo.z }, mode, true);
                } else {
                    if (cursorInstance.valid && unitDoingStationBuilding) {
                        Api::cmd_buildStation(unitDoingStationBuilding, movingTo, mode, cursorInstance.entity->name.c_str());
                        if (mode == InsertionMode::OVERWRITE) {
                            apiLocks[APIL_CURSOR_INSTANCE].lock();
                            setCursorInstance(Instance());
                            apiLocks[APIL_CURSOR_INSTANCE].unlock();
                            break;
                        }
                    }
                }
            }
            mouseAction = MOUSE_NONE;
        }
    }

    if (scrollAmount != 0.0f) {
        // Scroll to camera target
        // camera.position += scrollAmount / 5.0f * pointing;
        // if (glm::distance(camera.target, camera.position) > camera.maxZoom2) camera.position = pointing * camera.maxZoom2 + camera.target;
        // if (glm::distance(camera.target, camera.position) < camera.minZoom2) camera.position = pointing * camera.minZoom2 + camera.target;
        // scrollAmount = 0.0f;

        // scrollToCursor
        scrollAmount *= std::max(powf(1.5f, pointingLength / 10.0f), 1.5f);
        auto deltaPos = -scrollAmount / 5.0f * mouseRayNormed;
        auto deltaTarget = normalize(planeIntersection - camera.target) * (length(deltaPos) * length(planeIntersection - camera.target) / planeDist);
        auto newTar = camera.target + deltaTarget * sgn(-scrollAmount);
        auto newPos = camera.position + deltaPos;
        auto newDelta = length2(newTar - newPos);
        if (newDelta > Config::Camera.minZoom2 && (newDelta < Config::Camera.maxZoom2 || scrollAmount < 0)) {
            camera.target = newTar;
            camera.position = newPos;
            didCameraMovement = true;
        }
        if (newDelta > Config::Camera.maxZoom2 && scrollAmount > 0) {
            // linear interpolation to get to max zoom
            auto d = sqrtf(newDelta);
            auto m = sqrtf(Config::Camera.maxZoom2);
            auto c = distance(camera.target, camera.position);
            auto ratio = (m - c) / (d - c);
            camera.target += (newTar - camera.target) * ratio;
            camera.position += (newPos - camera.position) * ratio;
            didCameraMovement = true;
        }

        scrollAmount = 0.0f;
    }

    if (!keyboardCaptured) {
        if (keysPressed[GLFW_KEY_RIGHT]) {
            camera.position -= camera.strafing / 600.0f;
            camera.target -= camera.strafing / 600.0f;
            didCameraMovement = true;
        }
        if (keysPressed[GLFW_KEY_LEFT]) {
            camera.position += camera.strafing / 600.0f;
            camera.target += camera.strafing / 600.0f;
            didCameraMovement = true;
        }
        if (keysPressed[GLFW_KEY_DOWN]) {
            camera.position -= camera.fowarding / 600.0f;
            camera.target -= camera.fowarding / 600.0f;
            didCameraMovement = true;
        }
        if (keysPressed[GLFW_KEY_UP]) {
            camera.position += camera.fowarding / 600.0f;
            camera.target += camera.fowarding / 600.0f;
            didCameraMovement = true;
        }
    }

    if (mouseAction == MOUSE_PANNING) {
        camera.position -= camera.strafing * 4.0f * deltaX * pointingLength / 5.2;
        camera.target -= camera.strafing * 4.0f * deltaX * pointingLength / 5.2;
        camera.position -= camera.fowarding * 4.0f * deltaY / (sgn(camera.position.z) * std::clamp(fabs(camera.position.z) / pointingLength,
            0.5f, std::numeric_limits<float>::max())) * pointingLength / 5.2;
        camera.target -= camera.fowarding * 4.0f * deltaY / (sgn(camera.position.z) * std::clamp(fabs(camera.position.z) / pointingLength,
            0.5f, std::numeric_limits<float>::max())) * pointingLength / 5.2;
    } else if (mouseAction == MOUSE_ROTATING) {
        if (deltaY != 0.0f) {
            auto mouseTilt = glm::angleAxis(glm::radians(0 - deltaY * 400.0f) / 2.0f, camera.strafing);
            auto mouseTiltMatrix = glm::toMat3(mouseTilt);
            auto newPosition = mouseTiltMatrix * (camera.position - camera.target) + camera.target;
            if (/*(newPosition.z - camera.target.z) > camera.gimbleStop && */(newPosition.x - camera.target.x) * (newPosition.x - camera.target.x) +
                (newPosition.y - camera.target.y) * (newPosition.y - camera.target.y) > Config::Camera.gimbleStop) {
                    camera.position = newPosition;
                    didCameraMovement = true;
            }
        }
        if (deltaX != 0.0f) {
            auto mouseOrbit = glm::angleAxis(glm::radians(deltaX * 400.0f) / 2.0f, glm::vec3({ 0.0f, 0.0f, 1.0f }));
            auto mouseOrbitMatrix = glm::toMat3(mouseOrbit);
            camera.position = mouseOrbitMatrix * (camera.position - camera.target) + camera.target;
            didCameraMovement = true;
        }
    }

    if (mouseAction == MOUSE_MOVING_Z || mouseAction == MOUSE_PLACING_Z) {
        if (planeIntersectionDenominator != 0.0f) {
            float dist;
            if (glm::intersectRayPlane(camera.position, mouseRayNormed, { movingTo.x, movingTo.y, 0.0f }, zPlaneNormal, dist)) {
                auto intersection = camera.position + mouseRayNormed * dist;
                movingTo.z = intersection.z;
                apiLocks[APIL_CURSOR_INSTANCE].lock();
                cursorInstance.position = movingTo;
                apiLocks[APIL_CURSOR_INSTANCE].unlock();
                if (mouseAction == MOUSE_MOVING_Z) {
                    cursorLines->addCircle(movingTo, { 0.0f, 0.0f, 1.0f }, 1.0f, 20, glm::vec4({ 0.3f, 1.0f, 0.3f, 1.0f }));
                }
                cursorLines->addCircle({ movingTo.x, movingTo.y, 0 }, { 0.0f, 0.0f, 1.0f }, 1.0f, 20, glm::vec4({ 0.1f, 0.7f, 0.1f, 1.0f }));
            }
        }
    }

    if (idsSelectedChanged) {
        apiLocks[APIL_SELECTION].lock();
        this->idsSelected = idsSelected;
        apiLocks[APIL_SELECTION].unlock();
        GuiCommandData *what = new GuiCommandData();
        what->str = "onSelectionChanged";
        gui->submitCommand({ Gui::GUI_NOTIFY, what });
    }

    if (mousedOver && mouseAction == MOUSE_NONE && selectionCanAttack && mousedOver->team != engineSettings.teamIAm.id) {
        glfwSetCursor(window, cursors[CursorIndex::CURSOR_ATTACK]);
        canAttack = true;
    }

    lastMousePosition.x = x;
    lastMousePosition.y = y;

    notFirstTime = false;

    if (didCameraMovement) {
        camera.pointing = normalize(camera.position - camera.target);
        camera.strafing = normalize(cross(camera.pointing, { 0.0f, 0.0f, 1.0f }));
        camera.fowarding = normalize(cross(camera.strafing, { 0.0f, 0.0f, 1.0f }));
        camera.heading = normalize(cross(camera.pointing, camera.strafing));
        sound->setCameraPosition(camera.position, camera.heading, camera.pointing, { 0.0f, 0.0f, 0.0f });
    }
}

static std::array<std::pair<glm::vec3, float>, 6> extractFrustum(const glm::mat4& m) {
    return {{
        {{ m[0][3] + m[0][0] , m[1][3] + m[1][0] , m[2][3] + m[2][0] }, m[3][3] + m[3][0] },
        {{ m[0][3] - m[0][0] , m[1][3] - m[1][0] , m[2][3] - m[2][0] }, m[3][3] - m[3][0] },
        {{ m[0][3] + m[0][1] , m[1][3] + m[1][1] , m[2][3] + m[2][1] }, m[3][3] + m[3][1] },
        {{ m[0][3] - m[0][1] , m[1][3] - m[1][1] , m[2][3] - m[2][1] }, m[3][3] - m[3][1] },
        {{ m[0][2] , m[1][2] , m[2][2] }, m[3][2] },
        {{ m[0][3] - m[0][2] , m[1][3] - m[1][2] , m[2][3] - m[2][2] }, m[3][3] - m[3][2] }
    }};
}

static bool frustumContainsPoint(const std::array<std::pair<glm::vec3, float>, 6>& frustum, glm::vec3 point) {
    for(const auto& plane : frustum)
        if (whichSideOfPlane(plane.first, plane.second, point) < 0) return false;
    return true;
}

template<typename T> static T normalizePlanes(const T& planes) {
    T ret;
    for(int i = 0; i < planes.size(); i++) {
        auto len = length(planes[i].first);
        ret[i] = { planes[i].first / len, planes[i].second / len };
    }
    return ret;
}

static bool frustumContainsSphere(const std::array<std::pair<glm::vec3, float>, 6>& normedFrustum, glm::vec3 origin, float radius) {
    for(const auto& plane : normedFrustum)
        if (whichSideOfPlane(plane.first, plane.second, origin) < -radius) return false;
    return true;
}

// The index is for which descriptor index to put the lighting buffer information
// Here is the big graphics update function
float Engine::updateScene(int index) {
    // TODO This is where lag checking and compensation goes
    currentScene->state.syncToAuthoritativeState(authState);

    auto nowTime = std::chrono::steady_clock::now();
    float delta = std::chrono::duration<float, std::chrono::seconds::period>(nowTime - lastTime).count();
    lastTime = nowTime;

    pushConstants.view = glm::lookAt(camera.position, camera.target, glm::vec3(0.0f, 0.0f, 1.0f));
    pushConstants.projection = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float) swapChainExtent.height, Config::Camera.minClip, Config::Camera.maxClip);
    pushConstants.projection[1][1] *= -1;
    // TODO I should just store this in the camera struct since I need it here and in the input handling
    pushConstants.normedPointing = normalize(camera.position - camera.target);

    camera.cached.proj_1 = inverse(pushConstants.projection);
    camera.cached.view_1 = inverse(pushConstants.view);
    camera.cached.projView = pushConstants.projection * pushConstants.view;
    camera.cached.view_1Proj_1 = camera.cached.view_1 * camera.cached.proj_1;

    // The 0th instance is always the skybox
    currentScene->state.instances[0]->position = camera.position;

    auto projView = pushConstants.projection * pushConstants.view;
    auto cameraFrustumNormed = normalizePlanes(extractFrustum(projView));
    // auto cameraFrustum = extractFrustum(projView);

    // zSortedIcons.clear();
    // It is probably better just to reserve to avoid reallocations
    // zSortedIcons.reserve(currentScene->state.instances.size());

    // needed for building an optimal orthographic projection for doing the lighting
    float xMax, xMin, yMax, yMin, zMax, zMin, boundMax;
    xMax = yMax = zMax = std::numeric_limits<float>::lowest();
    xMin = yMin = zMin = std::numeric_limits<float>::max();
    boundMax = 0;
    uint fullyRenderCount = 0;

    for(int i = 1; i < currentScene->state.instances.size(); i++) {
        if (!currentScene->state.instances[i]->valid) continue;
        if (currentScene->state.instances[i]->entity->isProjectile) {
            // I will just let the gpu do its thing and not mess about
            currentScene->state.instances[i]->rendered = true;
            continue;
        }
        const Entity& entity = *currentScene->state.instances[i]->entity;
        currentScene->state.instances[i]->cameraDistance2 = distance2(camera.position, currentScene->state.instances[i]->position);
        // currentScene->state.instances[i].cameraDistance = sqrtf(currentScene->state.instances[i].cameraDistance2);
        currentScene->state.instances[i]->renderAsIcon = currentScene->state.instances[i]->cameraDistance2 > Config::Camera.renderAsIcon2;
        // if (currentScene->state.instances[i].renderAsIcon) zSortedIcons.push_back(i);
        currentScene->state.instances[i]->rendered =
            // frustumContainsPoint(cameraFrustumNormed, currentScene->state.instances[i].position);
            frustumContainsSphere(cameraFrustumNormed, currentScene->state.instances[i]->position, entity.boundingRadius);
        if (currentScene->state.instances[i]->rendered && !currentScene->state.instances[i]->renderAsIcon) {
            auto inLightingViewSpace = lightingData.view * glm::vec4{ currentScene->state.instances[i]->position, 1.0f };
            xMax = std::max(xMax, inLightingViewSpace.x);
            yMax = std::max(yMax, inLightingViewSpace.y);
            zMax = std::max(zMax, inLightingViewSpace.z);
            xMin = std::min(xMin, inLightingViewSpace.x);
            yMin = std::min(yMin, inLightingViewSpace.y);
            zMin = std::min(zMin, inLightingViewSpace.z);
            boundMax = std::max(boundMax, entity.boundingRadius);
            fullyRenderCount++;
        }
    }

    boundMax *= 2;

    // std::cout << "Fully rendered count = " << fullyRenderCount << std::endl;

    // auto max = lightingDataView_1 * glm::vec4(xMax, yMax, zMax, 1.0);
    // auto min = lightingDataView_1 * glm::vec4(xMin, yMin, zMin, 1.0);
    // std::cout << xMax << " " << yMax << " " << zMax << " " << xMin << " " << yMin << " " << zMin << std::endl;

    // std::sort(zSortedIcons.begin(), zSortedIcons.end(), currentScene->zSorter);

    // check if we need to draw shadows (if we are only rendering icons we don't need them)

    // xMax = yMax = 0;
    // xMin = yMin = -0;
    // zMax = -105;
    // zMin = -105;

    zSorted.resize(currentScene->state.instances.size() - 1);
    std::iota(zSorted.begin(), zSorted.end(), 1);
    std::sort(zSorted.begin(), zSorted.end(), currentScene->zSorter);

    if (fullyRenderCount) {
        // lightingData.nearFar = { 0.7f, 13.0f };
        // lightingData.view = glm::lookAt(lightingData.pos, glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        // lightingData.proj = glm::perspective(glm::radians(45.0f), 1.0f, lightingData.nearFar.x, lightingData.nearFar.y);
        // lightingData.view = glm::mat4(1.0f);
        // lightingData.proj = glm::ortho(-1.0f - xMax, 1.0f + xMin, 1.0f, -1.0f, 1.0f + zMax, -1.0f + zMin);
        lightingData.nearFar = { boundMax - zMin, -boundMax - zMax };
        lightingData.proj = glm::ortho(-boundMax + xMin, boundMax + xMax, boundMax - yMin, -boundMax - yMax, boundMax - zMin, -boundMax - zMax);
        lightingData.proj[1][1] *= -1;

        shadow.constants.lightPos = lightingData.pos;
        shadow.constants.view = lightingData.view;
        shadow.constants.projection = lightingData.proj;

        updateLightingDescriptors(index, lightingData);
    }

    return delta;
}

#if (DEPTH_DEBUG_IMAGE_USAGE == VK_IMAGE_USAGE_TRANSFER_SCR_BIT)
static std::array<int, 256> mainDepthHistogram {};
#endif

// TODO The synchornization code is pretty much nonsence, it works but is bad (it require the swap chain size to match the in flight frames count)
// The fences are for the swap chain, but they also protect the command buffers since we are tripple buffering the commands (at least on my machine)
void Engine::drawFrame() {
    totalFrames++;
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain();
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        throw std::runtime_error("Failed to aquire swap chain image.");

    // update the gui vram if the gui buffer has been rebuilt
    if (gui->rebuilt) {
        auto bufRet = gui->updateBuffer();
        hudVertexCount = get<0>(bufRet);
        guiOutOfDate = false;
        guiIdToBufferIndex = get<1>(bufRet);
        hudBuffer = get<2>(bufRet);
    }
    if (hudDescriptorNeedsRewrite[currentFrame]) {
        hudDescriptorNeedsRewrite[currentFrame] = false;
        rewriteHudDescriptors(currentFrame);
    }

    if (tooltipJob) {
        ComputeOp op;
        if (manager->getJob(tooltipJob, op)) {
            tooltipJob = 0;
            tooltipStillInUse = tooltipResource;
            tooltipResource = std::shared_ptr<GuiTexture>(reinterpret_cast<GuiTexture *>(op.data));
            setTooltipTexture(currentFrame, tooltipResource);
            tooltipDirty[currentFrame] = false;
            for (int i = 0; i < concurrentFrames; i++) if (i != currentFrame) tooltipDirty[i] = true;
        }
    } else {
        if (tooltipDirty[currentFrame]) {
            setTooltipTexture(currentFrame, tooltipResource);
            tooltipDirty[currentFrame] = false;
        }
    }

    vkWaitForFences(device, 1, inFlightFences.data() + (currentFrame + concurrentFrames - 1) % concurrentFrames, VK_TRUE, UINT64_MAX);

    gui->pushConstant.guiID = gui->idUnderPoint(lastMousePosition.normedX, lastMousePosition.normedY);

    // This relies on the fence to stay synchronized
    if (shadow.debugWritePending)
        doShadowDebugWrite();
    if (depthDump.writePending)
        logDepthBufferToFile(mainDepthHistogram, "depth_map.png");
    // Wait for the previous fence
    recordCommandBuffer(commandBuffers[currentFrame], swapChainFramebuffers[imageIndex], mainPass.descriptorSets[currentFrame],
        hudDescriptorSets[currentFrame], currentFrame);

    VkSubmitInfo submitInfo {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

    VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    // reset the fence to unsignalled
    vkResetFences(device, 1, &inFlightFences[currentFrame]);

    vulkanErrorGuard(vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]), "Failed to submit draw command buffer.");

    VkPresentInfoKHR presentInfo {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { swapChain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    presentInfo.pResults = nullptr;

    result = vkQueuePresentKHR(presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
        recreateSwapChain();
        framebufferResized = false;
    } else if (result != VK_SUCCESS)
        throw std::runtime_error("Failed to present swap chain image.");

    // update the next frame

    // vkDeviceWaitIdle(device);

    apiLocks[APIL_CURSOR_INSTANCE].lock();
    cursorRender = cursorInstance;
    apiLocks[APIL_CURSOR_INSTANCE].unlock();

    float timeDelta = updateScene((currentFrame + 1)  % concurrentFrames);
    currentScene->updateUniforms((currentFrame + 1) % concurrentFrames);
    // currentScene->state.doUpdate(timeDelta);

    currentFrame = (currentFrame + 1) % concurrentFrames;
}

void Engine::allocateLightingBuffers() {
    // uniformBuffers.resize(swapChainImages.size());
    // uniformBufferAllocations.resize(swapChainImages.size());
    // uniformSkip = calculateUniformSkipForUBO<InstanceUBO>();
    lightingBuffers.resize(concurrentFrames);
    lightingBufferAllocations.resize(concurrentFrames);

    for (int i = 0; i < concurrentFrames; i++) {
        // bufferInfo.size = instanceCount * uniformSkip;
        // bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

        // bufferAllocationInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        // bufferAllocationInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

        // if (vmaCreateBuffer(memoryAllocator, &bufferInfo, &bufferAllocationInfo, uniformBuffers.data() + i, uniformBufferAllocations.data() + i, nullptr) != VK_SUCCESS)
        //     throw std::runtime_error("Unable to allocate memory for uniform buffers.");

        VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        bufferInfo.size = sizeof(Utilities::ViewProjPosNearFar);
        bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

        VmaAllocationCreateInfo bufferAllocationInfo = {};
        bufferAllocationInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        bufferAllocationInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

        if (vmaCreateBuffer(memoryAllocator, &bufferInfo, &bufferAllocationInfo, &lightingBuffers[i], &lightingBufferAllocations[i], nullptr) != VK_SUCCESS)
            throw std::runtime_error("Unable to allocate memory for uniform lighting buffers.");
    }
}

void Engine::destroyLightingBuffers() {
    for (int i = 0; i < concurrentFrames; i++) {
        vmaDestroyBuffer(memoryAllocator, lightingBuffers[i], lightingBufferAllocations[i]);
    }
}

void Engine::handleConsoleInput() {
    int flags = fcntl(0, F_GETFL, 0);
    fcntl(0, F_SETFL, flags | O_NONBLOCK);
    char buf[4096];
    std::cout << ">> ";
    while (!done) {
        try {
            std::cin.getline(buf, 4096);
            if (strlen(buf)) {
                consoleLua->doString(buf);
                std::cout << ">> ";
            }
            std::cin.clear();
            if (!strlen(buf)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        } catch (const std::exception& e) { 
            std::cerr << e.what() << std::endl;
        }
    }
}

// This function is a shmorgishborg of random stuff
void Engine::runCurrentScene() {
    // We only need to create model buffers once for each scene since we load this into vram once
    createModelBuffers();
    allocateLightingBuffers();
    createMainDescriptors(currentScene->textures, {});
    createShadowDescriptorSets();
    uniformSync = new DynUBOSyncer<InstanceUBO>(this, {{ &mainPass.descriptorSets, 0 }, { &shadow.descriptorSets, 0 }});

    VkDescriptorPoolSize linePoolSize {};
    linePoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    linePoolSize.descriptorCount = concurrentFrames;

    VkDescriptorPoolCreateInfo poolInfo {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &linePoolSize;
    poolInfo.maxSets = concurrentFrames;

    vulkanErrorGuard(vkCreateDescriptorPool(device, &poolInfo, nullptr, &mainPass.lineDescriptorPool), "Failed to create descriptor pool.");

    mainPass.lineDescriptorSets.resize(concurrentFrames);

    std::vector<VkDescriptorSetLayout> lineLayouts(concurrentFrames, mainPass.lineDescriptorLayout);
    VkDescriptorSetAllocateInfo lineAllocInfo {};
    lineAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    lineAllocInfo.descriptorPool = mainPass.lineDescriptorPool;
    lineAllocInfo.descriptorSetCount = concurrentFrames;
    lineAllocInfo.pSetLayouts = lineLayouts.data();

    vulkanErrorGuard(vkAllocateDescriptorSets(device, &lineAllocInfo, mainPass.lineDescriptorSets.data()), "Failed to allocate descriptor sets.");

    lineSync = new DynUBOSyncer<LineUBO>(this, {{ &mainPass.lineDescriptorSets, 0 }});

    allocateCommandBuffers();
    initSynchronization();
    currentFrame = 0;
    double x, y;
    glfwSetCursor(window, cursors[CursorIndex::CURSOR_DEFAULT]);
    glfwGetCursorPos(window, &x, &y);
    lastMousePosition.x = x;
    lastMousePosition.y = y;
    auto mouseNormed = normedDevice(x, y);
    lastMousePosition.normedX = mouseNormed.first;
    lastMousePosition.normedY = mouseNormed.second;

    camera.position = glm::vec3({ 20.0f, 20.0f, 20.0f });
    camera.target = glm::vec3({ 0.0f, 0.0f, 0.0f });

    GuiTextures::setDefaultTexture();
    writeHudDescriptors();
    lua->exportEnumToLua<InsertionMode>();
    lua->exportEnumToLua<IntrinicStates>();
    lua->exportEnumToLua<IEngage>();
    lua->apiExport();

    manager = new ComputeManager(this);

    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
    std::vector<char32_t> glyphsToCache;
    for (const auto& chr : converter.from_bytes(GuiTextures::glyphsToCache)) {
        glyphsToCache.push_back(chr);
    }
    glyphCache = new GlyphCache(this, glyphsToCache, engineSettings.rebuildFontCache);
    glyphCache->writeDescriptors(computeSets[CP_SDF_BLIT], 0);

    gui = new Gui(&lastMousePosition.normedX, &lastMousePosition.normedY, swapChainExtent.height, swapChainExtent.width, this);

    // we need to get stuff for the first frame so we don't crash
    currentScene->updateUniforms(0);
    auto bufRet = gui->updateBuffer();
    hudVertexCount = get<0>(bufRet);
    guiIdToBufferIndex = std::move(get<1>(bufRet));
    hudBuffer = get<2>(bufRet);
    lastTime = std::chrono::steady_clock::now();
    updateScene(0);
    vkDeviceWaitIdle(device);

    // Api::cmd_createInstance("ship", { 0.0f, 1.0f, 0.0f}, normalize(glm::quat({ 1.0f, 1.0f, 1.0f, 0.0f})), 1);

    iconModelIndex = currentScene->entities["icon"]->modelIndex;

    staticLines = new LineHolder();
    cursorLines = new LineHolder();
    lineObjects = { cursorLines, staticLines };

    staticLines->addAxes(300, { 0.7f, 0.1f, 0.1f, 1.0f }, { 0.4f, 0.05f, 0.05f, 0.7f });

    gui->pushConstant.tooltipBox[0] = { -0.2, -0.2 };
    gui->pushConstant.tooltipBox[1] = {  0.2,  0.2 };

    tooltipResource = GuiTextures::defaultTexture;
    tooltipDirty.resize(concurrentFrames);
    for (int i = 0; i < concurrentFrames; i++) {
        tooltipDirty[i] = false;
        setTooltipTexture(i, GuiTextures::defaultTexture);
    }
    // tooltipDirty[concurrentFrames] = false;

    // Adding ui panels from lua is not thread safe. (It requires exclusive access of the lua stack).
    // This that they are added before any other lua stuff runs
    // net.bindStateUpdater(&authState, client);
    net.launchIo();

    assert(std::filesystem::exists("hud.lua"));
    GuiCommandData *what = new GuiCommandData();
    what->str2 = "Hud";
    what->flags = GUIF_INDEX;
    gui->submitCommand({ Gui::GUI_LOAD, what });
    GuiCommandData *what2 = new GuiCommandData();
    what2->str = "Hud";
    what2->action = GUIA_VISIBLE;
    what2->flags = GUIF_PANEL_NAME;
    gui->submitCommand({ Gui::GUI_VISIBILITY, what2 });

    consoleLua = new LuaWrapper(true);
    consoleLua->exportEnumToLua<InsertionMode>();
    consoleLua->exportEnumToLua<IEngage>();
    consoleLua->exportEnumToLua<IntrinicStates>();
    consoleLua->apiExport();
    consoleThread = std::thread(&Engine::handleConsoleInput, this);

    // I think this is fine since we should always get the defualt implementation
    lua->doString("local audioDeviceList = eng_listAudioDevices(); eng_pickAudioDevice(audioDeviceList[1])");

    std::chrono::steady_clock::time_point frameStart = std::chrono::steady_clock::now();
    while (!glfwWindowShouldClose(window)) {
        drawFrame();
        glfwPollEvents();
        handleInput();
        if (totalFrames % 50 == 2 & authState.frame.load(std::memory_order_relaxed) > 0) {
            GuiCommandData *what = new GuiCommandData();
            what->str = "onPeriodicUpdate";
            gui->submitCommand({ Gui::GUI_NOTIFY, what });
        }

        auto now = std::chrono::steady_clock::now();
        frameTimes[frameTimeIndex] = std::chrono::duration_cast<std::chrono::milliseconds>(now - frameStart).count();
        frameTimeIndex = (frameTimeIndex + 1) % frameTimes.size();
        if (!frameTimeIndex) {
            auto sum = std::accumulate(begin(frameTimes), end(frameTimes), 0);
            fps = static_cast<float>(frameTimes.size()) / static_cast<float>(sum) * 1000.0f;
        }
        frameStart = now;
    }

    net.io.stop();

    delete cursorLines;
    delete staticLines;

    vkDeviceWaitIdle(device);
}

// NOTE This function needs proper synchronization
void Engine::setCursorInstance(const Instance& instance) {
    auto position = cursorInstance.position;
    cursorInstance = instance;
    cursorInstance.dontDelete = true;
    cursorInstance.isPlacement = true;
    cursorInstance.position = position;
}

// NOTE This function needs proper synchronization
void Engine::setCursorInstance(Instance&& instance) {
    auto position = cursorInstance.position;
    cursorInstance = instance;
    cursorInstance.dontDelete = true;
    cursorInstance.isPlacement = true;
    cursorInstance.position = position;
}

void Engine::setCursorEntity(const std::string& name, InstanceID unitID) {
    if (name.empty()) {
        removeCursorEntity();
    }
    std::scoped_lock l(apiLocks[APIL_CURSOR_INSTANCE]);
    const auto ent = currentScene->entities.at(name);
    Instance inst(ent, currentScene->textures.data() + ent->textureIndex, currentScene->models.data() + ent->modelIndex, 1, engineSettings.teamIAm.id, false);
    setCursorInstance(std::move(inst));
    placingStructure = true;
    unitDoingStationBuilding = unitID;
}

void Engine::removeCursorEntity() {
    std::scoped_lock l(apiLocks[APIL_CURSOR_INSTANCE]);
    setCursorInstance(Instance());
}

void Engine::loadDefaultScene() {
    currentScene = new Scene(this, {
        {"models/sphere.obj", "textures/sphere.png", "", "sphere"}
    }, {"skyboxes/front.png", "skyboxes/back.png", "skyboxes/up.png", "skyboxes/down.png", "skyboxes/right.png", "skyboxes/left.png"});
    currentScene->makeBuffers();
    // This first is the skybox (the position and heading do not matter)
    currentScene->addInstance("sphere", { 0.0f, 0.0f, 0.0f}, { 0.0f, 0.0f, 0.0f});
    currentScene->state.instances.push_back(&cursorRender);
    cursorRender.dontDelete = true;
    cursorInstance.dontDelete = true;

    // +x = -x, +y = -y, z = -z
    lightingData.pos = { -50.0f, -110.0, 0.0f };
    lightingData.view = glm::lookAt(-lightingData.pos, glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    lightingDataView_1 = inverse(lightingData.view);
}

// TODO this function has a crappy name that confuses me
void Engine::createMainDescriptors(const std::vector<EntityTexture>& textures, const std::vector<GuiTexture>& hudTextures) {
    assert(textures.size() <= ECONF_MAX_ENTITY_TEXTURES);
    assert(hudTextures.size() <= ECONF_MAX_HUD_TEXTURES);

    std::array<VkDescriptorPoolSize, 5> poolSizes {};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    poolSizes[0].descriptorCount = concurrentFrames;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = ECONF_MAX_ENTITY_TEXTURES * concurrentFrames;
    poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[2].descriptorCount = concurrentFrames;
    poolSizes[3].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[3].descriptorCount = concurrentFrames;
    poolSizes[4].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[4].descriptorCount = concurrentFrames;

    std::array<VkDescriptorPoolSize, 4> hudPoolSizes;
    hudPoolSizes[0].type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    hudPoolSizes[0].descriptorCount = concurrentFrames;
    hudPoolSizes[1].type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    hudPoolSizes[1].descriptorCount = concurrentFrames;
    hudPoolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    hudPoolSizes[2].descriptorCount = ECONF_MAX_HUD_TEXTURES * concurrentFrames;
    hudPoolSizes[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    hudPoolSizes[3].descriptorCount = concurrentFrames;

    VkDescriptorPoolCreateInfo poolInfo {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = poolSizes.size();
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = concurrentFrames;

    VkDescriptorPoolCreateInfo hudPoolInfo {};
    hudPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    hudPoolInfo.poolSizeCount = hudPoolSizes.size();
    hudPoolInfo.pPoolSizes = hudPoolSizes.data();
    hudPoolInfo.maxSets = concurrentFrames;

    vulkanErrorGuard(vkCreateDescriptorPool(device, &poolInfo, nullptr, &mainPass.descriptorPool), "Failed to create descriptor pool.");
    vulkanErrorGuard(vkCreateDescriptorPool(device, &hudPoolInfo, nullptr, &hudDescriptorPool), "Failed to create hud descriptor pool.");

    std::vector<VkDescriptorSetLayout> layouts(concurrentFrames, mainPass.descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = mainPass.descriptorPool;
    allocInfo.descriptorSetCount = concurrentFrames;
    allocInfo.pSetLayouts = layouts.data();

    std::vector<VkDescriptorSetLayout> hudLayouts(concurrentFrames, hudDescriptorLayout);
    VkDescriptorSetAllocateInfo hudAllocInfo {};
    hudAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    hudAllocInfo.descriptorPool = hudDescriptorPool;
    hudAllocInfo.descriptorSetCount = concurrentFrames;
    hudAllocInfo.pSetLayouts = hudLayouts.data();

    mainPass.descriptorSets.resize(concurrentFrames);
    hudDescriptorSets.resize(concurrentFrames);

    vulkanErrorGuard(vkAllocateDescriptorSets(device, &allocInfo, mainPass.descriptorSets.data()), "Failed to allocate descriptor sets.");
    vulkanErrorGuard(vkAllocateDescriptorSets(device, &hudAllocInfo, hudDescriptorSets.data()), "Failed to allocate hud descriptor sets.");

    for (size_t i = 0; i < concurrentFrames; i++) {
        // VkDescriptorBufferInfo bufferInfo {};
        // bufferInfo.buffer = uniformBuffers[i];
        // bufferInfo.offset = 0;
        // // Idk if this involves a performance hit other than always copying the whole buffer even if it is not completely filled (this should be very small)
        // // It does save updating the descriptor set though, which means it could be faster this way (this would be my guess)
        // bufferInfo.range = VK_WHOLE_SIZE;

        VkDescriptorBufferInfo lightingBufferInfo {};
        lightingBufferInfo.buffer = lightingBuffers[i];
        lightingBufferInfo.offset = 0;
        lightingBufferInfo.range = sizeof(Utilities::ViewProjPosNearFar);

        std::array<VkWriteDescriptorSet, 4> descriptorWrites {};

        // descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        // descriptorWrites[0].dstSet = mainPass.descriptorSets[i];
        // descriptorWrites[0].dstBinding = 0;
        // descriptorWrites[0].dstArrayElement = 0;
        // descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        // descriptorWrites[0].descriptorCount = 1;
        // descriptorWrites[0].pBufferInfo = &bufferInfo;

        int j = 0;
        std::array<VkDescriptorImageInfo, ECONF_MAX_ENTITY_TEXTURES> imageInfos;
        for (; j < textures.size(); j++) {
            imageInfos[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfos[j].imageView = textures[j].imageView;
            imageInfos[j].sampler = textures[j].sampler;
        }
        for(; j < ECONF_MAX_ENTITY_TEXTURES; j++) {
            imageInfos[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfos[j].imageView = textures[currentScene->missingIcon].imageView;
            imageInfos[j].sampler = textures[currentScene->missingIcon].sampler;
        }

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = mainPass.descriptorSets[i];
        descriptorWrites[0].dstBinding = 1;
        // Hmm, can we use this for arraying our textures in a smarter way?
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[0].descriptorCount = imageInfos.size();
        descriptorWrites[0].pImageInfo = imageInfos.data();

        VkDescriptorImageInfo shadowImageInfo = {};
        shadowImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        shadowImageInfo.imageView = shadow.imageViews[i];
        shadowImageInfo.sampler = shadow.samplers[i];

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = mainPass.descriptorSets[i];
        descriptorWrites[1].dstBinding = 2;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &shadowImageInfo;

        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[2].dstSet = mainPass.descriptorSets[i];
        descriptorWrites[2].dstBinding = 3;
        descriptorWrites[2].dstArrayElement = 0;
        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[2].descriptorCount = 1;
        descriptorWrites[2].pBufferInfo = &lightingBufferInfo;

        VkDescriptorImageInfo skyboxInfo;
        skyboxInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        skyboxInfo.imageView = currentScene->skybox.imageView;
        skyboxInfo.sampler = currentScene->skybox.sampler;

        descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[3].dstSet = mainPass.descriptorSets[i];
        descriptorWrites[3].dstBinding = 4;
        descriptorWrites[3].dstArrayElement = 0;
        descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[3].descriptorCount = 1;
        descriptorWrites[3].pImageInfo = &skyboxInfo;

        vkUpdateDescriptorSets(device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
    }
}

void Engine::writeHudDescriptors() {
    hudDescriptorNeedsRewrite.resize(concurrentFrames);
    hudDescriptorWriteCount.resize(concurrentFrames);
    fill(hudDescriptorWriteCount.begin(), hudDescriptorWriteCount.end(), 0);

    for (size_t i = 0; i < concurrentFrames; i++) {
        std::array<VkWriteDescriptorSet, 3> hudDescriptorWrites {};

        // I think maybe you can array input attachments
        VkDescriptorImageInfo descriptorImageInfo {};
        descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        descriptorImageInfo.imageView = subpassImageViews[i];
        descriptorImageInfo.sampler = VK_NULL_HANDLE;

        hudDescriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        hudDescriptorWrites[0].dstSet = hudDescriptorSets[i];
        hudDescriptorWrites[0].dstBinding = 0;
        hudDescriptorWrites[0].dstArrayElement = 0;
        hudDescriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        hudDescriptorWrites[0].descriptorCount = 1;
        hudDescriptorWrites[0].pImageInfo = &descriptorImageInfo;

        VkDescriptorImageInfo iconDescriptorImageInfo {};
        iconDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        iconDescriptorImageInfo.imageView = bgSubpassImageViews[i];
        iconDescriptorImageInfo.sampler = VK_NULL_HANDLE;

        hudDescriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        hudDescriptorWrites[1].dstSet = hudDescriptorSets[i];
        hudDescriptorWrites[1].dstBinding = 1;
        hudDescriptorWrites[1].dstArrayElement = 0;
        hudDescriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        hudDescriptorWrites[1].descriptorCount = 1;
        hudDescriptorWrites[1].pImageInfo = &iconDescriptorImageInfo;

        std::vector<VkDescriptorImageInfo> hudTextureInfos(ECONF_MAX_HUD_TEXTURES);
        for (int i = 0; i < ECONF_MAX_HUD_TEXTURES; i++) {
            hudTextureInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            hudTextureInfos[i].imageView = GuiTextures::defaultTexture->imageView;
            hudTextureInfos[i].sampler = GuiTextures::defaultTexture->sampler;
        }

        hudDescriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        hudDescriptorWrites[2].dstSet = hudDescriptorSets[i];
        hudDescriptorWrites[2].dstBinding = 2;
        hudDescriptorWrites[2].dstArrayElement = 0;
        hudDescriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        hudDescriptorWrites[2].descriptorCount = hudTextureInfos.size();
        hudDescriptorWrites[2].pImageInfo = hudTextureInfos.data();

        // vkUpdateDescriptorSets(device, hudDescriptorWrites.size(), hudDescriptorWrites.data(), 0, nullptr);
        vkUpdateDescriptorSets(device, hudDescriptorWrites.size(), hudDescriptorWrites.data(), 0, nullptr);
    }
}

// This is to keep the refcount from hiting 0 on the textures in use by the gpu still, but with no guicomponents using it anymore
// I think if concurrent frams was higher this would need more layers (I have concurrent frames = 2 right now)
// TODO Think what is the better way to do this?
static std::vector<std::shared_ptr<GuiTexture>> textureRefs, textureRefsOld, textureRefsOldOld;

static std::ofstream frameLog("frameLog", std::ofstream::out);

void Engine::rewriteHudDescriptors(const std::vector<std::shared_ptr<GuiTexture>>& hudTextures) {
    textureRefsOldOld = textureRefsOld;
    textureRefsOld = textureRefs;
    textureRefs.clear();

    fill(hudDescriptorNeedsRewrite.begin(), hudDescriptorNeedsRewrite.end(), true);

    // frameLog << "On frame " << std::dec << totalFrames << std::endl;
    for(const auto textPtr : hudTextures) {
        // frameLog << "    " << textPtr->imageView << std::endl;
        textureRefs.push_back(textPtr);
        // textureRefsOld.push_back(textPtr);
    }
}

void Engine::rewriteHudDescriptors(int index) {
    std::array<VkWriteDescriptorSet, 1> hudDescriptorWrites {};

    std::vector<VkDescriptorImageInfo> hudTextureInfos(std::max(textureRefs.size(), hudDescriptorWriteCount[index]));
    int j;

    for (j = 0; j < textureRefs.size(); j++) {
        hudTextureInfos[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        hudTextureInfos[j].imageView = textureRefs[j]->imageView;
        hudTextureInfos[j].sampler = textureRefs[j]->sampler;
        // hudTextureInfos[i].imageView = currentScene->state.instances[0].texture->textureImageView;
        // hudTextureInfos[i].sampler = currentScene->state.instances[0].texture->textureSampler;
    }
    for (; j < hudDescriptorWriteCount[index]; j++) {
        hudTextureInfos[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        hudTextureInfos[j].imageView = GuiTextures::defaultTexture->imageView;
        hudTextureInfos[j].sampler = GuiTextures::defaultTexture->sampler;
    }

    hudDescriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    hudDescriptorWrites[0].dstSet = hudDescriptorSets[index];
    hudDescriptorWrites[0].dstBinding = 2;
    hudDescriptorWrites[0].dstArrayElement = 0;
    hudDescriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    hudDescriptorWrites[0].descriptorCount = hudTextureInfos.size();
    hudDescriptorWrites[0].pImageInfo = hudTextureInfos.data();

    vkUpdateDescriptorSets(device, hudDescriptorWrites.size(), hudDescriptorWrites.data(), 0, nullptr);

    hudDescriptorWriteCount[index] = textureRefs.size();
}

// Idk if this should be gui or engine (probably engine since it needs syncronization to the draws)
void Engine::setTooltipTexture(int index, std::shared_ptr<GuiTexture> texture) {
    VkDescriptorImageInfo tooltipTextureInfo;
    tooltipTextureInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    if (texture->imageView) {
        tooltipTextureInfo.imageView = texture->imageView;
        tooltipTextureInfo.sampler = texture->sampler;
    } else {
        tooltipTextureInfo.imageView = GuiTextures::defaultTexture->imageView;
        tooltipTextureInfo.sampler = GuiTextures::defaultTexture->sampler;
    }

    VkWriteDescriptorSet descriptorWrite {};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = hudDescriptorSets[index];
    descriptorWrite.dstBinding = 3;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &tooltipTextureInfo;

    vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
}

void Engine::updateLightingDescriptors(int index, const Utilities::ViewProjPosNearFar& data) {
    memcpy(lightingBufferAllocations[index]->GetMappedData(), &data, sizeof(Utilities::ViewProjPosNearFar));
}

void Engine::allocateCommandBuffers() {
    commandBuffers.resize(concurrentFrames);
    transferCommandBuffers.resize(concurrentFrames);

    VkCommandBufferAllocateInfo commandAlocInfo {};
    commandAlocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandAlocInfo.commandPool = commandPool;
    commandAlocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandAlocInfo.commandBufferCount = concurrentFrames;

    vulkanErrorGuard(vkAllocateCommandBuffers(device, &commandAlocInfo, commandBuffers.data()), "Failed to allocate command buffers.");

    if (engineSettings.useConcurrentTransferQueue) {
        VkCommandBufferAllocateInfo transferCommandAlocInfo {};
        transferCommandAlocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        transferCommandAlocInfo.commandPool = transferCommandPool;
        transferCommandAlocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        transferCommandAlocInfo.commandBufferCount = concurrentFrames;
        vulkanErrorGuard(vkAllocateCommandBuffers(device, &transferCommandAlocInfo, transferCommandBuffers.data()), "Failed to allocate transfer command buffers.");
    }
}

#include "shaders/render_modes.h"

// I am sorry that all the main pass rendering looks like it was made by a monkey, I didn't know how to plan it out at the start
// cause I didn't know what I was ultimately going to need (I should refactor it and make it less idiotic, but I don't really feel like doing that rn)
void Engine::recordCommandBuffer(const VkCommandBuffer& buffer, const VkFramebuffer& framebuffer, const VkDescriptorSet& descriptorSet,
    const VkDescriptorSet& hudDescriptorSet, int index) {
    // frameLog << "Doing frame " << std::dec << totalFrames << std::endl;
    VkCommandBufferBeginInfo beginInfo {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;

    pushConstants.blinkTime = sinf(static_cast<float>((std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()
        .time_since_epoch()) % 2000).count()) / (1999.0f / M_PI));

    vulkanErrorGuard(vkBeginCommandBuffer(buffer, &beginInfo), "Failed to begin recording command buffer.");

    runShadowPass(buffer, index);

    // VkImageMemoryBarrier barrier {};
    // barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    // barrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    // barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    // barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    // barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    // barrier.image = shadow.images[index];

    // barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    // barrier.subresourceRange.baseMipLevel = 0;
    // barrier.subresourceRange.levelCount = 1;
    // barrier.subresourceRange.baseArrayLayer = 0;
    // barrier.subresourceRange.layerCount = 1;

    // // not sure about src mask ????
    // barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
    // // I do know that the dst access mask is right
    // barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;

    // // also no clue about stage masks??? I am continually confused by these things
    // vkCmdPipelineBarrier(
    //     buffer,
    //     VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
    //     VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
    //     0,
    //     0, nullptr,
    //     0, nullptr,
    //     1, &barrier
    // );

    VkRenderPassBeginInfo renderPassInfo {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = framebuffer;

    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = swapChainExtent;

    std::array<VkClearValue, 6> clearValues = {};
    clearValues[0].color = {{ 0.0f, 0.0f, 0.0f, 0.0f }};
    clearValues[1].depthStencil = { 1.0f, 0 };
    clearValues[2].color = {{ 0.0f, 0.0f, 0.0f, 1.0f }};
    clearValues[3].color = {{ 0.0f, 0.0f, 0.0f, 0.0f }};
    clearValues[4].color = {{ 0.0f, 0.0f, 0.0f, 0.0f }};
    clearValues[5].depthStencil = { 1.0f, 0 };


    renderPassInfo.clearValueCount = clearValues.size();
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkBuffer vertexBuffers[] = { vertexBuffer };
    VkDeviceSize offsets[] = { 0 };
    uint32_t dynamicOffset = 0;

    vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipelines[GP_LINES]);

    vkCmdSetLineWidth(buffer, 1.0);

    // std::cout << nonBeamLines << ":" << lineSync->validCount[index] << std::endl;
    for (int j = 0; j < lineSync->validCount[index]; j++) {
        if (j == nonBeamLines) {
            vkCmdSetLineWidth(buffer, 4.0);
        }
        dynamicOffset = j * uniformSync->uniformSkip;
        vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, linePipelineLayout, 0, 1, &mainPass.lineDescriptorSets[index], 1, &dynamicOffset);
        vkCmdPushConstants(buffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstants), &pushConstants);
        vkCmdDraw(buffer, 2, 1, 0, 0);
    }

    vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipelines[GP_WORLD]);

    vkCmdBindVertexBuffers(buffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(buffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

    // 0th instance is always the skybox (see Engine::loadDefaultScene)
    for (auto j : zSorted) {
        if (!currentScene->state.instances[j]->valid || !currentScene->state.instances[j]->rendered) continue;
        if (engineSettings.teamIAm.id && engineSettings.teamIAm.id != currentScene->state.instances[j]->team &&
            currentScene->state.instances[j]->entity->isUnit && currentScene->state.instances[j]->outOfFog[engineSettings.teamIAm.id - 1]) continue;
        // TODO I need to decide how I am drawing radar dots
        pushConstants.teamColor = Scene::teamColors[currentScene->state.instances[j]->team];
        dynamicOffset = j * uniformSync->uniformSkip;
        // render the unit as an icon
        if (currentScene->state.instances[j]->renderAsIcon) {
            const auto ent = currentScene->state.instances[j]->entity;
            vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 1, &dynamicOffset);
            pushConstants.renderType = RINT_ICON | (int)currentScene->state.instances[j]->highlight * RFLAG_HIGHLIGHT;
            pushConstants.textureIndex = ent->hasIcon ? ent->iconIndex : currentScene->missingIcon; // ent->hasIcon ? ent->iconIndex : currentScene->textures.size() - 1;
            vkCmdPushConstants(buffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstants), &pushConstants);
            vkCmdDrawIndexed(buffer, (currentScene->models.data() + iconModelIndex)->indexCount, 1, (currentScene->models.data() + iconModelIndex)->indexOffset, 0, 0);
            continue;
        }
        // if (j == 1) std::cout << "drawing cursor instance: " << currentScene->state.instances[j]->position << std::endl;
        if (currentScene->state.instances[j]->entity->isProjectile && !currentScene->state.instances[j]->entity->isGuided) {
            pushConstants.renderType = RINT_PROJECTILE;
        } else {
            pushConstants.renderType = (currentScene->state.instances[j]->uncompleted ? RINT_UNCOMPLETE : RINT_OBJ)
                | (int)currentScene->state.instances[j]->highlight * RFLAG_HIGHLIGHT | (int)currentScene->state.instances[j]->isPlacement * RFLAG_TRANSPARENT
                | (int)!!currentScene->state.instances[j]->intrinicStates[static_cast<int>(IntrinicStates::CLOAKED)] * RFLAG_CLOAKED;
        }
        vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 1, &dynamicOffset);
        pushConstants.textureIndex = currentScene->state.instances[j]->entity->textureIndex;
        vkCmdPushConstants(buffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstants), &pushConstants);
        // We can bundle draw commands the entities are the same (this includes the textures), I doubt that the bottleneck is recording the command buffers
        vkCmdDrawIndexed(buffer, currentScene->state.instances[j]->sceneModelInfo->indexCount, 1,
            currentScene->state.instances[j]->sceneModelInfo->indexOffset, 0, 0);
        // The cursor entity (where j is 1 never has a health or resource bar)
        if (j == 1) continue;
        // draw the health bar
        if (currentScene->state.instances[j]->entity->isUnit) {
            pushConstants.renderType = RINT_HEALTH;
            vkCmdPushConstants(buffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstants), &pushConstants);
            vkCmdDrawIndexed(buffer, (currentScene->models.data() + iconModelIndex)->indexCount, 1, (currentScene->models.data() + iconModelIndex)->indexOffset, 0, 0);
        // draw the resource bar
        } else if (currentScene->state.instances[j]->entity->isResource) {
            pushConstants.renderType = RINT_RESOURCE;
            vkCmdPushConstants(buffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstants), &pushConstants);
            vkCmdDrawIndexed(buffer, (currentScene->models.data() + iconModelIndex)->indexCount, 1, (currentScene->models.data() + iconModelIndex)->indexOffset, 0, 0);
        }
    }

    vkCmdNextSubpass(buffer, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipelines[GP_BG]);

    vkCmdBindVertexBuffers(buffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(buffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

    // render the skybox
    dynamicOffset = 0;
    pushConstants.renderType = RINT_SKYBOX;
    vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 1, &dynamicOffset);
    vkCmdPushConstants(buffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstants), &pushConstants);
    vkCmdDrawIndexed(buffer, currentScene->state.instances[0]->sceneModelInfo->indexCount, 1,
        currentScene->state.instances[0]->sceneModelInfo->indexOffset, 0, 0);

    vkCmdNextSubpass(buffer, VK_SUBPASS_CONTENTS_INLINE);

    // VkClearAttachment depthClear;
    // depthClear.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    // depthClear.colorAttachment = 0;
    // depthClear.clearValue.depthStencil = { 1.0f, 0 };

    // VkClearRect depthClearRect;
    // depthClearRect.rect.offset = { 0, 0 };
    // depthClearRect.rect.extent = swapChainExtent;
    // depthClearRect.baseArrayLayer = 0;
    // depthClearRect.layerCount = 1;

    // vkCmdClearAttachments(buffer, 1, &depthClear, 1, &depthClearRect);

    vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipelines[GP_HUD]);
    vkCmdBindVertexBuffers(buffer, 0, 1, &hudBuffer, offsets);
    vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, hudPipelineLayout, 0, 1, &hudDescriptorSet, 0, 0);

    // Why need fragment stage access here???? (Obviously cause I declared the layout to need it, but if I don't the validation layer complains)?????
    vkCmdPushConstants(buffer, hudPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(GuiPushConstant), &gui->pushConstant);
    vkCmdDraw(buffer, Gui::dummyVertexCount - 6, 1, 0, 0);

    if(!guiOutOfDate) vkCmdDraw(buffer, hudVertexCount - Gui::dummyVertexCount, 1, Gui::dummyVertexCount, 0);

    // Draw the tooltip last
    if(drawTooltip) vkCmdDraw(buffer, 6, 1, 12, 0);

    // frameLog << "    hud vertex count: " << hudVertexCount << std::endl;

    vkCmdEndRenderPass(buffer);

    if (depthDump.makeDump)
        dumpDepthBuffer(buffer, index);

    vulkanErrorGuard(vkEndCommandBuffer(buffer), "Failed to record command buffer.");
}

void Engine::initSynchronization() {
    imageAvailableSemaphores.resize(concurrentFrames);
    renderFinishedSemaphores.resize(concurrentFrames);
    inFlightFences.resize(concurrentFrames);
    // imagesInFlight.resize(concurrentFrames, VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for(int i = 0; i < concurrentFrames; i++) {
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
                throw std::runtime_error("Failed to create synchronization primatives.");
        // if (engineSettings.verbose) std::cout << std::hex << "Fence " << i << " : " << inFlightFences[i] << std::endl;
    }
}

void Engine::recreateSwapChain() {
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(device);

    cleanupSwapChain();

    createSwapChain();
    createImageViews();
    createRenderPass();
    createGraphicsPipelines();
    createDepthResources();
    createColorResources();
    createFramebuffers();
    createMainDescriptors(currentScene->textures, {});
    uniformSync->rebindSyncPoints({{ &mainPass.descriptorSets, 0 }, { &shadow.descriptorSets, 0 }});
    writeHudDescriptors();
    fill(hudDescriptorNeedsRewrite.begin(), hudDescriptorNeedsRewrite.end(), true);

    drawTooltip = false;
    for (int i = 0; i < concurrentFrames; i++) setTooltipTexture(i, tooltipResource);
    // for (int i = 0; i < concurrentFrames; i++) setTooltipTexture(i, *GuiTextures::defaultTexture);
}

void Engine::cleanupSwapChain() {
    destroyDepthResources();
    destroyColorResources();

    for (auto framebuffer : swapChainFramebuffers)
        vkDestroyFramebuffer(device, framebuffer, nullptr);

    // vkFreeCommandBuffers(device, commandPool, commandBuffers.size(), commandBuffers.data());
    // if (engineSettings.useConcurrentTransferQueue)
    //     vkFreeCommandBuffers(device, transferCommandPool, transferCommandBuffers.size(), transferCommandBuffers.data());

    for (auto pipeline : graphicsPipelines)
        vkDestroyPipeline(device, pipeline, nullptr);
    // TODO I think we could technically keep the pipeline layouts even on resizing
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyPipelineLayout(device, linePipelineLayout, nullptr);
    vkDestroyPipelineLayout(device, hudPipelineLayout, nullptr);
    vkDestroyRenderPass(device, renderPass, nullptr);

    destroyImageViews();

    vkDestroySwapchainKHR(device, swapChain, nullptr);

    // TODO Resizing should not destroy the descriptor pools, this is stupid and not needed
    vkDestroyDescriptorPool(device, mainPass.descriptorPool, nullptr);
    vkDestroyDescriptorPool(device, hudDescriptorPool, nullptr);
}

void Engine::cleanup() {
    cleanupSwapChain();

    destroyComputeResources();
    vkDestroySampler(device, TextResource::sampler_, nullptr);

    destroyLightingBuffers();
    delete lineSync;
    delete uniformSync;
    vkDestroyDescriptorPool(device, mainPass.lineDescriptorPool, nullptr);

    vkDestroyDescriptorSetLayout(device, mainPass.descriptorSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, mainPass.lineDescriptorLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, hudDescriptorLayout, nullptr);

    vkDestroyBuffer(device, indexBuffer, nullptr);
    vkFreeMemory(device, indexBufferMemory, nullptr);
    vkDestroyBuffer(device, vertexBuffer, nullptr);
    vkFreeMemory(device, vertexBufferMemory, nullptr);

    for (size_t i = 0; i < concurrentFrames; i++) {
        vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(device, inFlightFences[i], nullptr);
    }

    if (engineSettings.useConcurrentTransferQueue) vkDestroyCommandPool(device, transferCommandPool, nullptr);
    vkDestroyCommandPool(device, commandPool, nullptr);
    vkDestroyCommandPool(device, guiCommandPool, nullptr);
    vkDestroyCommandPool(device, computeCommandPool, nullptr);

    vkDestroyPipelineCache(device, pipelineCache, nullptr);
    destroyShadowResources();
    vmaDestroyAllocator(memoryAllocator);

    vkDestroyDevice(device, nullptr);

    if (engineSettings.validationLayers.size()) DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);

    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);

    glfwDestroyWindow(window);
    destroyCursors();
    glfwTerminate();
}

Engine::~Engine() {
    if (std::uncaught_exceptions()) {
        return;
    }
    done = true;
    consoleThread.join();
    delete consoleLua;
    textureRefs.clear();
    textureRefsOld.clear();
    textureRefsOldOld.clear();
    delete currentScene;
    GuiTextures::defaultTexture.reset();
    GuiTextures::textureCache.clear();
    delete gui;
    delete glyphCache;
    delete manager;
    delete sound;
    tooltipResource.reset();
    tooltipStillInUse.reset();
    cleanup();
}

void Engine::createShadowResources(bool createDebugging) {
    auto depthFormat = findDepthFormat();
    shadow.debugging = createDebugging;
    shadow.makeSnapshot = false;
    shadow.debugWritePending = false;

    VkAttachmentDescription attachmentDescription {};
    attachmentDescription.format = depthFormat;
    attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkAttachmentReference depthReference = {};
    depthReference.attachment = 0;
    depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 0;
    subpass.pDepthStencilAttachment = &depthReference;
    subpass.pResolveAttachments = nullptr;

    // Use subpass dependencies for layout transitions
    std::array<VkSubpassDependency, 2> dependencies;

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    VkRenderPassCreateInfo renderPassCreateInfo = {};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.attachmentCount = 1;
    renderPassCreateInfo.pAttachments = &attachmentDescription;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpass;
    renderPassCreateInfo.dependencyCount = dependencies.size();
    renderPassCreateInfo.pDependencies = dependencies.data();

    vulkanErrorGuard(vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &shadow.renderPass), "Unable to create shadow render pass");

    auto vertShaderCode = Utilities::readFile("shaders/shadow_vert.spv");

    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo {};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    // VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo };

    VkPipelineVertexInputStateCreateInfo vertexInputInfo {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    auto bindingDescription = Utilities::Vertex::getBindingDescription();
    auto attributeDescriptions = Utilities::Vertex::getAttributeDescriptions();

    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(shadow.width);
    viewport.height = static_cast<float>(shadow.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 0.5f;

    VkRect2D scissor {};
    scissor.offset = { 0, 0 };
    scissor.extent = { shadow.width, shadow.height };

    VkPipelineViewportStateCreateInfo viewportState {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasEnable = VK_TRUE;

    VkPipelineMultisampleStateCreateInfo multisampling {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    // Idk if this is set right or if it matters
    VkPipelineColorBlendStateCreateInfo colorBlending {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 0;
    colorBlending.pAttachments = nullptr;

    VkDescriptorSetLayoutBinding uboLayoutBinding {};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr;

    std::array<VkDescriptorSetLayoutBinding, 1> bindings = { uboLayoutBinding };
    VkDescriptorSetLayoutCreateInfo layoutInfo {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = bindings.size();
    layoutInfo.pBindings = bindings.data();

    vulkanErrorGuard(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &shadow.descriptorLayout), "Failed to create shadow descriptor set layout.");

    VkPipelineLayoutCreateInfo pipelineLayoutInfo {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &shadow.descriptorLayout;

    VkPushConstantRange pushConstantRange;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(ShadowPushConstansts);
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    vulkanErrorGuard(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &shadow.pipelineLayout), "Failed to create shadow pipeline layout.");

    VkPipelineDepthStencilStateCreateInfo depthStencil {};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f;
    depthStencil.maxDepthBounds = 1.0f;
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {};
    depthStencil.back = {};

    std::array<VkDynamicState, 1> dynamicStateEnables = { VK_DYNAMIC_STATE_DEPTH_BIAS };
    VkPipelineDynamicStateCreateInfo dynamicStates = {};
    dynamicStates.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStates.dynamicStateCount = dynamicStateEnables.size();
    dynamicStates.pDynamicStates = dynamicStateEnables.data();
    dynamicStates.flags = 0;

    VkGraphicsPipelineCreateInfo pipelineInfo {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 1;
    pipelineInfo.pStages = &vertShaderStageInfo;

    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicStates;

    pipelineInfo.layout = shadow.pipelineLayout;
    pipelineInfo.renderPass = shadow.renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    pipelineInfo.pDepthStencilState = &depthStencil;

    vulkanErrorGuard(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineInfo, nullptr, &shadow.pipeline),
        "Failed to create shadow graphics pipeline.");

    vkDestroyShaderModule(device, vertShaderModule, nullptr);

    shadow.imageViews.resize(concurrentFrames);
    shadow.images.resize(concurrentFrames);
    shadow.allocations.resize(concurrentFrames);
    shadow.samplers.resize(concurrentFrames);
    shadow.framebuffers.resize(concurrentFrames);

    for (int i = 0; i < concurrentFrames; i++) {
        // For shadow mapping we only need a depth attachment
        VkImageCreateInfo imageInfo {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = shadow.width;
        imageInfo.extent.height = shadow.height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = depthFormat;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo imageAllocCreateInfo = {};
        imageAllocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        if (vmaCreateImage(memoryAllocator, &imageInfo, &imageAllocCreateInfo, &shadow.images[i], &shadow.allocations[i], nullptr) != VK_SUCCESS)
            throw std::runtime_error("Unable to create shadow framebuffer image");

        VkImageViewCreateInfo depthStencilView = {};
        depthStencilView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        depthStencilView.viewType = VK_IMAGE_VIEW_TYPE_2D;
        depthStencilView.format = depthFormat;
        depthStencilView.subresourceRange = {};
        depthStencilView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        depthStencilView.subresourceRange.baseMipLevel = 0;
        depthStencilView.subresourceRange.levelCount = 1;
        depthStencilView.subresourceRange.baseArrayLayer = 0;
        depthStencilView.subresourceRange.layerCount = 1;
        depthStencilView.image = shadow.images[i];
        vulkanErrorGuard(vkCreateImageView(device, &depthStencilView, nullptr, &shadow.imageViews[i]), "Unable to create shadow framebuffer image view.");

        // Create sampler to sample from to depth attachment
        VkFilter shadowmapFilter = formatIsFilterable(physicalDevice, depthFormat, VK_IMAGE_TILING_OPTIMAL) ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
        VkSamplerCreateInfo sampler = {};
        sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        sampler.magFilter = shadowmapFilter;
        sampler.minFilter = shadowmapFilter;
        sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler.unnormalizedCoordinates = false;
        sampler.anisotropyEnable = false;
        sampler.maxAnisotropy = 1.0f;
        sampler.mipLodBias = 0.0f;
        sampler.minLod = 0.0f;
        sampler.maxLod = 100.0f;
        sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
        vulkanErrorGuard(vkCreateSampler(device, &sampler, nullptr, &shadow.samplers[i]), "Unable to create shadow sampler.");

        // Create frame buffer
        VkFramebufferCreateInfo fbufCreateInfo = {};
        fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbufCreateInfo.renderPass = shadow.renderPass;
        fbufCreateInfo.attachmentCount = 1;
        fbufCreateInfo.pAttachments = &shadow.imageViews[i];
        fbufCreateInfo.width = shadow.width;
        fbufCreateInfo.height = shadow.height;
        fbufCreateInfo.layers = 1;

        vulkanErrorGuard(vkCreateFramebuffer(device, &fbufCreateInfo, nullptr, &shadow.framebuffers[i]), "Unable to create shadow framebuffer.");
    }

    if (shadow.debugging) {
        // TODO This code assumes that we are only using VK_FORMAT_D32_SFLOAT for the depth format

        VkBufferCreateInfo debuggingBufferInfo = {};
        debuggingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        debuggingBufferInfo.size = shadow.width * shadow.height * 4;
        debuggingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;

        VmaAllocationCreateInfo debuggingAllocationInfo = {};
        debuggingAllocationInfo.usage = VMA_MEMORY_USAGE_GPU_TO_CPU;
        debuggingAllocationInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

        VmaAllocationInfo debuggingBufferAl = {};

        if (vmaCreateBuffer(memoryAllocator, &debuggingBufferInfo, &debuggingAllocationInfo, &shadow.debugBuffer, &shadow.debugAllocation, &shadow.debugAllocInfo) != VK_SUCCESS)
            throw std::runtime_error("Unable to allocate memory for shadow debug buffer.");
    }
}

void Engine::destroyShadowResources() {
    vkDestroyRenderPass(device, shadow.renderPass, nullptr);
    vkDestroyPipelineLayout(device, shadow.pipelineLayout, nullptr);
    vkDestroyPipeline(device, shadow.pipeline, nullptr);

    for (int i = 0; i < concurrentFrames; i++) {
        vkDestroyFramebuffer(device, shadow.framebuffers[i], nullptr);
        vkDestroySampler(device, shadow.samplers[i], nullptr);
        vkDestroyImageView(device, shadow.imageViews[i], nullptr);
        vmaDestroyImage(memoryAllocator, shadow.images[i], shadow.allocations[i]);
    }

    if(shadow.debugging)
        vmaDestroyBuffer(memoryAllocator, shadow.debugBuffer, shadow.debugAllocation);

    vkDestroyDescriptorPool(device, shadow.descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(device, shadow.descriptorLayout, nullptr);
}

void Engine::writeShadowBufferToFile(const VkCommandBuffer& buffer, const char *filename, int idx) {
    assert(!shadow.debugWritePending);

    VkImageMemoryBarrier barrier {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = shadow.images[idx];

    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    // not sure about src mask ????
    barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    // I do know that the dst access mask is right
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

    // also no clue about stage masks??? I am continually confused by these things
    vkCmdPipelineBarrier(
        buffer,
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    VkBufferImageCopy region = {};
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    region.imageSubresource.layerCount = 1;
    region.imageExtent.width = shadow.width;
    region.imageExtent.height = shadow.height;
    region.imageExtent.depth = 1;

    vkCmdCopyImageToBuffer(buffer, shadow.images[idx], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, shadow.debugBuffer, 1, &region);

    shadow.debugWritePending = true;
    shadow.filename = std::string(filename);

    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

    barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

    vkCmdPipelineBarrier(
        buffer,
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );
}

// This can go away, but for right now it is convinient for checking things
static int histogram[256];

void Engine::doShadowDebugWrite() {
    memset(histogram, 0, 256 * sizeof(int));

    // idk about single channle png images?
    unsigned char *buf = new unsigned char[shadow.width * shadow.height];
    float *data = static_cast<float *>(shadow.debugAllocation->GetMappedData());

    for (int y = 0; y < shadow.height; y++) {
        for (int x = 0; x < shadow.width; x++) {
            unsigned char value = static_cast<unsigned char>(data[y * shadow.width + x] * 255.0f);
            buf[y * shadow.width + x] = value;
            histogram[value] += 1;
        }
    }

    stbi_write_png(shadow.filename.c_str(), shadow.width, shadow.height, 1, buf, shadow.width);

    delete buf;

    shadow.debugWritePending = false;
}

void Engine::createShadowDescriptorSets() {
    std::array<VkDescriptorPoolSize, 1> poolSizes {};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    poolSizes[0].descriptorCount = concurrentFrames;

    VkDescriptorPoolCreateInfo poolInfo {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = poolSizes.size();
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = concurrentFrames;

    vulkanErrorGuard(vkCreateDescriptorPool(device, &poolInfo, nullptr, &shadow.descriptorPool), "Failed to create shadow descriptor pool.");

    std::vector<VkDescriptorSetLayout> layouts(concurrentFrames, shadow.descriptorLayout);
    VkDescriptorSetAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = shadow.descriptorPool;
    allocInfo.descriptorSetCount = concurrentFrames;
    allocInfo.pSetLayouts = layouts.data();

    shadow.descriptorSets.resize(concurrentFrames);

    vulkanErrorGuard(vkAllocateDescriptorSets(device, &allocInfo, shadow.descriptorSets.data()), "Failed to allocate shadow descriptor sets.");

    // for (size_t i = 0; i < concurrentFrames; i++) {
    //     VkDescriptorBufferInfo bufferInfo {};
    //     bufferInfo.buffer = uniformBuffers[i];
    //     bufferInfo.offset = 0;
    //     bufferInfo.range = VK_WHOLE_SIZE;

    //     std::array<VkWriteDescriptorSet, 1> descriptorWrites {};

    //     descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    //     descriptorWrites[0].dstSet = shadow.descriptorSets[i];
    //     descriptorWrites[0].dstBinding = 0;
    //     descriptorWrites[0].dstArrayElement = 0;
    //     descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    //     descriptorWrites[0].descriptorCount = 1;
    //     descriptorWrites[0].pBufferInfo = &bufferInfo;

    //     vkUpdateDescriptorSets(device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
    // }
}

void Engine::runShadowPass(const VkCommandBuffer& buffer, int index) {
    VkRenderPassBeginInfo renderPassInfo {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = shadow.renderPass;
    renderPassInfo.framebuffer = shadow.framebuffers[index];

    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = { shadow.width, shadow.height };

    std::array<VkClearValue, 1> clearValues = {};
    clearValues[0].depthStencil = { 1.0f, 0 };

    renderPassInfo.clearValueCount = clearValues.size();
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdSetDepthBias(buffer, 4.0f, 0.0f, 1.5f);

    vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shadow.pipeline);

    VkBuffer vertexBuffers[] = { vertexBuffer };
    VkDeviceSize offsets[] = { 0 };

    vkCmdBindVertexBuffers(buffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(buffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

    for(int j = 1; j < uniformSync->validCount[index]; j++) {
        if (!currentScene->state.instances[j]->valid || !currentScene->state.instances[j]->rendered || currentScene->state.instances[j]->renderAsIcon) continue;
        uint32_t dynamicOffset = j * uniformSync->uniformSkip;
        vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shadow.pipelineLayout, 0, 1, &shadow.descriptorSets[index], 1, &dynamicOffset);
        vkCmdPushConstants(buffer, shadow.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ShadowPushConstansts), &shadow.constants);
        vkCmdDrawIndexed(buffer, currentScene->state.instances[j]->sceneModelInfo->indexCount, 1,
            currentScene->state.instances[j]->sceneModelInfo->indexOffset, 0, 0);
    }

    vkCmdEndRenderPass(buffer);

    if (shadow.makeSnapshot && shadow.debugging) {
        writeShadowBufferToFile(buffer, shadowMapFile, index);
        shadow.makeSnapshot = false;
    }
}

// Just check for linear filtering support
VkBool32 Engine::formatIsFilterable(VkPhysicalDevice physicalDevice, VkFormat format, VkImageTiling tiling) {
    VkFormatProperties formatProps;
    vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProps);

    if (tiling == VK_IMAGE_TILING_OPTIMAL)
        return formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;

    if (tiling == VK_IMAGE_TILING_LINEAR)
        return formatProps.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;

    return false;
}

void Engine::createCursors() {
    GLFWimage imageData;
    int channels;
    for(const auto& data : cursorData) {
        imageData.pixels = stbi_load(get<0>(data), &imageData.width, &imageData.height, &channels, STBI_rgb_alpha);

        auto ret = glfwCreateCursor(&imageData, get<1>(data), get<2>(data));
        if (!ret) throw std::runtime_error("Unable to create cursor.");
        cursors.push_back(ret);

        stbi_image_free(imageData.pixels);
    }
}

void Engine::destroyCursors() {
    for(auto& cursor : cursors)
        glfwDestroyCursor(cursor);
}

namespace EntityTextures {
    // I am pretty sure I dont need to synchronize this since only the render thread uses it
    static std::map<VkImage, uint> references = {};
}

EntityTexture::EntityTexture(Engine *context, TextureCreationData creationData) {
    this->context = context;
    width = creationData.width;
    height = creationData.height;
    channels = creationData.channels;
    VkDeviceSize imageSize = width * height * creationData.channels;

    VkFormat format = Utilities::channelsToFormat(channels);

    VkBufferCreateInfo stagingBufInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    stagingBufInfo.size = imageSize;
    stagingBufInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    VmaAllocationCreateInfo stagingBufAllocCreateInfo = {};
    stagingBufAllocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    stagingBufAllocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VmaAllocation stagingBufferAllocation = VK_NULL_HANDLE;
    VmaAllocationInfo stagingBufferAllocationInfo = {};
    if (vmaCreateBuffer(context->memoryAllocator, &stagingBufInfo, &stagingBufAllocCreateInfo, &stagingBuffer, &stagingBufferAllocation, &stagingBufferAllocationInfo) !=VK_SUCCESS)
        throw std::runtime_error("Unable to allocate memory for texture.");

    memcpy(stagingBufferAllocationInfo.pMappedData, creationData.pixels, imageSize);

    mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;

    VkImageCreateInfo imageInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0;

    VmaAllocationCreateInfo imageAllocCreateInfo = {};
    imageAllocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    if (vmaCreateImage(context->memoryAllocator, &imageInfo, &imageAllocCreateInfo, &image, &allocation, nullptr) != VK_SUCCESS)
        throw std::runtime_error("Unable to create image");

    // Hm, how to use the transfer queue for this?
    // VkCommandBuffer commandBuffer = context->engineSettings.useConcurrentTransferQueue ?
    //     context->beginSingleTimeCommands(context->transferCommandPool) : context->beginSingleTimeCommands();
    VkCommandBuffer commandBuffer = context->beginSingleTimeCommands();

    VkImageMemoryBarrier imageMemoryBarrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
    imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
    imageMemoryBarrier.subresourceRange.levelCount = 1;
    imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
    imageMemoryBarrier.subresourceRange.layerCount = 1;
    imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imageMemoryBarrier.image = image;
    imageMemoryBarrier.srcAccessMask = 0;
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &imageMemoryBarrier);

    VkBufferImageCopy region = {};
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.layerCount = 1;
    region.imageExtent.width = width;
    region.imageExtent.height = height;
    region.imageExtent.depth = 1;

    vkCmdCopyBufferToImage(commandBuffer, stagingBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageMemoryBarrier.image = image;
    imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &imageMemoryBarrier);

    context->endSingleTimeCommands(commandBuffer);

    vmaDestroyBuffer(context->memoryAllocator, stagingBuffer, stagingBufferAllocation);

    VkImageViewCreateInfo textureImageViewInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    textureImageViewInfo.image = image;
    textureImageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    textureImageViewInfo.format = format;
    textureImageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    // Fix this
    textureImageViewInfo.subresourceRange.baseMipLevel = 0;
    textureImageViewInfo.subresourceRange.levelCount = 1;
    textureImageViewInfo.subresourceRange.baseArrayLayer = 0;
    textureImageViewInfo.subresourceRange.layerCount = 1;
    vulkanErrorGuard(vkCreateImageView(context->device, &textureImageViewInfo, nullptr, &imageView), "Unable to create texture image view.");

    EntityTextures::references.insert({ image, 1 });

    VkSamplerCreateInfo samplerInfo {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;

    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    samplerInfo.anisotropyEnable = VK_TRUE;
    // For right now we just use the max supported sampler anisotrpy
    samplerInfo.maxAnisotropy = context->maxSamplerAnisotropy;

    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    // fix me
    samplerInfo.maxLod = static_cast<float>(1);
    samplerInfo.mipLodBias = 0.0f;

    vulkanErrorGuard(vkCreateSampler(context->device, &samplerInfo, nullptr, &sampler), "Failed to create texture sampler.");
}

// MaxLoD is not working right cause something is wrong
void EntityTexture::generateMipmaps() {
    // I will leave this in here in case we want to handle multiple image formats in the future
    VkFormat imageFormat = Utilities::channelsToFormat(channels);
    // Check if image format supports linear blitting
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(context->physicalDevice, imageFormat, &formatProperties);

    if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
        throw std::runtime_error("Texture image format does not support linear blitting.");

    VkCommandBuffer commandBuffer = context->beginSingleTimeCommands();

    VkImageMemoryBarrier barrier {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    barrier.subresourceRange.baseMipLevel = 0;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;

    vkCmdPipelineBarrier(commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
        0, nullptr,
        0, nullptr,
        1, &barrier);

    for (uint32_t i = 1; i < mipLevels; i++) {
        // std::cout << std::dec << "Blitting mipmap level: " << i  << std::endl;

        const std::int32_t mipWidth = std::max<int32_t>(width >> i, 1);
        const std::int32_t mipHeight = std::max<int32_t>(height >> i, 1);

        VkImageBlit imageBlit = {};
        imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBlit.srcSubresource.baseArrayLayer = 0;
        imageBlit.srcSubresource.layerCount = 1;
        imageBlit.srcSubresource.mipLevel = 0;
        imageBlit.srcOffsets[ 0 ] = { 0, 0, 0 };
        imageBlit.srcOffsets[ 1 ] = { width, height, 1 };

        imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBlit.dstSubresource.baseArrayLayer = 0;
        imageBlit.dstSubresource.layerCount = 1;
        imageBlit.dstSubresource.mipLevel = i;
        imageBlit.dstOffsets[ 0 ] = { 0, 0, 0 };
        imageBlit.dstOffsets[ 1 ] = { mipWidth, mipHeight, 1 };

        vkCmdBlitImage(commandBuffer,
            image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &imageBlit,
            VK_FILTER_LINEAR);

        barrier.subresourceRange.baseMipLevel = i;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &barrier);
    }

    for (uint32_t i = 1; i < mipLevels; i++) {
        barrier.subresourceRange.baseMipLevel = i;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    }

    barrier.subresourceRange.baseMipLevel = 0;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
        0, nullptr,
        0, nullptr,
        1, &barrier);

    context->endSingleTimeCommands(commandBuffer);
    // if (context->engineSettings.useConcurrentTransferQueue) context->endSingleTimeCommands(commandBuffer, context->transferCommandPool, context->transferQueue);
    // else context->endSingleTimeCommands(commandBuffer);
}

EntityTexture::~EntityTexture() {
    if (--EntityTextures::references[image] == 0) {
        vkDestroySampler(context->device, sampler, nullptr);
        vkDestroyImageView(context->device, imageView, nullptr);
        vmaDestroyImage(context->memoryAllocator, image, allocation);
        EntityTextures::references.erase(image);
    }
}

EntityTexture::EntityTexture(const EntityTexture& other) {
    imageView = other.imageView;
    image = other.image;
    sampler = other.sampler;
    allocation = other.allocation;
    context = other.context;
    mipLevels = other.mipLevels;
    width = other.width;
    height = other.height;
    EntityTextures::references[image]++;
}

EntityTexture& EntityTexture::operator=(const EntityTexture& other) {
    return *this = EntityTexture(other);
}

EntityTexture::EntityTexture(EntityTexture&& other) noexcept
: mipLevels(other.mipLevels),
image(other.image), context(other.context), allocation(other.allocation),
width(other.width), height(other.height)
{
    imageView = other.imageView;
    sampler = other.sampler;
    EntityTextures::references[image]++;
}

EntityTexture& EntityTexture::operator=(EntityTexture&& other) noexcept {
    if (--EntityTextures::references[image] == 0) {
        vkDestroySampler(context->device, sampler, nullptr);
        vkDestroyImageView(context->device, imageView, nullptr);
        vmaDestroyImage(context->memoryAllocator, image, allocation);
        EntityTextures::references.erase(image);
    }
    imageView = other.imageView;
    image = other.image;
    sampler = other.sampler;
    allocation = other.allocation;
    context = other.context;
    mipLevels = other.mipLevels;
    width = other.width;
    height = other.height;
    return *this;
}

#include "unit_ai.hpp"

Scene::Scene(Engine* context, std::vector<std::tuple<const char *, const char *, const char *, const char *>> entities, std::array<const char *, 6> skyboxImages)
: skybox(context, skyboxImages), zSorter(InstanceZSorter(this)) {
    this->context = context;
    auto icon = new Entity(ENT_ICON);
    for (const auto& entityData : entities) {
        auto entity = new Entity(get<0>(entityData), get<1>(entityData), get<2>(entityData));
        this->entities.insert({ std::string(get<3>(entityData)), entity });
        if (entity->hasTexture) {
            entity->textureIndex = textures.size();
            textures.push_back(EntityTexture(context, { entity->textureHeight, entity->textureWidth, entity->textureChannels, entity->texturePixels }));
        }
        if (entity->hasIcon) {
            entity->iconIndex = textures.size();
            textures.push_back(EntityTexture(context, { entity->iconHeight, entity->iconWidth, entity->iconChannels, entity->iconPixels }));
        }
    }
    this->entities.insert({ std::string("icon"), icon });
    missingIcon = textures.size();
    textures.push_back(EntityTexture(context, { icon->textureHeight, icon->textureWidth, icon->textureChannels, icon->texturePixels }));
    auto weapons = loadWeaponsFromLua("weapons");
    for (const auto& weapon : weapons) {
        if (weapon->hasEntity()) {
            // I think they should all have textures?
            if (weapon->entity->hasTexture) {
                weapon->entity->textureIndex = textures.size();
                textures.push_back(EntityTexture(context, { weapon->entity->textureHeight, weapon->entity->textureWidth,
                    weapon->entity->textureChannels, weapon->entity->texturePixels }));
            }
            this->entities.insert({ weapon->entity->name, weapon->entity.get() });
        }
        this->weapons.insert({ weapon->name, weapon });
    }
    auto ents = loadEntitiesFromLua("units");
    for (const auto& entity : ents) {
        this->entities.insert({ entity->name, entity });
        if (entity->hasTexture) {
            entity->textureIndex = textures.size();
            textures.push_back(EntityTexture(context, { entity->textureHeight, entity->textureWidth, entity->textureChannels, entity->texturePixels }));
        }
        if (entity->hasIcon) {
            entity->iconIndex = textures.size();
            textures.push_back(EntityTexture(context, { entity->iconHeight, entity->iconWidth, entity->iconChannels, entity->iconPixels }));
        }
        // Populate the weapon vector
        for (auto& weapon : entity->weaponNames) {
            if (this->weapons.contains(weapon)) {
                entity->weapons.push_back(this->weapons[weapon]);
            } else {
                std::cerr << "Weapon not found: " << weapon << std::endl;
            }
        }
        entity->precompute();
    }
}

void Scene::addInstance(const std::string& name, glm::vec3 position, glm::vec3 heading) {
    assert(models.size());
    auto ent = entities[name];
    state.instances.push_back(new Instance(ent, textures.data() + ent->textureIndex, models.data() + ent->modelIndex, 0, 0, false));
    state.instances.back()->position = position;
    state.instances.back()->heading = heading;
}

void Scene::makeBuffers() {
    size_t vertexOffset = 0, indicesOffset = 0;
    for(const auto& [name, ent] : entities) {
        ent->modelIndex = models.size();
        models.push_back({ vertexOffset, indicesOffset, ent->indices.size() });
        vertexBuffer.insert(vertexBuffer.end(), ent->vertices.begin(), ent->vertices.end());
        for(const auto& idx : ent->indices) {
            indexBuffer.push_back(idx + vertexOffset);
        }
        vertexOffset += ent->vertices.size();
        indicesOffset += ent->indices.size();
    }
}

void Scene::updateUniforms(int idx) {
    float aspectRatio = static_cast<Engine *>(context)->swapChainExtent.width / (float) static_cast<Engine *>(context)->swapChainExtent.height;
    static_cast<Engine *>(context)->uniformSync->sync(idx, state.instances.size(), [this, aspectRatio] (size_t n) -> InstanceUBO * {
        return this->state.instances[n]->getUBO(static_cast<Engine *>(context)->pushConstants.view, static_cast<Engine *>(context)->camera.cached.projView,
            static_cast<Engine *>(context)->camera.cached.view_1Proj_1, aspectRatio, Config::Camera.minClip, Config::Camera.maxClip);
    });

    std::vector<uint> inPlayIndices;
    for (uint i = 0; i < state.instances.size(); i++) {
        if (state.instances[i]->inPlay) inPlayIndices.push_back(i);
    }
    uint commandCount = state.commandCount(inPlayIndices);

    auto cmdGen = state.getCommandGenerator(&inPlayIndices);

    auto lineObjectSize = std::accumulate(static_cast<Engine *>(context)->lineObjects.begin(), static_cast<Engine *>(context)->lineObjects.end(),
        0UL, [](size_t s, LineHolder *x) -> size_t {
        return s + x->lines.size();
    });
    // I don't like making extra copies (With some thinking I can make a generator function that avoids this)
    std::vector<LineUBO> lineTmp;
    lineTmp.reserve(commandCount + lineObjectSize + state.beams.size());

    for (const auto&  lineHolder : static_cast<Engine *>(context)->lineObjects) {
        lineTmp.insert(lineTmp.end(), lineHolder->lines.begin(), lineHolder->lines.end());
    }

    uint32_t lastId = UINT32_MAX;
    glm::vec3 lastDest;
    auto commandLines = [&] () -> LineUBO {
        cmdGen.next();
        auto a = cmdGen.value();
        auto id = a.which->at(a.idx);
        LineUBO ret;
        if (a.command->kind == CommandKind::MOVE) {
            ret = {
                id == lastId ? lastDest : this->state.instances[id]->position,
                a.command->data.dest,
                { 0.3f, 1.0f, 0.3f, 1.0f },
                { 0.3f, 1.0f, 0.3f, 1.0f }
            };
            lastDest = a.command->data.dest;
            lastId = id;
        } else {
            ret = { 
                { 0.0f, 0.0f, 0.0f },
                { 0.0f, 0.0f, 0.0f },
                { 0.0f, 0.0f, 0.0f, 0.0f },
                { 0.0f, 0.0f, 0.0f, 0.0f },
            };
        }
        return ret; };

    std::generate_n(std::back_inserter(lineTmp), commandCount, commandLines);

    lineTmp.insert(lineTmp.end(), state.beams.begin(), state.beams.end());

    static_cast<Engine *>(context)->nonBeamLines = commandCount + lineObjectSize;
    static_cast<Engine *>(context)->lineSync->sync(idx, lineTmp);
}

namespace CubeMaps {
    // Again I don't think I need to synchronize this
    static std::map<VkImage, uint> references = {};
}

CubeMap::CubeMap() : Textured() {}

CubeMap::CubeMap(Engine *context, std::array<const char *, 6> files) {
    this->context = context;
    std::vector<uint8_t *> pixelsBuffers;

    int height, width;
    bool set = false;
    int readHeight, readWidth, readChannles;
    for (const auto& filename : files) {
        pixelsBuffers.push_back(stbi_load(filename, &readHeight, &readWidth, &readChannles, STBI_rgb_alpha));
        if (readChannles != 4) throw std::runtime_error("Cubemap resource format incorrect.");
        // I know this is true for open gl shaders in glsl, idk about vulkan, but just to be safe for now
        if (readHeight != readWidth) throw std::runtime_error("Cubemap textures need to be square.");
        if (!set) {
            height = readHeight;
            width = readWidth;
        } else {
            if (readHeight != height || readWidth != width)
                throw std::runtime_error("Cubemap resource mismatching sizes.");
        }
    }

    VkBufferCreateInfo stagingBufInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    stagingBufInfo.size = height * width * 24;
    stagingBufInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    VmaAllocationCreateInfo stagingBufAllocCreateInfo = {};
    stagingBufAllocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    stagingBufAllocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VmaAllocation stagingBufferAllocation = VK_NULL_HANDLE;
    VmaAllocationInfo stagingBufferAllocationInfo = {};
    if (vmaCreateBuffer(context->memoryAllocator, &stagingBufInfo, &stagingBufAllocCreateInfo, &stagingBuffer, &stagingBufferAllocation, &stagingBufferAllocationInfo) != VK_SUCCESS)
        throw std::runtime_error("Unable to allocate memory for texture.");

    for (int i = 0; i < 6; i++)
        memcpy((uint8_t *)stagingBufferAllocationInfo.pMappedData + height * width * 4 * i, pixelsBuffers[i], height * width * 4);

    for (const auto& buffer : pixelsBuffers)
        stbi_image_free(buffer);

    VkImageCreateInfo imageInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    imageInfo.imageType = VK_IMAGE_TYPE_2D ;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 6;
    imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

    VmaAllocationCreateInfo imageAllocCreateInfo = {};
    imageAllocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    if (vmaCreateImage(context->memoryAllocator, &imageInfo, &imageAllocCreateInfo, &image, &allocation, nullptr) != VK_SUCCESS)
        throw std::runtime_error("Unable to create image");

    // Hm, how to use the transfer queue for this?
    // VkCommandBuffer commandBuffer = context->engineSettings.useConcurrentTransferQueue ?
    //     context->beginSingleTimeCommands(context->transferCommandPool) : context->beginSingleTimeCommands();
    VkCommandBuffer commandBuffer = context->beginSingleTimeCommands();

    VkImageMemoryBarrier imageMemoryBarrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
    imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
    imageMemoryBarrier.subresourceRange.levelCount = 1;
    imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
    imageMemoryBarrier.subresourceRange.layerCount = 6;
    imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imageMemoryBarrier.image = image;
    imageMemoryBarrier.srcAccessMask = 0;
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &imageMemoryBarrier);

    VkBufferImageCopy region = {};
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.layerCount = 6;
    region.imageExtent.width = width;
    region.imageExtent.height = height;
    region.imageExtent.depth = 1;

    vkCmdCopyBufferToImage(commandBuffer, stagingBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageMemoryBarrier.image = image;
    imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &imageMemoryBarrier);

    context->endSingleTimeCommands(commandBuffer);

    vmaDestroyBuffer(context->memoryAllocator, stagingBuffer, stagingBufferAllocation);

    VkImageViewCreateInfo textureImageViewInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    textureImageViewInfo.image = image;
    textureImageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    textureImageViewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    textureImageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    textureImageViewInfo.subresourceRange.baseMipLevel = 0;
    textureImageViewInfo.subresourceRange.levelCount = 1;
    textureImageViewInfo.subresourceRange.baseArrayLayer = 0;
    textureImageViewInfo.subresourceRange.layerCount = 6;
    vulkanErrorGuard(vkCreateImageView(context->device, &textureImageViewInfo, nullptr, &imageView), "Unable to create texture image view.");

    CubeMaps::references.insert({ image, 1 });

    VkSamplerCreateInfo samplerInfo {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;

    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    samplerInfo.anisotropyEnable = VK_TRUE;
    // For right now we just use the max supported sampler anisotrpy
    samplerInfo.maxAnisotropy = context->maxSamplerAnisotropy;

    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.maxLod = 1.0f;
    samplerInfo.mipLodBias = 0.0f;

    vulkanErrorGuard(vkCreateSampler(context->device, &samplerInfo, nullptr, &sampler), "Failed to create texture sampler.");
}

CubeMap::~CubeMap() {
    if (--CubeMaps::references[image] == 0) {
        vkDestroySampler(context->device, sampler, nullptr);
        vkDestroyImageView(context->device, imageView, nullptr);
        vmaDestroyImage(context->memoryAllocator, image, allocation);
        CubeMaps::references.erase(image);
    }
}

CubeMap::CubeMap(const CubeMap& other) {
    assert(other.image);
    imageView = other.imageView;
    image = other.image;
    sampler = other.sampler;
    allocation = other.allocation;
    context = other.context;
    CubeMaps::references[image]++;
}

CubeMap& CubeMap::operator=(const CubeMap& other) {
    assert(other.image);
    return *this = CubeMap(other);
}

CubeMap::CubeMap(CubeMap&& other) noexcept
: image(other.image), context(other.context), allocation(other.allocation) {
    assert(other.image);
    imageView = other.imageView;
    sampler = other.sampler;
    CubeMaps::references[image]++;
}

CubeMap& CubeMap::operator=(CubeMap&& other) noexcept {
    assert(other.image);
    if (--CubeMaps::references[image] == 0) {
        vkDestroySampler(context->device, sampler, nullptr);
        vkDestroyImageView(context->device, imageView, nullptr);
        vmaDestroyImage(context->memoryAllocator, image, allocation);
        CubeMaps::references.erase(image);
    }
    imageView = other.imageView;
    image = other.image;
    sampler = other.sampler;
    allocation = other.allocation;
    context = other.context;
    CubeMaps::references[image]++;
    return *this;
}

namespace GuiTextures {
    static FT_Library library;
    static FT_Face face;
    static Engine *context;
}

std::shared_ptr<GuiTexture> GuiTexture::defaultTexture() { return GuiTextures::defaultTexture; }

GuiTexture::GuiTexture(Engine *context, void *pixels, int width, int height, int channels, int strideBytes,
    VkFormat format, VkFilter filter, bool useRenderQueue, bool storable) {
    // force using a linear color space format
    assert(!isSRGB(format));

    this->context = context;
    widenessRatio = (float)width / height;

    VkBufferCreateInfo stagingBufInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    stagingBufInfo.size = height * width * channels;
    stagingBufInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    VmaAllocationCreateInfo stagingBufAllocCreateInfo = {};
    stagingBufAllocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    stagingBufAllocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VmaAllocation stagingBufferAllocation = VK_NULL_HANDLE;
    VmaAllocationInfo stagingBufferAllocationInfo = {};
    if (vmaCreateBuffer(context->memoryAllocator, &stagingBufInfo, &stagingBufAllocCreateInfo, &stagingBuffer,
        &stagingBufferAllocation, &stagingBufferAllocationInfo) != VK_SUCCESS)
            throw std::runtime_error("Unable to allocate memory for texture.");

    for (int i = 0; i < height; i++) {
        memcpy((uint8_t *)stagingBufferAllocationInfo.pMappedData + i * width * channels, (uint8_t *)pixels + i * strideBytes, width * channels);
    }

    VkImageUsageFlags imageUsage = storable ?
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT : VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

    VkImageCreateInfo imageInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    imageInfo.imageType = VK_IMAGE_TYPE_2D ;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = imageUsage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;

    VmaAllocationCreateInfo imageAllocCreateInfo = {};
    imageAllocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    if (vmaCreateImage(context->memoryAllocator, &imageInfo, &imageAllocCreateInfo, &image, &allocation, nullptr) != VK_SUCCESS)
        throw std::runtime_error("Unable to create image");

    VkCommandBuffer commandBuffer = useRenderQueue ? context->beginSingleTimeCommands() : context->beginSingleTimeCommands(context->guiCommandPool);

    VkImageMemoryBarrier imageMemoryBarrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
    imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
    imageMemoryBarrier.subresourceRange.levelCount = 1;
    imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
    imageMemoryBarrier.subresourceRange.layerCount = 1;
    imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imageMemoryBarrier.image = image;
    imageMemoryBarrier.srcAccessMask = 0;
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &imageMemoryBarrier);

    VkBufferImageCopy region = {};
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.layerCount = 1;
    region.imageExtent.width = width;
    region.imageExtent.height = height;
    region.imageExtent.depth = 1;

    vkCmdCopyBufferToImage(commandBuffer, stagingBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    // Assume we will access this in the compute shader
    if (!storable) {
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageMemoryBarrier.image = image;
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(
            commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &imageMemoryBarrier);
    }

    if (useRenderQueue) {
        context->endSingleTimeCommands(commandBuffer);
    } else {
        context->endSingleTimeCommands(commandBuffer, context->guiCommandPool, context->guiGraphicsQueue);
    }

    vmaDestroyBuffer(context->memoryAllocator, stagingBuffer, stagingBufferAllocation);

    VkImageViewCreateInfo textureImageViewInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    textureImageViewInfo.image = image;
    textureImageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    textureImageViewInfo.format = format;
    textureImageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    textureImageViewInfo.subresourceRange.baseMipLevel = 0;
    textureImageViewInfo.subresourceRange.levelCount = 1;
    textureImageViewInfo.subresourceRange.baseArrayLayer = 0;
    textureImageViewInfo.subresourceRange.layerCount = 1;
    textureImageViewInfo.pNext = nullptr;
    vulkanErrorGuard(vkCreateImageView(context->device, &textureImageViewInfo, nullptr, &imageView), "Unable to create texture image view.");

    sampler = TextResource::sampler_;

#ifdef LOG_GUI_TEXTURES
    // std::ostringstream ss;
    // ss << std::hex << "Created " << this->imageView << " " << std::this_thread::get_id() << std::endl;
    // ss << boost::stacktrace::stacktrace() << std::endl;
    // textureLog << ss.str();
#endif
}

GuiTexture::~GuiTexture() {
    if (invalid) return;
    vkDestroyImageView(context->device, imageView, nullptr);
    vmaDestroyImage(context->memoryAllocator, image, allocation);
    invalid = true;
}

bool GuiTexture::operator==(const GuiTexture& other) const {
    return image == other.image;
}

ResourceID GuiTexture::resourceID() const {
    return image;
}

void GuiTexture::makeComputable() {
    auto buffer = context->beginSingleTimeCommands();

    VkImageMemoryBarrier imageMemoryBarrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
    imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
    imageMemoryBarrier.subresourceRange.levelCount = 1;
    imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
    imageMemoryBarrier.subresourceRange.layerCount = 1;
    imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    imageMemoryBarrier.image = image;
    imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;

    vkCmdPipelineBarrier(
        buffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &imageMemoryBarrier);

    context->endSingleTimeCommands(buffer);
}

// This font stuff is ultrajank
// It may seem less than ideal that we render to a buffer like this, but indeed this is how freetype does things anyways, with this psuedo global state
namespace GuiTextures {
    static std::mutex ftLock;

    static void *textTextureBuffer;
    void destroyTextTextureBuffer() {
        ::operator delete(textTextureBuffer);
    }
    const int maxTextWidth = 2048;

    // I know what you are thinking...
    //  ...that this should be an object since I practically gave it a destructor with the atexit bit
    void initFreetype2(Engine *context) {
        std::scoped_lock l(ftLock);
        GuiTextures::context = context;
        FT_Error error = FT_Init_FreeType(&library);
        if (error) throw std::runtime_error("Unable to initialize freetype.");

        error = FT_New_Face(GuiTextures::library, "fonts/FreeSans.ttf", 0, &face);
        if (error == FT_Err_Unknown_File_Format) throw std::runtime_error("Unsupported font format.");
        else if (error) throw std::runtime_error("Unable to read font file.");

        // std::cout << "Font contains glyph count: " << face->num_glyphs << std::endl;

        error = FT_Set_Char_Size(face, 0, 12*64, 300, 300);
        if (error) throw std::runtime_error("Unable to setup font.");

        error = FT_Load_Glyph(face, FT_Get_Char_Index(face, 'T'), 0);
        if (error) throw std::runtime_error("Unable to load glyph.");
        // error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_SDF);
        error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_SDF);
        if (error) throw std::runtime_error("Unable to render glyph.");
        maxGlyphHeight = face->glyph->bitmap_top;

        textTextureBuffer = ::operator new(maxGlyphHeight * 2 * maxTextWidth);
        atexit(destroyTextTextureBuffer);
    }

    std::shared_ptr<GuiTexture> makeGuiTexture(const std::string& file) {
        if (!std::filesystem::exists(file)) {
            std::string msg = "Unable to find file: ";
            msg += file;
            throw std::runtime_error(msg);
        }

        textureCacheMutex.lock();
        if (textureCache.contains(file)) {
            auto ret = textureCache.at(file);
            textureCacheMutex.unlock();
            return ret;
        }
        textureCacheMutex.unlock();

        int texWidth, texHeight, texChannels;
        auto buf = stbi_load(file.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

        assert(texChannels == 4);
        auto ret = std::make_shared<GuiTexture>(context, buf, texWidth, texHeight, texChannels, texWidth * texChannels,
            VK_FORMAT_R8G8B8A8_UNORM, VK_FILTER_LINEAR);

        textureCacheMutex.lock();
        textureCache[file] = ret;
        textureCacheMutex.unlock();

        // stbi_write_png("check.png", texWidth, texHeight, texChannels, buf, texWidth * texChannels);

        stbi_image_free(buf);
        return ret;
    }

    void setDefaultTexture() {
        unsigned char value[] = {
            0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0,
            0xa0, 0xff, 0xff, 0xff, 0xff, 0xa0,
            0xa0, 0xff, 0xff, 0xff, 0xff, 0xa0,
            0xa0, 0xff, 0xff, 0xff, 0xff, 0xa0,
            0xa0, 0xff, 0xff, 0xff, 0xff, 0xa0,
            0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0,
        };
        defaultTexture = std::make_shared<GuiTexture>(context, &value, 6, 6, 1, 6, VK_FORMAT_R8_UNORM);
    }
}

template<typename T> void printIfLineUBO(const T& obj) {
    if constexpr (std::is_same<T, LineUBO>::value)
        std::cout << obj.a << " - " << obj.b << std::endl;
}

// probably can combine these 2 function with some clever template code but I don't want to spend the brainpower now
template<typename T> void DynUBOSyncer<T>::sync(int descriptorIndex, size_t count, std::function<T *(size_t idx)> func) {
    if (count > currentSize[descriptorIndex]) {
        reallocateBuffer(descriptorIndex, count + 50);
    }
    // assume for right now that this updates every valid instance
    validCount[descriptorIndex] = count;
    for (size_t i = 0; i < count; i++) {
        memcpy(static_cast<unsigned char *>(uniformBufferAllocations[descriptorIndex]->GetMappedData()) + i * uniformSkip,
            func(i), sizeof(T));
    }
}

template<typename T> void DynUBOSyncer<T>::sync(int descriptorIndex, size_t count, std::function<T ()> func) {
    if (count > currentSize[descriptorIndex]) {
        reallocateBuffer(descriptorIndex, count + 50);
    }
    // assume for right now that this updates every valid instance
    validCount[descriptorIndex] = count;
    for (size_t i = 0; i < count; i++) {
        auto tmp = func();
        // printIfLineUBO(tmp);
        memcpy(static_cast<unsigned char *>(uniformBufferAllocations[descriptorIndex]->GetMappedData()) + i * uniformSkip,
            &tmp, sizeof(T));
    }
}

template<typename T> void DynUBOSyncer<T>::sync(int descriptorIndex, const std::vector<T>& items) {
    if (items.size() > currentSize[descriptorIndex]) {
        reallocateBuffer(descriptorIndex, items.size() + 50);
    }
    // assume for right now that this updates every valid instance
    validCount[descriptorIndex] = items.size();
    for (size_t i = 0; i < items.size(); i++) {
        memcpy(static_cast<unsigned char *>(uniformBufferAllocations[descriptorIndex]->GetMappedData()) + i * uniformSkip,
            items.data() + i, sizeof(T));
    }
}

GlyphCache::GlyphCache(Engine *context, const std::vector<char32_t>& glyphsWanted, bool rebuildFontCache)
: context(context), bufferWidth(GuiTextures::maxGlyphHeight / 2), lineHeight(GuiTextures::maxGlyphHeight + GuiTextures::maxGlyphHeight / 2) {
    height = GuiTextures::maxGlyphHeight + 2 * bufferWidth;
    struct FontCacheData {
        char32_t glyph;
        uint width, height, advance;
    };

    std::vector<char> cacheBuffer;
    std::vector<std::pair<FontCacheData, std::vector<char>>> moreBuffers;
    std::map<char32_t, std::pair<FontCacheData *, char *>> fontCache;
    if (!rebuildFontCache && std::filesystem::exists("fontcache")) {
        std::ifstream cacheData("fontcache", std::ios::binary);
        cacheBuffer = std::vector<char>(std::istreambuf_iterator<char>(cacheData), {});

        auto it = cacheBuffer.begin();
        while (it != cacheBuffer.end()) {
            auto fcd = (FontCacheData *)&*it;
            // std::cout << std::hex << "Glyph: 0x" << (uint)fcd->glyph << std::dec << " - " << fcd->width << ":" << fcd->height << ":" << fcd->advance << std::endl;
            advance(it, sizeof(FontCacheData));

            fontCache.insert({ fcd->glyph, { fcd, &*it }});
            advance(it, fcd->height * fcd->width);
        }
    }

    std::scoped_lock l(GuiTextures::ftLock);
    memset(GuiTextures::textTextureBuffer, 0, GuiTextures::maxGlyphHeight * 2 * GuiTextures::maxTextWidth);
    defaultGlyph = new GuiTexture(context, GuiTextures::textTextureBuffer, defaultGlyphWidth,
        GuiTextures::maxGlyphHeight * 2, 1, GuiTextures::maxTextWidth, VK_FORMAT_R8_UNORM, VK_FILTER_LINEAR, true, true);
    defaultGlyph->makeComputable();
    uint index = 0;
    bool fontCacheModified = false;
    for (const auto& glyphCode : glyphsWanted) {
        if (fontCache.contains(glyphCode)) {
            const auto& data = fontCache.at(glyphCode);
            auto tex = std::make_shared<GuiTexture>(context, data.second, data.first->width, data.first->height, 1, data.first->width,
                VK_FORMAT_R8_UNORM, VK_FILTER_LINEAR, true, true);
            tex->makeComputable();
            glyphDatum.insert({ glyphCode, { data.first->advance, tex, index++ }});

            continue;
        }

        FT_Vector pen;
        pen.x = GuiTextures::maxGlyphHeight / 2;
        pen.y = GuiTextures::maxGlyphHeight / 2;
        memset(GuiTextures::textTextureBuffer, 0, GuiTextures::maxGlyphHeight * 2 * GuiTextures::maxTextWidth);

        // Weirdness I guess
        if (glyphCode == 0x20) {
            // FIXME For some reason I can't render space with true tpye despite looking right at it on the font file with a font editor
            // I pulled the width out anyways and just pretend (it isn't like we don't have to handle special cases anyways (newlines and the like))
            GuiTextures::face->glyph->advance.x = 25 * 64;
        } else {
            FT_UInt charCode = FT_Get_Char_Index(GuiTextures::face, glyphCode);
            FT_Error error = FT_Load_Glyph(GuiTextures::face, charCode, 0);
            if (error) throw std::runtime_error("Unable to load glyph.");
            error = FT_Render_Glyph(GuiTextures::face->glyph, FT_RENDER_MODE_SDF);
            std::stringstream ss;
            ss << "Unable to render glyph (code): 0x" << std::hex << (uint32_t)glyphCode << " freetype char index: 0x" << charCode;
            if (error) throw std::runtime_error(ss.str());

            // TODO Clean this up
            for(int i = 0; i < GuiTextures::face->glyph->bitmap.rows; i++)
                for(int j = 0; j < abs(GuiTextures::face->glyph->bitmap.pitch); j++)
                    // Ah, silent failures... the best kind (it should be obvious in context though)
                    if (GuiTextures::face->glyph->bitmap_left + pen.x + j < GuiTextures::maxTextWidth)
                        *((unsigned char *)GuiTextures::textTextureBuffer + (pen.y + GuiTextures::maxGlyphHeight - GuiTextures::face->glyph->bitmap_top + i) * GuiTextures::maxTextWidth + GuiTextures::face->glyph->bitmap_left + pen.x + j) =
                            std::max(*(GuiTextures::face->glyph->bitmap.buffer + i * abs(GuiTextures::face->glyph->bitmap.pitch) + j),
                                *((unsigned char *)GuiTextures::textTextureBuffer + (pen.y + GuiTextures::maxGlyphHeight - GuiTextures::face->glyph->bitmap_top + i) * GuiTextures::maxTextWidth + GuiTextures::face->glyph->bitmap_left + pen.x + j));

            pen.x += GuiTextures::face->glyph->advance.x >> 6;
        }
        int imageWidth = pen.x + GuiTextures::maxGlyphHeight / 2;

        fontCacheModified = true;
        moreBuffers.push_back({{ glyphCode, (uint)imageWidth, height, uint(GuiTextures::face->glyph->advance.x >> 6) }, std::vector<char>(imageWidth * height) });
        auto it = moreBuffers.rbegin()->second.begin();
        for (int i = 0; i < height; i++) {
            memcpy(&*it, (char *)GuiTextures::textTextureBuffer + i * GuiTextures::maxTextWidth, imageWidth);
            advance(it, imageWidth);
        }

        // stbi_write_png("glyph.png", GuiTextures::maxTextWidth, GuiTextures::maxGlyphHeight * 2, 1, GuiTextures::textTextureBuffer, GuiTextures::maxTextWidth);

        // ultimately I don't think it maters which queue we use, neither should be in use at this time anyways, but hey I coded the functionallity to keep them seperate
        // and maybe it might mater if the gui wants to do gpu stuff in setup up at the same time
        auto tex = std::make_shared<GuiTexture>(context, GuiTextures::textTextureBuffer, imageWidth, height, 1, GuiTextures::maxTextWidth,
            VK_FORMAT_R8_UNORM, VK_FILTER_LINEAR, true, true);
        tex->makeComputable();
        glyphDatum.insert({ glyphCode, { uint(GuiTextures::face->glyph->advance.x >> 6), tex, index++ }});
    }
    syncer = nullptr;

    if (fontCacheModified) {
        std::ofstream cacheData("fontcache", std::ios::binary | std::ios::trunc);
        std::ostreambuf_iterator<char> writer(cacheData);
        copy(cacheBuffer.begin(), cacheBuffer.end(), writer);
        for (const auto& val : moreBuffers) {
            for (int i = 0; i < sizeof(FontCacheData); i++) {
                writer = *(((char *)&val.first) + i);
                writer++;
            }
            copy(val.second.begin(), val.second.end(), writer);
        }
    }
}

void GlyphCache::writeDescriptors(VkDescriptorSet& set, uint32_t binding) {
    if (!syncer) syncer = new SSBOSyncer<IndexWidthSSBO>(context, {{ set,  3 }});
    std::array<VkWriteDescriptorSet, 1> descriptorWrites {};

    std::vector<VkDescriptorImageInfo> textureInfo(ECONF_MAX_GLYPH_TEXTURES);

    for (const auto& [code, data] : glyphDatum) {
        textureInfo[data.descriptorIndex].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        textureInfo[data.descriptorIndex].imageView = data.onGpu->imageView;
        textureInfo[data.descriptorIndex].sampler = VK_NULL_HANDLE;
        // hudTextureInfos[i].imageView = currentScene->state.instances[0].texture->textureImageView;
        // hudTextureInfos[i].sampler = currentScene->state.instances[0].texture->textureSampler;
    }
    // TODO, replace this with a texture that represents an error character
    for (int i = glyphDatum.size(); i < ECONF_MAX_GLYPH_TEXTURES; i++) {
        textureInfo[i].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        textureInfo[i].imageView = defaultGlyph->imageView;
        textureInfo[i].sampler = VK_NULL_HANDLE;
    }

    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = set;
    descriptorWrites[0].dstBinding = binding;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    descriptorWrites[0].descriptorCount = textureInfo.size();
    descriptorWrites[0].pImageInfo = textureInfo.data();

    vkUpdateDescriptorSets(context->device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
}

// returns the width of the needed buffer
std::pair<uint, uint> GlyphCache::writeUBO(GlyphInfoUBO *glyphInfo, const std::string& str, bool cacheWidth) {
    uint maxWidth = 0;
    int index = 0;
    IndexWidthSSBO *buf = nullptr;

    auto ss = std::stringstream { str };

    glyphInfo->lineHeight = lineHeight;
    glyphInfo->pixelOffset = bufferWidth;

    glyphInfo->lineCount = 0;
    for (std::string line; std::getline(ss, line);) {
        uint width = 0;
        auto wstr = converter.from_bytes(line);
        for (const auto& chr : wstr) {
            buf = syncer->ensureSize(buf, index);
            if (!glyphDatum.contains(chr)) {
                std::cerr << "Unsuported character: " << std::hex << (uint32_t)chr << std::endl;
                // TODO continuing is not the correct behavior, priting an error character should be correct
                continue;
            }
            const auto& data = glyphDatum.at(chr);
            buf[index].index = data.descriptorIndex;
            buf[index].width = data.width;
            width += data.width;
            index++;
        }
        glyphInfo->writeCounts[glyphInfo->lineCount * 4] = wstr.length();

        glyphInfo->lineCount++;
        maxWidth = std::max(width, maxWidth);
    }
    glyphInfo->totalWidth = maxWidth + 2 * bufferWidth;

    syncer->sync();

    if (cacheWidth && !cachedWidths.contains(str)) cachedWidths.insert({ str, glyphInfo->totalWidth });
    return { glyphInfo->totalWidth, height + lineHeight * (glyphInfo->lineCount - 1) };
}

GlyphCache::~GlyphCache() {
    delete defaultGlyph;
    delete syncer;
}

TextResource::TextResource(Engine *context, uint height, uint width)
: Textured(), context(context), widenessRatio((float)width / height) {
    VkDeviceSize imageSize = width * height;

    VkImageCreateInfo imageInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = VK_FORMAT_R8_UNORM;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0;
    // imageInfo.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;

    VmaAllocationCreateInfo imageAllocCreateInfo = {};
    imageAllocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    if (vmaCreateImage(context->memoryAllocator, &imageInfo, &imageAllocCreateInfo, &image, &allocation, nullptr) != VK_SUCCESS)
        throw std::runtime_error("Unable to create image");

    VkImageViewCreateInfo imageViewInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    imageViewInfo.image = image;
    imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewInfo.format = VK_FORMAT_R8_UNORM;
    imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageViewInfo.subresourceRange.baseMipLevel = 0;
    imageViewInfo.subresourceRange.levelCount = 1;
    imageViewInfo.subresourceRange.baseArrayLayer = 0;
    imageViewInfo.subresourceRange.layerCount = 1;
    imageViewInfo.pNext = nullptr;
    vulkanErrorGuard(vkCreateImageView(context->device, &imageViewInfo, nullptr, &imageView), "Unable to create texture image view.");

    sampler = TextResource::sampler_;
}

TextResource::~TextResource() {
    if (imageView) vkDestroyImageView(context->device, imageView, nullptr);
    if (image) vmaDestroyImage(context->memoryAllocator, image, allocation);
}

void TextResource::writeDescriptor(VkDescriptorSet set) {
    VkWriteDescriptorSet descriptorWrite {};

    VkDescriptorImageInfo textureInfo {};

    textureInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    textureInfo.imageView = imageView;
    textureInfo.sampler = VK_NULL_HANDLE;

    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = set;
    descriptorWrite.dstBinding = 1;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &textureInfo;

    vkUpdateDescriptorSets(context->device, 1, &descriptorWrite, 0, nullptr);
}

template<typename T>
T *SSBOSyncer<T>::ensureSize(T *buf, size_t count) {
    if (count <= this->currentSize) {
        if (buf == nullptr) return (T *)bufferAllocation->GetMappedData();
        return buf;
    }
    if (buf == nullptr) buf = (T *)bufferAllocation->GetMappedData();
    auto bufferOld = buffer;
    auto bufferAllocationOld = bufferAllocation;

    VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufferInfo.size = (count + increment) * sizeof(T);
    bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    VmaAllocationCreateInfo bufferAllocationInfo = {};
    bufferAllocationInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
    bufferAllocationInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

    if (vmaCreateBuffer(context->memoryAllocator, &bufferInfo, &bufferAllocationInfo, &buffer,
        &bufferAllocation, nullptr) != VK_SUCCESS)
        throw std::runtime_error("Unable to allocate memory for uniform buffers.");

    memcpy(bufferAllocation->GetMappedData(), buf, currentSize * sizeof(T));

    currentSize = count + increment;

    info.buffer = buffer;

    vmaDestroyBuffer(context->memoryAllocator, bufferOld, bufferAllocationOld);
    return (T *)bufferAllocation->GetMappedData();
}

GuiTexture::GuiTexture() : invalid(true) {}

GuiTexture::GuiTexture(Engine *context, VkImage image, VmaAllocation allocation, VkImageView imageView, float widenessRatio)
: widenessRatio(widenessRatio), image(image), allocation(allocation), context(context), invalid(false) {
    this->imageView = imageView;
    sampler = TextResource::sampler_;

#ifdef LOG_GUI_TEXTURES
    // std::ostringstream ss;
    // ss << std::hex << "Created " << this->imageView << " " << std::this_thread::get_id() << std::endl;
    // ss << boost::stacktrace::stacktrace() << std::endl;
    // textureLog << ss.str();
#endif
}

GuiTexture *TextResource::toGuiTexture(std::unique_ptr<TextResource>&& textResource) {
    auto that = textResource.get();
    auto ret = new GuiTexture(that->context, that->image, that->allocation, that->imageView, that->widenessRatio);
    that->imageView = VK_NULL_HANDLE;
    that->image = VK_NULL_HANDLE;
    return ret;
}

// This blocks until completion (Do not use in the render thread!)
std::shared_ptr<GuiTexture> GlyphCache::makeGuiTexture(const std::string& str) {
    ComputeOp op, done;
    op.kind = ComputeKind::TEXT;
    op.data = (void *)&str;
    auto id = context->manager->submitJob(std::move(op));
    while (true) {
        if (context->manager->getJob(id, done)) {
            auto tex = reinterpret_cast<GuiTexture *>(done.data);
            auto ret = std::shared_ptr<GuiTexture>(tex);
            return ret;
        }
        std::this_thread::yield();
    }
}

ComputeManager::ComputeManager(Engine *context)
: context(context) {
    VkFenceCreateInfo fenceInfo {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    vulkanErrorGuard(vkCreateFence(context->device, &fenceInfo, nullptr, &fence), "Failed to create compute fence.");

    computeThread = std::thread(&ComputeManager::poll, this);
}

ComputeManager::~ComputeManager() {
    submitJob({ ComputeKind::TERMINATE });
    std::cout << "Waiting to join compute thread..." << std::endl;
    computeThread.join();
    vkDestroyFence(context->device, fence, nullptr);
    while (!outList.empty()) {
        auto op = outList.back();
        if (op.kind == ComputeKind::TEXT) {
            delete reinterpret_cast<GuiTexture *>(op.data);
        }
        outList.pop_back();
    }
}

uint64_t ComputeManager::submitJob(ComputeOp&& op) {
    static std::atomic<int> id;
    op.id = id;
    std::scoped_lock l(inLock);
    inQueue.push(op);
    return id++;
}

bool ComputeManager::getJob(uint64_t id, ComputeOp& op) {
    std::scoped_lock l(outLock);
    for (auto it = outList.begin(); it != outList.end(); it++) {
        if (it->id == id) {
            op = *it;
            outList.erase(it);
            return true;
        }
    }
    return false;
}

// TODO This needs to be refactored to make the concerns of the actual compute stuff seperate from the polling
void ComputeManager::poll() {
    bool running = true;
    ComputeOp op, *runningOp = nullptr;
    std::unique_ptr<TextResource> textResource;
    while (running) {
        if (runningOp && vkWaitForFences(context->device, 1, &fence, VK_TRUE, 0) == VK_SUCCESS) {
            ComputeOp result;
            switch (op.kind) {
                case ComputeKind::TEXT:
                    result.kind = ComputeKind::TEXT;
                    result.data = TextResource::toGuiTexture(std::move(textResource));
                    result.id = op.id;
                    outLock.lock();
                    outList.push_back(std::move(result));
                    outLock.unlock();
                    break;
                default:
                    break;
            }
            bufferFreer();
            vkResetFences(context->device, 1, &fence);
            runningOp = nullptr;
        } else if (runningOp) {
            std::this_thread::yield();
            continue;
        } else if (!runningOp) {
            inLock.lock();
            if (!inQueue.empty()) {
                op = inQueue.front();
                inQueue.pop();
                inLock.unlock();

                switch (op.kind) {
                    case ComputeKind::TEXT:
                        {
                            auto wh = context->glyphCache->writeUBO((GlyphInfoUBO *)context->sdfUBOAllocation->GetMappedData(),
                                *reinterpret_cast<std::string *>(op.data));

                            const uint& width = wh.first;
                            const uint& height = wh.second;
                            textResource = std::make_unique<TextResource>(context, height, width);
                            textResource->writeDescriptor(context->computeSets[Engine::CP_SDF_BLIT]);
                            
                            auto buffer = context->beginSingleTimeCommands(context->computeCommandPool);

                            VkImageMemoryBarrier imageMemoryBarrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
                            imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                            imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                            imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                            imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
                            imageMemoryBarrier.subresourceRange.levelCount = 1;
                            imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
                            imageMemoryBarrier.subresourceRange.layerCount = 1;
                            imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                            // This seems wrong that we have to make it to general layout
                            imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
                            imageMemoryBarrier.image = textResource->image;
                            imageMemoryBarrier.srcAccessMask = 0;
                            imageMemoryBarrier.dstAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;

                            vkCmdPipelineBarrier(
                                buffer,
                                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                0,
                                0, nullptr,
                                0, nullptr,
                                1, &imageMemoryBarrier);

                            vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_COMPUTE, context->computePipelines[Engine::CP_SDF_BLIT]);
                            vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_COMPUTE, context->computePipelineLayouts[Engine::CP_SDF_BLIT], 0, 1,
                                &context->computeSets[Engine::CP_SDF_BLIT], 0, nullptr);
                            vkCmdDispatch(buffer, context->glyphCache->height, 1, 1);

                            imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                            imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                            imageMemoryBarrier.image = textResource->image;
                            imageMemoryBarrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
                            imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                            vkCmdPipelineBarrier(
                                buffer,
                                VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                0,
                                0, nullptr,
                                0, nullptr,
                                1, &imageMemoryBarrier);


                            bufferFreer = context->endSingleTimeCommands(buffer, context->computeCommandPool, context->computeQueue, fence);
                            runningOp = &op;

                            if (op.deleteData) delete reinterpret_cast<std::string *>(op.data);
                        }
                        break;
                    case ComputeKind::TERMINATE:
                        running = false;
                        break;
                    default:
                        break;
                }

            }
            inLock.unlock();
        }
    }
}