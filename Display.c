#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "Display.h"
#include <sys/time.h>
#include <lodepng.h>
#define _USE_MATH_DEFINES
#include <math.h>

/**********************
 * Struct Definitions *
 **********************/

/******************
 * Device Structs *
 ******************/
#define NOSTAGE
#undef NOSTAGE
typedef struct QueueFamilies_T {
    unsigned char familiesFound;
    unsigned int graphicsFamily;
    unsigned int presentFamily;
} QueueFamilies;

typedef struct SwapChainSupportDetails_T {
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR* formats;
    uint formatCount;
    VkPresentModeKHR* presentModes;
    uint modeCount;
} SwapChainSupportDetails;

/******************
 * Render Structs *
 ******************/

typedef struct positionVertex {
    __attribute__((unused))  short x;
    __attribute__((unused))  short y;
    __attribute__((unused))  short z;
} positionVertex;

typedef struct colorVertex {
    __attribute__((unused)) unsigned char red;
    __attribute__((unused)) unsigned char green;
    __attribute__((unused)) unsigned char blue;
    __attribute__((unused)) unsigned char alpha;
} colorVertex;

typedef struct textureVertex {
    //unsigned short x;
    //unsigned short y;
    __attribute__((unused)) float x;
    __attribute__((unused)) float y;

} textureVertex;

typedef unsigned short textureIndex;

typedef struct vertexArrayObject {
    __attribute__((unused)) positionVertex position; /**Specify Vertex Position on Screen*/
    __attribute__((unused)) colorVertex color; /**Specify Vertex Color on Screen*/
    __attribute__((unused))textureVertex texture; /**Specify Vertex Texture on Screen*/
    textureIndex index;
} vertexArrayObject;

typedef struct uniformBufferObject {
    float model[4][4];
    unsigned int texW[32];
    unsigned int texH[32];
} uniformBufferObject;

#ifndef NDEBUG
/**********************
 * Validation Objects *
 * ********************/

const int vLayerCount = 1; /**Number of Validation layers in use*/

/**Identifiers for Validation Layers for Debugging*/
const char* validationLayers[1] = { "VK_LAYER_KHRONOS_validation" };

VkDebugUtilsMessengerEXT debugMessenger; /**Debugging Object*/

#endif

/********************
 * Device Variables *
 ********************/
const int extensionCount = 2; /**Number of Device Extensions*/

const char* requiredExtensions[2] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_UNIFORM_BUFFER_STANDARD_LAYOUT_EXTENSION_NAME }; /**List of Device Extensions*/

/**Reference to GPU Device to be used*/
static VkPhysicalDevice pDevice = 0;

/**Logical consolidation of GPU Device tools that will be used*/
static VkDevice lDevice;

/**Graphics Operation Queue*/
static VkQueue graphicsQueue;

/**Presentation Operation Queue*/
static VkQueue presentQueue;

/**Extra Bits for More Queue Family Types*/
typedef enum VkQueueExtraFlagBits {
    VK_QUEUE_PRESENT_BIT = 0x00000003
} VkQueueExtraFlagBits;

/********************
 * Render Variables *
 ********************/
/**Definition for the Uniform Buffer Object*/
VkDescriptorSetLayout descriptorSetLayout;

VkDescriptorPool descriptorPool;

VkDescriptorSet* descriptorSets;

/**Routine Object for Rendering Graphics*/
static VkPipelineLayout pipelineLayout;

/**Graphics Pipeline*/
static VkPipeline gPipeline;

/**Determines How Render Operation is performed in Memory*/
static VkRenderPass renderPass;

#ifndef NOSTAGE
/**Where Vertex Data is Defined Before Transferring it to the Physical Device*/
VkBuffer stagingBuffer;

/**Memory for Vertex Staging Buffer */
VkDeviceMemory stagingBufferMemory;

/**Where Index Data is Defined Before Transferring it to the Physical Device*/
VkBuffer iStagingBuffer;

/**Memory for Index Staging Buffer */
VkDeviceMemory iStagingBufferMemory;
#endif

/**Array where Vertex Data is defined*/
vertexArrayObject* vData;

/**Array where Index Data is Defined*/
unsigned short* iData;

/**Uniform Transformation Data definition*/

uniformBufferObject uPrototype;
uniformBufferObject* uData;

/**Storage for Vertex Data*/
static VkBuffer vertexBuffer;

/**Allocated Memory for Vertex Data*/
static VkDeviceMemory vertexBufferMemory;

/**Storage for Index Data*/
static VkBuffer indexBuffer;

/**Allocated Memory for Index Data*/
static VkDeviceMemory indexBufferMemory;

/**Number of Textures*/


/**Storage For Texture Data*/
static VkImage textureImage[2];

/**ALlocated Memory for Texture Data*/
static VkDeviceMemory textureImageMemory[2];

static VkImageView textureImageView[2];



static uint texCount = 2;

static VkSampler textureSampler;
vertexArrayObject vertices[8] = {
        {{512, 0, 0}, {255, 255, 255, 255}, {1024, 0}, 0},
        {{1024, 0, 0}, {255, 255, 255, 255}, {2047, 0}, 0},
        {{1024, 1024, 0}, {255, 255, 255, 255}, {2047,1023}, 0},
        {{512, 1024, 0}, {255, 255, 255, 255}, {1024,1023}, 0},
        {{0, 0, 0}, {255, 255, 255, 255}, {0, 0}, 1},
        {{512, 0, 0}, {255, 255, 255, 255}, {1023, 0}, 1},
        {{512, 1024, 0}, {255, 255, 255, 255}, {1023,1023}, 1},
        {{0, 1024, 0}, {255, 255, 255, 255}, {0,1023}, 1}
};

typedef short indexArrayObject[6];

indexArrayObject indices[2] = {{ 0, 1, 2, 2, 3, 0},
                               {4,5,6,6,7,4}};

unsigned short texW[32];
unsigned short texH[32];

uniformBufferObject uniformData;

/**Storage for Uniform Data*/
VkBuffer* uniformBuffers;

/**Allocated Memory for Uniform Data*/
VkDeviceMemory* uniformBuffersMemory;


#define INDEXTYPE unsigned short
/************************
 * Swap Chain Variables *
 ************************/

/**Buffer for images in the swap chain*/
static VkFramebuffer* swapChainFrameBuffers;

/**Handles presenting images on the window*/
static VkSwapchainKHR swapChain;

/**Settings for Swap Chain Presentation*/
static VkFormat swapChainImageFormat;

/**Swap Chain Size*/
static VkExtent2D swapChainExtent;

/**Images in Swap Chain*/
static uint swapChainSize;

/**Images to be presented to the Swap Chain*/
static VkImage* swapChainImages;

/**How the images will be presented to the swap chain*/
static VkImageView* swapChainImageViews;

/**Notification Object For when the Window has been resized*/
static bool resized;
/*****************************
 * Synchronization Variables *
 *****************************/

/**Semaphores for ImageView Synchronization*/
VkSemaphore* imageAvailableSemaphores;

/**Semaphores for Next Render Sync*/
VkSemaphore* renderFinishedSemaphores;

VkFence* inFlightFences;
VkFence* imagesInFlight;
const int MAX_FRAMES_IN_FLIGHT = 2;
int currentFrame = 0;

/*********************
 * Command Variables *
 *********************/

/**Allocation to Store Command Buffers*/
static VkCommandPool commandPool;

/**Commands to be made from the Logical Device*/
static VkCommandBuffer* commandBuffers;

/********************
 * Window Variables *
 ********************/

/**Visualizer object to connect GLFW and Vulkan*/
static VkSurfaceKHR surface;

/**Window Instance*/
static GLFWwindow* window;

/*********************
 * General Variables *
 *********************/

/**Vulkan Context Object*/
static VkInstance vInstance;


#ifndef NDEBUG
/************************
 * Validation Functions *
 ************************/

