#pragma once

#include "utilities.hpp"

#include <boost/lockfree/spsc_queue.hpp>
#include <chrono>
#include <mutex>
#include <thread>
#include <queue>

// I am not sure about using yaml for the gui layout stuff (xml might be better because I could specify a schema and validate it easily)
#include <yaml-cpp/yaml.h>

// This is by no mean a high performance gui, when something is out of date it rebuilds all the vertex data from the tree
// I just want to get it working though for right now (avoid that premature optimization)

// For right now lets just draw triangles and get the rest right
struct GuiVertex {
    glm::vec3 pos;
    glm::vec4 color;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription {};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(GuiVertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions {};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(GuiVertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(GuiVertex, color);

        return attributeDescriptions;
    }
};

// for the quickly updating stuff in the gui (right now just the drag box)
struct GuiPushConstant {
    glm::vec2 dragBox[2];
};

class GuiComponent {
public:
    GuiComponent(uint32_t color, std::pair<float, float> c0, std::pair<float, float> c1, int layer);
    ~GuiComponent();
    GuiComponent(const GuiComponent& other) = delete;
    GuiComponent(GuiComponent&& other) noexcept = delete;
    GuiComponent& operator=(const GuiComponent& other) = delete;
    GuiComponent& operator=(GuiComponent&& other) noexcept = delete;

    std::vector<GuiComponent *> children;
    GuiComponent *parent;
    // size_t vertexCount();
    // void appendVerticies(std::back_insert_iterator<std::vector<GuiVertex>> vertexIterator);
    // void signalDirty(); // ???? How did I think I was going to use this (I guess components can change their own state or something)

    std::vector<GuiVertex> vertices;

    // indices to parent
    void addComponent(std::queue<int>& childIdices, GuiComponent *component, GuiComponent *parent);
    // indices to child
    void removeComponent(std::queue<int>& childIndices);
private:
    bool layoutOnly;
};

class GuiCommandData {
public:
    std::queue<int> childIndices;
    GuiComponent *component;
};

class Gui {
public:
    Gui();
    ~Gui();

    // These should be automatically deleted, but I want to be sure
    Gui(const Gui& other) = delete;
    Gui(Gui&& other) noexcept = delete;
    Gui& operator=(const Gui& other) = delete;
    Gui& operator=(Gui&& other) noexcept = delete;

    size_t updateBuffer(void *buffer, size_t max);
    // Lets not reinvent the wheel here even if stl is sometimes slow

    // set this to true to indicate the need to update the buffers in vram
    bool rebuilt;

    // Takes normalized device coordinates
    // I think these are in the wrong place
    static constexpr float layerZOffset = 0.001f;
    static std::vector<GuiVertex> rectangle(std::pair<float, float> c0, std::pair<float, float> c1, glm::vec4 color, int layer);
    
    void setDragBox(std::pair<float, float> c0, std::pair<float, float> c1);

    // Gui is going to be a seperate thread
    GuiPushConstant *pushConstant();
    void lockPushConstant();
    void unlockPushConstant();

    enum GuiAction {
        GUI_ADD,
        GUI_REMOVE,
        GUI_TERMINATE
    };

    struct GuiCommand {
        GuiAction action;
        GuiCommandData *data;
    };

    void submitCommand(GuiCommand command);
private:
    std::thread guiThread;
    boost::lockfree::spsc_queue<GuiCommand, boost::lockfree::capacity<1024>> guiCommands;

    std::mutex constantMutex;
    GuiPushConstant _pushConstant;
    std::mutex dataMutex;
    std::vector<GuiVertex> data;
    std::vector<GuiVertex> dragBox;

    void rebuildBuffer();

    void pollChanges();

    static constexpr auto pollInterval = std::chrono::milliseconds(5);
    // This matches with a value in the hud vertex shader
};

class Panel {
public:
    Panel(const char *filename);

private:
    YAML::Node root;
};