bool checkValidationLayerSupport() {
    unsigned int  layerCount; /**Number of Supported Layers*/
    vkEnumerateInstanceLayerProperties(&layerCount, 0); /**Get Count of Layers*/
    VkLayerProperties availableLayers[layerCount]; /**Array of Layers*/
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers); /**Use layer count to populate layer information*/

    for(unsigned int i = 0; i < vLayerCount; i++) { /**Iterate through requested Layers*/
        unsigned char layerFound = 0; /**Check if Layer was found*/
        const char* layerName = validationLayers[i];
        for(unsigned int j = 0; j < layerCount; j++)  /**Iterate through found layers, check if requested layer is present*/
            if(!strcmp(layerName, availableLayers[j].layerName)) {
                layerFound = 1;
                break;
            }
        if(!layerFound) return FALSE; /**If layer is not found, return false*/
    }

    return TRUE; /**Return True if all the requested layers are present*/
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance inst, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator) {
    /**Load Function from Vulkan Instance*/
    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(inst, "vkCreateDebugUtilsMessengerEXT");

    /**If Function is present, return the result*/
    if(func) return func(inst, pCreateInfo, pAllocator,  &debugMessenger); else return VK_ERROR_EXTENSION_NOT_PRESENT;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData) {
    /**Output Message Based on Severity and Type*/
    if(messageSeverity & (VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)) {
        fprintf(stderr,"VK ");
        if(messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) fprintf(stderr,"General ");
        if(messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) fprintf(stderr,"Validation ");
        if(messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) fprintf(stderr,"Performance ");
        fprintf(stderr, "Alert - %s\n", pCallbackData->pMessage);
    } else  {
        printf("VK ");
        if(messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) printf("General ");
        if(messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) printf("Validation ");
        if(messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) printf("Performance ");
        printf("Message - %s\n", pCallbackData->pMessage);
    }

    return VK_FALSE;
}

void DestroyDebugUtilsMessengerEXT(VkInstance inst, VkDebugUtilsMessengerEXT dMessenger, const VkAllocationCallbacks* pAllocator) {
    /**Load Function from Vulkan Instance*/
    PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(inst, "vkDestroyDebugUtilsMessengerEXT");

    /**If Function is present, return the result*/
    if (func != 0) func(inst, dMessenger, pAllocator);
}

#endif


/********************
 * Device Functions *
 ********************/

void pickDevice() {
    /**Get Number of Available Devices*/
    unsigned int deviceCount;
    vkEnumeratePhysicalDevices(vInstance, &deviceCount, NULL);

    /**If no devices available exit*/
    if (!deviceCount) {
        fprintf(stderr, "Could Not Find any Compatible Devices\n");
        exit(EXIT_FAILURE);
    }

    /**Get List of all Found Devices*/
    VkPhysicalDevice devices[deviceCount];
    vkEnumeratePhysicalDevices(vInstance, &deviceCount, devices);

    /**Check if Device is Suitable*/
    for(unsigned int i = 0; i < deviceCount; i++) {
        VkPhysicalDevice dev = devices[i];
        if(deviceSuitable(dev)) {
            pDevice = dev;
            break;
        }
    }


    if(!pDevice) {
        fprintf(stderr,"No Compatible GPU Found\n");
        exit(EXIT_FAILURE);
    }
}

bool deviceSuitable(VkPhysicalDevice dev) {

    /**Get Properties of Graphics Device*/
    VkPhysicalDeviceProperties dProperties;
    vkGetPhysicalDeviceProperties(dev, &dProperties);

    /**Get Features of Device*/
    VkPhysicalDeviceFeatures dFeatures;
    vkGetPhysicalDeviceFeatures(dev, &dFeatures);

    /**Get Queue Families of Device*/
    QueueFamilies devFamilies = findQueueFamilies(dev);

    /**Check that Devices are Supported*/
    bool extensionsSupported;
    if(!(extensionsSupported = checkDeviceExtensionSupport(dev))) return FALSE;

    /**Check if Swap Chain is Adequate*/
    bool swapChainAdequate = FALSE;
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(dev);
    swapChainAdequate = swapChainSupport.formatCount && swapChainSupport.modeCount;


    /**Return true if the following conditions are met*/
    return dFeatures.geometryShader && dFeatures.samplerAnisotropy &&
           (devFamilies.familiesFound & VK_QUEUE_GRAPHICS_BIT & VK_QUEUE_PRESENT_BIT) &&
           swapChainAdequate;
}

QueueFamilies findQueueFamilies(VkPhysicalDevice dev) {

    /**Initialize Return Object*/
    QueueFamilies foundFamilies;

    /**Get Number of Queue Families*/
    unsigned int familyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(dev,&familyCount, NULL);

    /**Exit if no Queues are Found*/
    if(!familyCount) {
        fprintf(stderr, "No Queue Families Available\n");
        exit(EXIT_FAILURE);
    }

    /**Get Family Properties*/
    VkQueueFamilyProperties families[familyCount];
    vkGetPhysicalDeviceQueueFamilyProperties(dev,&familyCount, families);

    /**Iterate through Found Families*/
    for(unsigned int i = 0; i < familyCount; i++) {
        VkQueueFamilyProperties family = families[i];
        /**If the Queue is of the Graphics Family Pass it's index*/
        if(family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            foundFamilies.graphicsFamily = i;
            foundFamilies.familiesFound |= VK_QUEUE_GRAPHICS_BIT;
        }

        /**Check if Queue Has Present Support*/
        VkBool32 presentSupport = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(dev, i, surface, &presentSupport);
        if(presentSupport) {
            foundFamilies.presentFamily = i;
            foundFamilies.familiesFound |= VK_QUEUE_PRESENT_BIT;
        }

    }

    return foundFamilies;
}

bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
    /**Get Number of available Extensions*/
    unsigned int exCount;
    vkEnumerateDeviceExtensionProperties(device,NULL, &exCount, NULL);

    /**Populate Properties into Array*/
    VkExtensionProperties availableExtensions[exCount];
    vkEnumerateDeviceExtensionProperties(device,NULL, &exCount, availableExtensions);

    /**Check if Device Supports all Required Extensions*/
    for(uint i = 0; i < extensionCount; i++) {
        bool devFound = FALSE;
        for(uint j = 0; j < exCount; j++) {
            if(!strcmp(requiredExtensions[i], availableExtensions[j].extensionName)) {
                devFound = TRUE;
                break;
            }
        }
        if(!devFound) return FALSE;
    }
    return TRUE;
}

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice dev) {

    /**Get Capabilities of Device + Surface*/
    SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(dev, surface, &details.capabilities);

    /**Get Number of Surface Formats*/
    uint formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(dev, surface, &formatCount, 0);

    /**If there are no formats available, exit*/
    if(formatCount) {
        details.formats = malloc(sizeof(VkSurfaceFormatKHR) * formatCount);
        details.formatCount = formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(dev, surface, &formatCount, details.formats);
    } else {
        fprintf(stderr, "No Surface Formats Available\n");
        exit(EXIT_FAILURE);
    }

    /**Get Number of Presentation Modes*/
    uint presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(dev, surface, &presentModeCount, 0);

    /**If there are no presentation modes available, exit*/
    if(presentModeCount) {
        details.presentModes = malloc(sizeof(VkPresentModeKHR) * presentModeCount);
        details.modeCount = presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(dev, surface, &presentModeCount, details.presentModes);
    } else {
        fprintf(stderr, "No Surface Formats Available\n");
        exit(EXIT_FAILURE);
    }

    return details;
}

void createLogicalDevice() {

    /**Get Available Families For the Device*/
    QueueFamilies families = findQueueFamilies(pDevice);

    printf("Families Found: %d\n", families.familiesFound);
    /**If Desired Families are not present error*/
    if(!(families.familiesFound & VK_QUEUE_PRESENT_BIT & VK_QUEUE_GRAPHICS_BIT)) {
        //TODO: Create a dedicated Queue Family Checker
        fprintf(stderr,"Desired Queue Family Could Not be Found\n");
        exit(1);
    }

    //TODO: Construct Array of QueueCreateInfo's based on Unique Indices
    uint requestedFamilies[] = {families.graphicsFamily, families.presentFamily};
    bool uniqueQues[families.familiesFound]; /**Bool Map of Indices that are desired*/ //TODO: Convert to Bitwise Array

    uint* selectedFamilies = malloc(sizeof(uint) * families.familiesFound); /**Array of the Families That will Actually Be Used*/
    memset(uniqueQues, FALSE, sizeof(uniqueQues));
    uint uniqueCount = 0; /**Number of Unique Queues that will be used*/
    for(uint i = 0; i < sizeof(requestedFamilies)/sizeof(uint); i++) {
        /**Check Which Families Are used*/
        if(!uniqueQues[requestedFamilies[i]]) {
            uniqueQues[requestedFamilies[i]] = TRUE;
            selectedFamilies[uniqueCount] = i;
            uniqueCount++;
        }
        if(uniqueCount > families.familiesFound) {
            fprintf(stderr, "More Unique Indices Found than Families\n");
            exit(EXIT_FAILURE);
        } /**Error Checking*/
    }
    if(!uniqueCount) {
        fprintf(stderr,"No Indices Found (What?)\n");
        exit(EXIT_FAILURE);

    }
    VkDeviceQueueCreateInfo qCreateInfos[uniqueCount]  ;
    for(uint i = 0; i < uniqueCount; i++) {
        qCreateInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        qCreateInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        qCreateInfos[i].pNext = 0;
        qCreateInfos[i].flags = 0;
        qCreateInfos[i].queueFamilyIndex = selectedFamilies[i];
        qCreateInfos[i].queueCount = 1;
        float queuePriority = 1;
        qCreateInfos[i].pQueuePriorities = &queuePriority;

    }


    VkPhysicalDeviceFeatures deviceFeatures;
    memset(&deviceFeatures, VK_FALSE, sizeof(deviceFeatures));
    deviceFeatures.geometryShader = VK_TRUE;
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    /**Create Device*/
    VkDeviceCreateInfo  createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = qCreateInfos;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = extensionCount;
    createInfo.ppEnabledExtensionNames = requiredExtensions;
    createInfo.pNext = 0;
    createInfo.flags = 0;
#ifndef NDEBUG /**If in Debug Mode Specify Layers*/
    createInfo.enabledLayerCount = vLayerCount;
    createInfo.ppEnabledLayerNames = validationLayers;
#else /**If not Layer Count is 0*/
    createInfo.enabledLayerCount = 0;
#endif
    if((vkCreateDevice(pDevice, &createInfo, 0, &lDevice)) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create logical device\n");
        exit(EXIT_FAILURE);
    }

    vkGetDeviceQueue(lDevice, families.graphicsFamily, 0, &graphicsQueue);
    vkGetDeviceQueue(lDevice, families.presentFamily, 0, &presentQueue);
    printf("Created Logical Device\n");
}
/********************
 * Render Functions *
 ********************/

void createRenderPass() {
    /**Initialize Color Buffer*/
    VkAttachmentDescription  colorAtt;
    colorAtt.format = swapChainImageFormat;
    colorAtt.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAtt.flags = 0;

    /**Define how attachment uData is handled*/
    colorAtt.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAtt.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAtt.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAtt.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAtt.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAtt.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    /**Define Subpass*/
    VkAttachmentReference caRef;
    caRef.attachment = 0;
    caRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass;
    subpass.flags = 0;
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &caRef;
    subpass.pInputAttachments = 0;
    subpass.inputAttachmentCount = 0;
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = 0;
    subpass.pResolveAttachments = 0;
    subpass.pDepthStencilAttachment = 0;
    VkRenderPassCreateInfo renderPassInfo;
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.flags = 0;
    renderPassInfo.pNext = 0;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAtt;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.pDependencies = 0;
    renderPassInfo.dependencyCount = 0;
    VkDevice dev = lDevice;

    if(vkCreateRenderPass(dev, &renderPassInfo, 0, &renderPass) != VK_SUCCESS) {
        fprintf(stderr, "Failed To Create Render Pass\n");
        exit(EXIT_FAILURE);
    }
    printf("Created Render }ass\n");
}

void createGraphicsPipeline() {
    VkShaderModule vShader = createShaderModule("vert.spv");
    VkShaderModule fShader = createShaderModule("frag.spv");

    /**Create Pipeline Object*/
    VkPipelineShaderStageCreateInfo vertShaderStageInfo;
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.pNext = 0;
    vertShaderStageInfo.flags = 0;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vShader;
    vertShaderStageInfo.pName = "main";
    vertShaderStageInfo.pSpecializationInfo = 0;

    VkPipelineShaderStageCreateInfo  fragShaderStageInfo;
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.pNext = 0;
    fragShaderStageInfo.flags = 0;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fShader;
    fragShaderStageInfo.pName = "main";
    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    /**Create Format of Vertex Data*/
    VkPipelineVertexInputStateCreateInfo vInputInfo;
    vInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vInputInfo.pNext = 0;
    vInputInfo.flags = 0;


    /**Set Vertex Binding Description*/
    vInputInfo.vertexBindingDescriptionCount = 1;
    VkVertexInputBindingDescription vertexBind;
    vertexBind.binding = 0;
    vertexBind.stride = sizeof(vertexArrayObject);
    vertexBind.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    vInputInfo.pVertexBindingDescriptions = &vertexBind;

    /**Set Vertex Attributes*/
    vInputInfo.vertexAttributeDescriptionCount = 4;
    VkVertexInputAttributeDescription attDesc[4];

    /**Define Position Data*/
    attDesc[0].binding = 0;
    attDesc[0].location = 0;
    attDesc[0].format = VK_FORMAT_R16G16B16_SINT;
    attDesc[0].offset = 0;

    /**Define Color Data*/
    attDesc[1].binding = 0;
    attDesc[1].location = 1;
    attDesc[1].format = VK_FORMAT_R8G8B8A8_UINT;
    attDesc[1].offset = offsetof(vertexArrayObject, color);

    /**Define Texture Data*/
    attDesc[2].binding = 0;
    attDesc[2].location = 2;
    attDesc[2].format = VK_FORMAT_R32G32_SFLOAT;
    attDesc[2].offset = offsetof(vertexArrayObject, texture);

    attDesc[3].binding = 0;
    attDesc[3].location = 3;
    attDesc[3].format = VK_FORMAT_R16_UINT;
    attDesc[3].offset = offsetof(vertexArrayObject, index);
    vInputInfo.pVertexAttributeDescriptions = attDesc;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly;
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.pNext = 0;
    inputAssembly.flags = 0;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    /**Describe region of the framebuffer that the output will be rendered to*/
    VkViewport viewport;
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = (float) swapChainExtent.width;
    viewport.height = (float) swapChainExtent.height;
    viewport.minDepth = 0;
    viewport.maxDepth = 1;

    /**Define regions the pixels will be stored*/
    VkRect2D  scissor;
    scissor.offset.x = scissor.offset.y = 0;
    scissor.extent = swapChainExtent;

    /**Combine Viewport and Rectangle into a viewport*/
    VkPipelineViewportStateCreateInfo  vpState;
    vpState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vpState.flags = 0;
    vpState.pNext = 0;
    vpState.viewportCount = vpState.scissorCount = 1;
    vpState.pViewports = & viewport;
    vpState.scissorCount = 1;
    vpState.pScissors = &scissor;

    /**Initialize Rasterizer*/
    VkPipelineRasterizationStateCreateInfo rasterizer;
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.pNext = 0;
    rasterizer.flags = 0;
    rasterizer.depthClampEnable = VK_FALSE; /**Do not Clamp Fragments*/
    rasterizer.rasterizerDiscardEnable = VK_FALSE; /**Do Not Disable Rasterization*/
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL; /**Fill Polygons with desired Images*/
    rasterizer.lineWidth = 1;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0;
    rasterizer.depthBiasSlopeFactor = 0;

    /**Configure Multisampling*/
    VkPipelineMultisampleStateCreateInfo mSample;
    mSample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    mSample.flags = 0;
    mSample.pNext = 0;
    mSample.sampleShadingEnable = VK_FALSE;
    mSample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    mSample.minSampleShading = 1;
    mSample.pSampleMask = 0;
    mSample.alphaToCoverageEnable = VK_FALSE;
    mSample.alphaToOneEnable = 0;

    /**Set Color Blending Configs*/
    VkPipelineColorBlendAttachmentState colBlend;
    colBlend.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colBlend.blendEnable = VK_TRUE;
    colBlend.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colBlend.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colBlend.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colBlend.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colBlend.colorBlendOp = colBlend.alphaBlendOp  = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo  blendConsts;
    blendConsts.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    blendConsts.flags = 0;
    blendConsts.pNext = 0;
    blendConsts.logicOpEnable = VK_FALSE;
    blendConsts.attachmentCount = 1;
    blendConsts.pAttachments = &colBlend;
    memset(blendConsts.blendConstants, 0, 4 * sizeof(float));

    /**Initialize Pipeline Layout Creation Object*/
    VkPipelineLayoutCreateInfo plInfo;
    plInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    plInfo.flags = 0;
    plInfo.pNext = 0;
    plInfo.setLayoutCount = 1;
    plInfo.pSetLayouts = &descriptorSetLayout;
    plInfo.pushConstantRangeCount = 0;
    plInfo.pPushConstantRanges = 0;
    VkResult res;
    if((res = vkCreatePipelineLayout(lDevice, &plInfo,0,&pipelineLayout)) != VK_SUCCESS) {
        fprintf(stderr, "Failed to Create Pipeline: %d\n", res);
        exit(EXIT_FAILURE);
    }

    /**Consolidate Pipeline*/
    VkGraphicsPipelineCreateInfo pipeline;
    pipeline.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline.pNext = 0;
    pipeline.flags = 0;
    pipeline.stageCount = 2;
    pipeline.pStages = shaderStages;
    pipeline.pVertexInputState = &vInputInfo;
    pipeline.pInputAssemblyState = &inputAssembly;
    pipeline.pTessellationState = 0;
    pipeline.pViewportState = &vpState;
    pipeline.pRasterizationState = &rasterizer;
    pipeline.pMultisampleState = &mSample;
    pipeline.pDepthStencilState = 0;
    pipeline.pColorBlendState = &blendConsts;
    pipeline.pDynamicState = 0;
    pipeline.layout = pipelineLayout;
    pipeline.renderPass = renderPass;
    pipeline.subpass = 0;
    pipeline.basePipelineHandle = VK_NULL_HANDLE;
    pipeline.basePipelineIndex = -1;

    printf("Creating Graphics Pipeleine\n");
    if(vkCreateGraphicsPipelines(lDevice, 0, 1, &pipeline, 0, &gPipeline) != VK_SUCCESS) {
        fprintf(stderr, "Failed to Create Graphics Pipeline\n");
        exit(1);
    }
    vkDestroyShaderModule(lDevice, vShader, 0);
    vkDestroyShaderModule(lDevice, fShader, 0);
}

VkCommandBuffer beginSingleTimeCommands() {
    VkCommandBufferAllocateInfo  allocInfo;
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.pNext = 0;

    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer comBuffer;
    vkAllocateCommandBuffers(lDevice, &allocInfo, &comBuffer);

    VkCommandBufferBeginInfo beginInfo;
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pNext = 0;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pInheritanceInfo = 0;

    vkBeginCommandBuffer(comBuffer, &beginInfo);

    return comBuffer;
}

void endSingleTimeCommands(VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo;
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = 0;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    submitInfo.waitSemaphoreCount = 0;
    submitInfo.pWaitSemaphores = 0;
    submitInfo.waitSemaphoreCount = 0;
    submitInfo.pWaitDstStageMask = 0;
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.pSignalSemaphores = 0;


    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);

    vkFreeCommandBuffers(lDevice, commandPool, 1, & commandBuffer);

}

void createFrameBuffers() {
    /**Set the size of the FrameBuffer Array*/swapChainFrameBuffers = malloc(sizeof(VkFramebuffer) * swapChainSize);
    VkImageView* imgViews = swapChainImageViews;
    for(unsigned int i = 0; i < swapChainSize; i++) {
        VkImageView attachments[] = {imgViews[i]};
        VkFramebufferCreateInfo framebufferInfo;
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.flags = 0;
        framebufferInfo.pNext = 0;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        if(vkCreateFramebuffer(lDevice, &framebufferInfo, 0, &swapChainFrameBuffers[i])) {
            fprintf(stderr, "Failed to create Frame Buffer\n");
            exit(EXIT_FAILURE);
        }

    }
}

VkShaderModule createShaderModule(const char* filename) {
    FILE* file = fopen(filename, "ab+");

    if(!file) {
        fprintf(stderr, "Could Not Open File\n");
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    rewind(file);
    const char* buffer = malloc(fileSize+1);
    fread((void *) buffer, fileSize, 1, file);

    VkShaderModuleCreateInfo  createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.flags = 0;
    createInfo.pNext = 0;
    createInfo.codeSize = fileSize;
    createInfo.pCode = (const uint32_t *) buffer;
    VkShaderModule shaderModule;

    if(vkCreateShaderModule(lDevice, &createInfo, 0, &shaderModule)) {
        fprintf(stderr, "Failed to Create Shader Module\n");
        exit(1);
    }
    return shaderModule;
}

void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* bufferMemory) {
    VkBufferCreateInfo bufferInfo;
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = 0;
    bufferInfo.flags = 0;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.queueFamilyIndexCount = 0;
    bufferInfo.pQueueFamilyIndices = 0;

    VkResult res;
    if ((res = vkCreateBuffer(lDevice, &bufferInfo, 0, buffer)) != VK_SUCCESS) {
        fprintf(stderr, "failed to create buffer!");
        exit(1);
    }

    VkMemoryRequirements memRequirements;

    vkGetBufferMemoryRequirements(lDevice, *buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo;
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.pNext = 0;

    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(lDevice, &allocInfo, 0, bufferMemory) != VK_SUCCESS) {
        fprintf(stderr,"Could Not Allocate Memory for Vertex Buffer\n");
        exit(1);
    }

    vkBindBufferMemory(lDevice, *buffer, *bufferMemory, 0);


}

void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();
    VkBufferCopy copyRegion;
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
    endSingleTimeCommands(commandBuffer);
}


void createUniformBuffers() {
    /**Uniform Buffer for View Transformation*/
    VkDeviceSize bufferSize = sizeof(uniformBufferObject);

    uniformBuffers = malloc(swapChainSize * sizeof(VkBuffer));
    uniformBuffersMemory = malloc(swapChainSize * sizeof (VkDeviceMemory));

    for(uint i = 0; i < swapChainSize; i++) {
        createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &uniformBuffers[i], &uniformBuffersMemory[i]);
        //vkMapMemory(lDevice, uniformBuffersMemory[i], 0, sizeof(uniformBufferObject), 0, (void*) &uData);
    }

}

void createDescriptorPool() {
    VkDescriptorPoolSize poolSizes[3];
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = swapChainSize;

    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = swapChainSize;

    poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[2].descriptorCount = swapChainSize;
    VkDescriptorPoolCreateInfo poolInfo;
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.pNext = 0;
    poolInfo.flags = 0;
    poolInfo.poolSizeCount = 3;
    poolInfo.pPoolSizes = poolSizes;
    poolInfo.maxSets = swapChainSize;

    if(vkCreateDescriptorPool(lDevice, &poolInfo, 0, & descriptorPool)) {
        fprintf(stderr, "Failed to create Descriptor Pool\n");
        exit(1);
    }
}

void createDescriptorSets() {
    VkDescriptorSetLayout layouts[swapChainSize];
    for(uint i = 0; i < swapChainSize; i++) layouts[i] = descriptorSetLayout;

    VkDescriptorSetAllocateInfo  allocInfo;
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.pNext = 0;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = swapChainSize;
    allocInfo.pSetLayouts = layouts;

    descriptorSets = malloc(sizeof(VkDescriptorSet) * swapChainSize);
    VkResult res;
    if((res = vkAllocateDescriptorSets(lDevice, &allocInfo, descriptorSets)) != VK_SUCCESS ) {
        fprintf(stderr,"Failed to allocate descriptor sets: %d\n",res);
        exit(1);
    }

    for(uint i = 0; i < swapChainSize; i++) {

        VkDescriptorBufferInfo  bufferInfo;
        bufferInfo.buffer = uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = VK_WHOLE_SIZE; //sizeof(uniformBufferObject);



        VkWriteDescriptorSet descriptorWrites[2];

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].pNext =  0;
        descriptorWrites[0].dstSet = descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;
        descriptorWrites[0].pImageInfo = 0;
        descriptorWrites[0].pTexelBufferView = 0;
        descriptorWrites[0].dstArrayElement = 0;


        VkDescriptorImageInfo imageInfo[2];
        for(int j =0; j < 2; j++) {
            imageInfo[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo[j].imageView = textureImageView[j];
            imageInfo[j].sampler = textureSampler;
        }


        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].pNext = 0;
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstSet = descriptorSets[i];
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 2;
        descriptorWrites[1].pImageInfo = imageInfo;
        descriptorWrites[1].pBufferInfo = 0;
        descriptorWrites[1].pTexelBufferView = 0;
        descriptorWrites[1].dstArrayElement = 0;

        vkUpdateDescriptorSets(lDevice, 2, descriptorWrites,0,0);

    }

}

void initUniformBuffers() {
    for(uint i = 0; i < swapChainSize; i++)
    vkMapMemory(lDevice, uniformBuffersMemory[i], 0, sizeof(uniformBufferObject), 0, (void*) &uData);
}

float pos = 0;
float angle = 0;

bool* imgMaps;
void updateUniformBuffer(uint currentImage) {

    vkMapMemory(lDevice, uniformBuffersMemory[currentImage], 0, sizeof(uniformBufferObject), 0, (void*) &uData);
    memcpy(uData, &uniformData, sizeof(uniformBufferObject));

    uniformBufferObject * uPT = uData;

    memset(uData->model, 0, sizeof(uData->model));


    uData->model[2][2] = uData->model[3][3] = 1;
    uData->model[0][0] = cosf(angle);
    uData->model[0][1] = -sinf(angle);
    uData->model[1][1] = cosf(angle);
    uData->model[1][0] = sinf(angle);

    //uData->model[3][0] = pos;
    //angle+= 0.05f;
    uData->texH[0] = 1024;

    uData->texW[0] = 2048;
    uData->texW[1] = 2048;
    //for(int i = 0; i < 32; i++) uData->texW[i] = 2048;
    uniformBufferObject* uDRef = uData;
    vkUnmapMemory(lDevice,uniformBuffersMemory[currentImage]);
}



void createVertexBuffer() {
#ifdef NOSTAGE
    createBuffer(sizeof(vertices), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &vertexBuffer, &vertexBufferMemory);
#else
    VkDeviceSize bufferSize = sizeof(vertices);
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);
    vkMapMemory(lDevice, stagingBufferMemory, 0, bufferSize, 0, (void**) &vData);
    memcpy(vData, vertices, bufferSize);
    vkUnmapMemory(lDevice, stagingBufferMemory);
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &vertexBuffer, &vertexBufferMemory);
    copyBuffer(stagingBuffer, vertexBuffer, bufferSize);
#endif
}




void createIndexBuffer() {
#ifdef NOSTAGE
    createBuffer(sizeof(vertices),VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &indexBuffer, &indexBufferMemory);
#else
    VkDeviceSize bufferSize = sizeof(indices);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &iStagingBuffer, &iStagingBufferMemory);

    void* data;
    vkMapMemory(lDevice, iStagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices, bufferSize);
    vkUnmapMemory(lDevice, iStagingBufferMemory);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &indexBuffer, &indexBufferMemory);

    copyBuffer(iStagingBuffer, indexBuffer, bufferSize);

    vkDestroyBuffer(lDevice, iStagingBuffer, 0);
    vkFreeMemory(lDevice, iStagingBufferMemory, 0);
#endif
}



void createImage(uint width, uint height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage* image, VkDeviceMemory* imageMemory) {

    VkImageCreateInfo  imageInfo;
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.pNext = 0;
    imageInfo.flags = 0;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

    if(vkCreateImage(lDevice, &imageInfo, 0, image) != VK_SUCCESS) {
        fprintf(stderr,"Failed to Create Image\n");
        exit(1);
    }

    VkMemoryRequirements memRequirements;

    vkGetImageMemoryRequirements(lDevice, *image, &memRequirements);

    VkMemoryAllocateInfo  allocInfo;
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.pNext = 0;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);
    if(vkAllocateMemory(lDevice, &allocInfo, 0, imageMemory) != VK_SUCCESS) {
        fprintf(stderr, "Failed to Allocate Memory\n");
        exit(1);
    }

    vkBindImageMemory(lDevice, *image, *imageMemory, 0);
}

void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();
    VkImageMemoryBarrier  barrier;
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;


    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destStage;
    if(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if(oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        fprintf(stderr, "Unsupported Layout Transition\n");
        exit(1);
    }



    vkCmdPipelineBarrier(commandBuffer,sourceStage ,destStage,0,0,0,0,0,1, &barrier);

    endSingleTimeCommands(commandBuffer);
}



void copyBufferToImage(VkBuffer buffer, VkImage image, uint width, uint height) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();
    VkBufferImageCopy region;
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = (VkOffset3D) {0, 0, 0};
    region.imageExtent = (VkExtent3D) {width, height, 1};

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);



    endSingleTimeCommands(commandBuffer);
}

void combineImgsWid(unsigned char** finalImg, unsigned char* img1, unsigned char* img2, unsigned int width, unsigned int height) {


    *finalImg = malloc(2 * width * height * 4);
    //*finalImg = malloc(2 * width * height * 4);
    for(uint i = 0; i < height; i++) {
        memcpy((*finalImg) + 2 * width * 4 * i, img1 + width * 4 * i, width * 4 );
        memcpy((*finalImg) + (2 * width * 4 * i) + width *4, img2 + width * 4 * i, width * 4 );
    }

}

void combineImgsHeight(unsigned char** finalImg, unsigned char* img1, unsigned char* img2, unsigned int width, unsigned int height) {

    //TODO: Implement
    *finalImg = malloc(2 * width * height * 4);
    (void) img1;
    (void) img2;


}

void createTextureImage(int tIndex, const char* file) {
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(pDevice, &properties);

    unsigned char* img = 0;
    unsigned char* img2 = 0;
    unsigned int width, height, width2, height2;
    unsigned error = lodepng_decode32_file(&img, &width, &height, "Tic2.png");
    unsigned error2 = lodepng_decode32_file(&img2, &width2, &height2, file);
    unsigned char* img3 = 0;
    combineImgsWid(&img3,  img, img2, width, height);

    width *= 2;
    VkDeviceSize imageSize = width * height * 4;

    if (error) {
        fprintf(stderr,"error %u: %s\n", error, lodepng_error_text(error)); exit(1);
    }

    VkBuffer tStagingBuffer;
    VkDeviceMemory tStagingBufferMemory;
    //uniformData.texW[tIndex] = width;
    //uniformData.texH[tIndex] = height;
    createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &tStagingBuffer, &tStagingBufferMemory);

    void* data;
    vkMapMemory(lDevice, tStagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, img3, imageSize);
    vkUnmapMemory(lDevice, tStagingBufferMemory);

    free(img);
    free(img2);
    free(img3);

    texW[tIndex] = width;
    texH[tIndex] = height;
    VkImageCreateInfo  imageInfo;
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.pNext = 0;
    imageInfo.flags = 0;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

    if(vkCreateImage(lDevice, &imageInfo, 0, &textureImage[tIndex]) != VK_SUCCESS) {
        fprintf(stderr,"Failed to Create Image\n");
        exit(1);
    }

    VkMemoryRequirements memRequirements;

    vkGetImageMemoryRequirements(lDevice, textureImage[tIndex], &memRequirements);

    VkMemoryAllocateInfo  allocInfo;
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.pNext = 0;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if(vkAllocateMemory(lDevice, &allocInfo, 0, &textureImageMemory[tIndex]) != VK_SUCCESS) {
        fprintf(stderr, "Failed to Allocate Memory\n");
        exit(1);
    }

    vkBindImageMemory(lDevice, textureImage[tIndex], textureImageMemory[tIndex], 0);


    transitionImageLayout(textureImage[tIndex], VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToImage(tStagingBuffer, textureImage[tIndex], width, height);
    transitionImageLayout(textureImage[tIndex], VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(lDevice, tStagingBuffer, 0);
    vkFreeMemory(lDevice, tStagingBufferMemory, 0);
}
VkImageView createImageView(VkImage image, VkFormat format) {
    VkImageViewCreateInfo viewInfo;
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.pNext = 0;
    viewInfo.flags = 0;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.components = (VkComponentMapping) {VK_COMPONENT_SWIZZLE_IDENTITY,
                                                VK_COMPONENT_SWIZZLE_IDENTITY,
                                                VK_COMPONENT_SWIZZLE_IDENTITY,
                                                VK_COMPONENT_SWIZZLE_IDENTITY};

    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if(vkCreateImageView(lDevice, &viewInfo, 0, &imageView)) {
        fprintf(stderr,"Failed to create texture image view\n");
        exit(1);
    }
    return imageView;
}
void swapTextureImage(int tIndex, const char* file) {
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(pDevice, &properties);

    unsigned char* img = 0;
    unsigned char* img2 = 0;
    unsigned int width, height, width2, height2;
    unsigned error = lodepng_decode32_file(&img, &width, &height, "Tic.png");
    unsigned error2 = lodepng_decode32_file(&img2, &width2, &height2, file);
    unsigned char* img3 = 0;
    combineImgsWid(&img3,  img, img2, width, height);

    width *= 2;
    VkDeviceSize imageSize = width * height * 4;

    if (error) {
        fprintf(stderr,"error %u: %s\n", error, lodepng_error_text(error)); exit(1);
    }

    VkBuffer tStagingBuffer;
    VkDeviceMemory tStagingBufferMemory;
    createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &tStagingBuffer, &tStagingBufferMemory);

    void* data;
    vkMapMemory(lDevice, tStagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, img3, imageSize);
    vkUnmapMemory(lDevice, tStagingBufferMemory);

    free(img);
    free(img2);
    free(img3);

    texW[tIndex] = width;
    texH[tIndex] = height;

    transitionImageLayout(textureImage[tIndex], VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToImage(tStagingBuffer, textureImage[tIndex], width, height);
    transitionImageLayout(textureImage[tIndex], VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(lDevice, tStagingBuffer, 0);
    vkFreeMemory(lDevice, tStagingBufferMemory, 0);

    //textureImageView[tIndex] = createImageView(textureImage[tIndex], VK_FORMAT_R8G8B8A8_UNORM);
}



void createTextureImageView() {
    for(uint i = 0; i < texCount; i++) textureImageView[i] = createImageView(textureImage[i], VK_FORMAT_R8G8B8A8_UNORM);


}

void createTextureSampler() {
    VkSamplerCreateInfo samplerInfo;
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.pNext = 0;
    samplerInfo.flags = 0;
    /**Determine how texels are interpolated*/
    samplerInfo.magFilter = samplerInfo.minFilter = VK_FILTER_NEAREST;

    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    samplerInfo.addressModeU =
        samplerInfo.addressModeV =
            samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    samplerInfo.anisotropyEnable = VK_TRUE;
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(pDevice, &properties);

    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

    samplerInfo.minLod = samplerInfo.maxLod = samplerInfo.mipLodBias = 0;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;

    if(vkCreateSampler(lDevice, &samplerInfo, 0, &textureSampler) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create texture Sampler\n");
        exit(1);
    }
}


uint findMemoryType(uint typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(pDevice, &memoryProperties);

    for(uint i = 0; i < memoryProperties.memoryTypeCount; i++) if((typeFilter & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) return i;

    fprintf(stderr,"Could Not Find Suitable Memory Type\n");
    exit(1);
}



/************************
 * Swap Chain Functions *
 ************************/

void createSwapChain() {

    /**Get Swap Chain Info from the Physical Device*/
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(pDevice);

    /**Get the Optimal Swap Chain Surface Format*/
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats, swapChainSupport.formatCount);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes, swapChainSupport.modeCount);
    VkExtent2D extent = chooseSwapExtent(&swapChainSupport.capabilities);

    /**Choose Number of images*/
    uint imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if(swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageArrayLayers) imageCount = swapChainSupport.capabilities.maxImageCount;

    /**Initialize Swap Chain Object*/
    VkSwapchainCreateInfoKHR createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.pNext = 0;
    createInfo.flags = 0;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers =  1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilies families = findQueueFamilies(pDevice);

    /**If the family for Graphics and Presenting differ, use sharing mode*/
    if(families.graphicsFamily != families.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        uint indices[2] = {families.graphicsFamily, families.presentFamily};
        createInfo.pQueueFamilyIndices = indices;
    } else { /**If Not use exclusive mode*/
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = NULL;
    }

    /**No Transformation Required*/
    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE; /**Clip Obscured Pixels*/
    createInfo.oldSwapchain = 0; /**Swapchain Will Remain the same for now*/

    if(vkCreateSwapchainKHR(lDevice, &createInfo, 0, &swapChain) != VK_SUCCESS) {
        fprintf(stderr,"Could Not Create Swap Chain!\n");
        exit(EXIT_FAILURE);
    }

    /**Get number of images*/
    vkGetSwapchainImagesKHR(lDevice, swapChain, &imageCount, 0);
    swapChainImages = malloc(sizeof(VkImage) * imageCount);
    /**Get array of images*/
    vkGetSwapchainImagesKHR(lDevice, swapChain, &imageCount, swapChainImages);
    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
    swapChainSize = imageCount;
}

VkSurfaceFormatKHR chooseSwapSurfaceFormat(const VkSurfaceFormatKHR* availableFormats, unsigned int formatCount) {
    for(uint i = 0; i < formatCount; i++) {
        VkSurfaceFormatKHR format = availableFormats[i];
        /**Choose format with RGBA pixels and supports SRGB Color Space*/
        if(format.format == VK_FORMAT_B8G8R8A8_SSCALED && availableFormats->colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) return format;
    }
    return availableFormats[0];
}

VkPresentModeKHR chooseSwapPresentMode(const VkPresentModeKHR* availableModes, unsigned int modeCount) {
    VkPresentModeKHR mode;
    /**Return Mailbox Mode if Available*/
    for(uint i = 0; i < modeCount; i++) if((mode = availableModes[i]) == VK_PRESENT_MODE_MAILBOX_KHR) return mode;

    /**If not, return the FIFO Mode (Guaranteed)*/
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR* capabilities) {
    /**Return current extent if it cannot reach max*/
    if (capabilities->currentExtent.width != UINT32_MAX) return capabilities->currentExtent;

    else { /**In the other case, use the size of the window*/
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        VkExtent2D actualExtent = {(uint) width, (uint) height};

        /**Lock extent to given bounds*/
        actualExtent.width = actualExtent.width > capabilities->minImageExtent.width ? actualExtent.width : capabilities->minImageExtent.width;
        actualExtent.width = actualExtent.width < capabilities->maxImageExtent.width ? actualExtent.width : capabilities->maxImageExtent.width;
        return actualExtent;
    }
}

void createImageViews() {
    /**Allocate an Image View for */
    swapChainImageViews = malloc(sizeof(VkImageView) * swapChainSize);

    for(uint i = 0; i < swapChainSize; i++)
        swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat);

}

void reCreateSwapChain() {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    while(width == 0 || height == 0) {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }
    /**Wait for Resources to free up*/
    vkDeviceWaitIdle(lDevice);
    printf("Devices Freed\n");

    /**Clean Objects*/
    cleanSwapChain();

    createSwapChain();
    createImageViews();
    createRenderPass();
    createGraphicsPipeline();
    createFrameBuffers();
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
    createCommandBuffers();

    //imagesInFlight =
}

void cleanSwapChain() {
    printf ("__FUNCTION__ = %s\n", __FUNCTION__);
    /**Remove Frame Buffers*/
    for(uint i = 0; i < swapChainSize; i++) vkDestroyFramebuffer(lDevice, swapChainFrameBuffers[i], 0);
    printf("CSW: Frame Destroyed\n");
    vkFreeCommandBuffers(lDevice, commandPool, swapChainSize, commandBuffers);
    printf("COmmand Buffers Freed\n");
    vkDestroyPipeline(lDevice, gPipeline, 0);
    printf("Pipeline Destroyed\n");
    vkDestroyPipelineLayout(lDevice, pipelineLayout, 0);
    printf("Pipeline Layout Destroyed\n");
    vkDestroyRenderPass(lDevice, renderPass, 0);

    for(uint i = 0; i < swapChainSize; i++) vkDestroyImageView(lDevice, swapChainImageViews[i], 0);

    /*for(uint i = 0; i < swapChainSize; i++) {
        vkDestroyBuffer(lDevice, uniformBuffers[i], 0);
        vkFreeMemory(lDevice, uniformBuffersMemory[i], 0);
    }*/
    printf("Buffers Dest");
    printf("Destroying Chain\n");
    vkDestroySwapchainKHR(lDevice, swapChain, 0);
    for(uint i = 0; i < swapChainSize; i++) {
        vkDestroyBuffer(lDevice, uniformBuffers[i], 0);
        vkFreeMemory(lDevice, uniformBuffersMemory[i], 0);
    }
    vkDestroyDescriptorPool(lDevice, descriptorPool, 0);
    //for(uint i = 0; i < swapChainSize; i++)
}
/*********************
 * Command Functions *
 *********************/


void createCommandPool() {
    QueueFamilies  queueFamilies = findQueueFamilies(pDevice);

    VkCommandPoolCreateInfo poolInfo;
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = 0;
    poolInfo.pNext = 0;
    poolInfo.queueFamilyIndex = queueFamilies.graphicsFamily;

    if(vkCreateCommandPool(lDevice, &poolInfo, 0, &commandPool)) {
        fprintf(stderr, "Failed to Create Command Pool\n");
        exit(1);
    }
}

void createCommandBuffers() {
    /**Allocate Space for Command Buffer*/
    commandBuffers = malloc(swapChainSize * sizeof(VkCommandBuffer));
    VkCommandBufferAllocateInfo allocInfo;
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.pNext = 0;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = swapChainSize;

    if(vkAllocateCommandBuffers(lDevice, &allocInfo, commandBuffers)) {
        fprintf(stderr, "Could not Allocate Command Buffers\n");
        exit(0);
    }

    for(uint i = 0; i < swapChainSize; i ++) {
        VkCommandBufferBeginInfo beginInfo;
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0;
        beginInfo.pNext = 0;
        beginInfo.pInheritanceInfo = 0;

        if(vkBeginCommandBuffer(commandBuffers[i], &beginInfo)) {
            fprintf(stderr,"Could Not Record Command Buffer");
            exit(EXIT_FAILURE);
        }

        VkRenderPassBeginInfo  renderPassInfo;
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.pNext = 0;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = swapChainFrameBuffers[i];
        renderPassInfo.renderArea.offset.x = renderPassInfo.renderArea.offset.y = 0;
        renderPassInfo.renderArea.extent = swapChainExtent;
        VkClearValue clearColor =  {{{0.5f,0.5f,0.5f, 1}}};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;
        vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        /*vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, gPipeline);
        vkCmdDraw(commandBuffers[i],3,1,0,0);*/


        vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, gPipeline);
        VkBuffer vertexBuffers[] = {vertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT16);
        //vkCmdDraw(commandBuffers[i], sizeof(vertices), 1, 0,0);
        vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0,  1, &descriptorSets[i], 0, 0);
        vkCmdDrawIndexed(commandBuffers[i], sizeof(indices)/sizeof(INDEXTYPE),1,0,0,0);
        vkCmdEndRenderPass(commandBuffers[i]);
        if(vkEndCommandBuffer(commandBuffers[i])) {
            fprintf(stderr, "Failed to record Command Buffer\n");
        }


    }


}

void createSyncObjects() {
    imageAvailableSemaphores = malloc(2 * sizeof(VkSemaphore));
    renderFinishedSemaphores = malloc(2 * sizeof(VkSemaphore));
    inFlightFences = malloc(MAX_FRAMES_IN_FLIGHT * sizeof(VkFence));
    imagesInFlight = malloc(swapChainSize * sizeof(VkFence));
    memset(imagesInFlight, 0, sizeof(VkFence) * swapChainSize);
    VkSemaphoreCreateInfo semaphoreInfo;
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreInfo.pNext = 0;
    semaphoreInfo.flags = 0;

    VkFenceCreateInfo  fenceInfo;
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.pNext = 0;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if(vkCreateSemaphore(lDevice, &semaphoreInfo, 0, &imageAvailableSemaphores[i]) ||
           vkCreateSemaphore(lDevice, &semaphoreInfo, 0, &renderFinishedSemaphores[i]) ||
           vkCreateFence(lDevice, &fenceInfo, 0, &inFlightFences[i])) {
            fprintf(stderr, "Failed To Create Semaphores\n");
            exit(1);
        }
    }
}



/********************
 * Window Functions *
 ********************/

void createVulkanInstance() {
#ifndef NDEBUG
    if(!checkValidationLayerSupport()) { /**If No Validation Layer Support, then we can't run with Diagnostics*/
        fprintf(stderr, "Validation Layers Unavailable\n");
        exit(1);
    }
#endif
    /**Initialize Application Info*/
    VkApplicationInfo appInfo;
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext = 0; /**No Next*/
    appInfo.pApplicationName = "Hello Triangle"; /**Name of the Application*/
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0); /**Initial Build*/
    appInfo.pEngineName = "C-Kada Engine"; /**No Engine*/
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;


    /**Initialize Create Info*/
    VkInstanceCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pNext = 0; /**No other Instance (Should Be Explicitly Defined)*/
    createInfo.flags = 0; /**No Flags (Should Be Explicitly Defined)*/
    createInfo.pApplicationInfo = &appInfo;

    /**Get Default Extension Info*/
    unsigned int defaultGlfwExtensionCount = 0;
    const char** defaultglfwExtensions = glfwGetRequiredInstanceExtensions(&defaultGlfwExtensionCount);

    unsigned int glfwExtensionCount = defaultGlfwExtensionCount; /**Extension Count*/

#ifndef NDEBUG
    /**Include Debug Extension if Debugging*/
    glfwExtensionCount++;
#endif
    const char* glfwExtensions[glfwExtensionCount]; /**Array of Extension Names*/
    for(unsigned int i = 0; i < defaultGlfwExtensionCount; i++) glfwExtensions[i] = defaultglfwExtensions[i];
#ifndef NDEBUG
    /**Add Debug Extension if in Debug Mode*/
    glfwExtensions[defaultGlfwExtensionCount] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
#endif

    /**Define GLFW Extension Parameters*/
    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;


#ifndef NDEBUG
    /**Initialize Debug Info*/
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugCreateInfo.flags = 0;
    debugCreateInfo.pNext = 0;
    debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debugCreateInfo.pfnUserCallback = (PFN_vkDebugUtilsMessengerCallbackEXT) debugCallback;

    /**Apply Debug Info to Create Info*/
    createInfo.enabledLayerCount =  vLayerCount;
    createInfo.ppEnabledLayerNames = validationLayers;
    createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;

    /**Create the VK Instance*/
    if (vkCreateInstance(&createInfo, 0, &vInstance) != VK_SUCCESS) {
        fprintf(stderr,"Failed to create instance!");
        exit(1);
    }

    /**Create the Debug Instance*/
    VkResult res = CreateDebugUtilsMessengerEXT(vInstance, &debugCreateInfo, 0);
    if(res != VK_SUCCESS) {
        fprintf(stderr, "Could not Create Debug Utility\n");
        exit(EXIT_FAILURE);
    }
#endif

#ifdef NDEBUG/**If No Debug Mode, these values are null*/

    /**Apply Info to Create Info*/
    createInfo.enabledLayerCount = 0;
    createInfo.ppEnabledLayerNames = 0;
    createInfo.pNext = 0;

    /**Create the VK Instance*/
    if (vkCreateInstance(&createInfo, 0, &vInstance) != VK_SUCCESS) {
        fprintf(stderr,"Failed to create instance!");
        exit(1);
    }
#endif
    printf("Vulkan Instance Successfully Initialized\n");
}

void createDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding  uboLayoutBinding;
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;


    /**Reference Descriptor from Vertex Shader*/
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = 0;


    VkDescriptorSetLayoutBinding  samplerLayoutBinding;
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 2;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = 0;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding bindings[2] = {uboLayoutBinding, samplerLayoutBinding};
    VkDescriptorSetLayoutCreateInfo  layoutInfo;
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.pNext = 0;
    layoutInfo.flags = 0;
    layoutInfo.bindingCount = 2;
    layoutInfo.pBindings = bindings;


    if(vkCreateDescriptorSetLayout(lDevice, &layoutInfo, 0, &descriptorSetLayout) != VK_SUCCESS) {
        fprintf(stderr, "Could not Create Descriptor Set Layout\n");
        exit(1);
    }


}



void initWindow(int width, int height) {
    if(!glfwInit()) { /**Initialize GLFW Library Context*/
        fprintf(stderr,"GLFW Could Not Init\n");
        exit(1);
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); /**Set GLFW Window to be compatible with Vulkan*/
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); /**GLFW Window is not resizable*/
    window = glfwCreateWindow(width, height, "Vulkan Test", 0, 0); /**Initialize Window*/
    glfwSetFramebufferSizeCallback(window, resizeCallback);

}

static void resizeCallback() {
    resized = TRUE;
}
/*********************
 * General Functions *
 *********************/


void closeDisplay() {
#ifndef NDEBUG
    DestroyDebugUtilsMessengerEXT(vInstance, debugMessenger, 0);
#endif
    for(unsigned int i = 0; i < swapChainSize; i++) vkDestroyFramebuffer(lDevice, swapChainFrameBuffers[i], 0);

    vkDestroyPipeline(lDevice, gPipeline, 0);

    vkDestroyPipelineLayout(lDevice, pipelineLayout, 0);

    vkDestroyRenderPass(lDevice, renderPass, 0);
    vkDestroyCommandPool(lDevice, commandPool, 0);
    for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i ++) {
        vkDestroySemaphore(lDevice, renderFinishedSemaphores[i], 0);
        vkDestroySemaphore(lDevice, imageAvailableSemaphores[i], 0);
        vkDestroyFence(lDevice, inFlightFences[i], 0);
    }
    vkDestroySwapchainKHR(lDevice, swapChain, 0);
    for(uint i = 0; i < swapChainSize; i++) {
        vkDestroyBuffer(lDevice, uniformBuffers[i], 0);
        vkFreeMemory(lDevice, uniformBuffersMemory[i], 0);
    }
    vkDestroyDescriptorSetLayout(lDevice, descriptorSetLayout, 0);
    vkDestroyDescriptorPool(lDevice, descriptorPool, 0);

#ifndef NOSTAGE
    vkDestroyBuffer(lDevice, stagingBuffer, 0);

    vkFreeMemory(lDevice, stagingBufferMemory,0);
#endif
    vkDestroySampler(lDevice, textureSampler,0);
    vkDestroyImageView(lDevice, textureImageView[0], 0);
    vkDestroyImage(lDevice, textureImage[0], 0);
    vkFreeMemory(lDevice, textureImageMemory[0], 0);
    vkDestroyImageView(lDevice, textureImageView[1], 0);
    vkDestroyImage(lDevice, textureImage[1], 0);
    vkFreeMemory(lDevice, textureImageMemory[1], 0);

    vkDestroyBuffer(lDevice, vertexBuffer, 0);
    vkFreeMemory(lDevice, vertexBufferMemory,0);

    vkDestroyBuffer(lDevice, indexBuffer, 0);
    vkFreeMemory(lDevice, indexBufferMemory,0);
    for(uint i = 0; i < swapChainSize; i++) vkDestroyImageView(lDevice, swapChainImageViews[i], 0);
    vkDestroyDevice(lDevice,0);
    vkDestroySurfaceKHR(vInstance, surface, 0);
    vkDestroyInstance(vInstance, 0);
    glfwDestroyWindow(window);
    glfwTerminate();

}
bool ui = FALSE;
void runDisplay() {
    imgMaps = malloc(sizeof(int) * swapChainSize);
    memset(imgMaps,0,sizeof(int) * swapChainSize);
#ifdef NOSTAGE
    vkMapMemory(lDevice, vertexBufferMemory, 0, sizeof(vertexArrayObject) * 4, 0, (void**) &vData);
    memcpy(vData, vertices,sizeof vertices);
    vkMapMemory(lDevice, indexBufferMemory, 0, sizeof(unsigned short) * 6, 0, (void**) &iData);
    memcpy(iData, indices, sizeof indices);
#endif


    double avgOpTime = 0;
    int opCount = 0;
    int countDown = 20;
    bool swap = FALSE;
    while(!glfwWindowShouldClose(window)) {
        struct timeval start, end;
        gettimeofday(&start, NULL);
        float tranX = 1;
        vData[0].position.x -= tranX;
        vData[1].position.x -= tranX;
        vData[2].position.x -= tranX;
        vData[3].position.x -= tranX;

        vData[4].position.x += tranX;
        vData[5].position.x += tranX;
        vData[6].position.x += tranX;
        vData[7].position.x += tranX;
        if(countDown == 0) {
            countDown = 20;
            if(!swap) {

                vData[0].texture.x = vData[3].texture.x = 0;
                vData[1].texture.x = vData[2].texture.x = 1023;
                vData[4].texture.x = vData[7].texture.x = 1023;
                vData[5].texture.x = vData[6].texture.x = 2047;
            } else {
                vData[0].texture.x = vData[3].texture.x = 1024;
                vData[1].texture.x = vData[2].texture.x = 2047;
                vData[4].texture.x = vData[7].texture.x = 0;
                vData[5].texture.x = vData[6].texture.x = 1023;
            }

            swap = !swap;

        }
        countDown--;
        VkDeviceSize bufferSize = sizeof(vertices);
#ifndef NOSTAGE
        copyBuffer(stagingBuffer, vertexBuffer, bufferSize);
#endif



        glfwPollEvents(); /**Check for input*/

        vkWaitForFences(lDevice, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

        uint imageIndex;

        /**Get Image from the Swap Chain*/
        VkResult imgRes = vkAcquireNextImageKHR(lDevice, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

        /**Check that Swap Chain is still Optimal*/
        if(imgRes == VK_ERROR_OUT_OF_DATE_KHR) {
            reCreateSwapChain();
            return;
        } else if(imgRes != VK_SUCCESS && imgRes != VK_SUBOPTIMAL_KHR) {
            fprintf(stderr, "Failed to Acquire Swap Chain Image\n");
            return;
        }

        if(imagesInFlight[imageIndex]) vkWaitForFences(lDevice, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
        imagesInFlight[imageIndex] = inFlightFences[currentFrame];

        updateUniformBuffer(imageIndex);
        VkSubmitInfo submitInfo;
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.pNext = 0;

        VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};;
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

        VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        vkResetFences(lDevice, 1, &inFlightFences[currentFrame]);

        if(vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame])) {
            fprintf(stderr,"Failed to submit Draw Command Buffer\n");
            exit(1);
        }


        VkPresentInfoKHR  presentInfo;
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.pNext = 0;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = {swapChain};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = 0;
        VkResult presRes = vkQueuePresentKHR(presentQueue, &presentInfo);

        if(presRes == VK_ERROR_OUT_OF_DATE_KHR || presRes == VK_SUBOPTIMAL_KHR || resized) {
            reCreateSwapChain();
            resized = FALSE;
            printf("Resized: %d\n", resized);
        } else if(presRes != VK_SUCCESS) {
            fprintf(stderr,"Failed to Present Swap Chain Image]n");
            return;
        }



        currentFrame = (currentFrame + 1)% MAX_FRAMES_IN_FLIGHT;
        gettimeofday(&end, NULL);
        //printf("took %f milliseconds\n", ((end.tv_sec - start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec)/1000.0);
        avgOpTime += ((end.tv_sec - start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec)/1000.0;
        opCount++;
        usleep(300);
    }
    vkDeviceWaitIdle(lDevice);
    printf("Average Op Time: %f\n", avgOpTime/opCount);

}

void openDisplay() {
    memset(uniformData.texW,0,sizeof(uniformData.texW));
    initWindow(2048,1024); /**Open Window*/
    createVulkanInstance(); /**Start Vulkan*/
    if(glfwCreateWindowSurface(vInstance, window, 0, &surface)) { /**Link Vulkan to Window*/
        fprintf(stderr, "Failed to generate Window Surface");
        exit(1);
    }
    pickDevice();
    createLogicalDevice();
    createSwapChain();
    createImageViews();
    createRenderPass();
    createDescriptorSetLayout();
    createGraphicsPipeline();
    createFrameBuffers();
    createCommandPool();
    createUniformBuffers();
    createTextureImage(0, "Rochelle.png");
    createTextureImage(1, "CC.png");
    //swapTextureImage(1, "Rochelle.png");
    createTextureImageView();
    createTextureSampler();
    createVertexBuffer();
    createIndexBuffer();

    createDescriptorPool();
    createDescriptorSets();
    createCommandBuffers();

    createSyncObjects();

}