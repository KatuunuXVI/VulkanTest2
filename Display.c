



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Display.h"
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


const int extensionCount = 1; /**Number of Device Extensions*/

const char* requiredExtensions[1] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME }; /**List of Device Extensions*/

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

/**Routine Object for Rendering Graphics*/
static VkPipelineLayout pipelineLayout;

/**Graphics Pipeline*/
static VkPipeline gPipeline;

/**Determines How Render Operation is performed in Memory*/
static VkRenderPass renderPass;

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

/**********************
 * Struct Definitions *
 **********************/

typedef struct QueueFamilies_T {
    unsigned char familiesFound;
    unsigned int familyCount;
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
    return dFeatures.geometryShader &&
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
    foundFamilies.familyCount = familyCount;

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

    /**Define how attachment data is handled*/
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
}

void createGraphicsPipeline() {
    VkShaderModule vShader = createShaderModule("shaders/vert.spv");
    VkShaderModule fShader = createShaderModule("shaders/frag.spv");

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
    vInputInfo.vertexBindingDescriptionCount =
    vInputInfo.vertexAttributeDescriptionCount = 0;
    vInputInfo.pVertexBindingDescriptions = 0;
    vInputInfo.pVertexAttributeDescriptions = 0;

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
    vpState.pScissors = &scissor;

    /**Initialize Rasterizer*/
    VkPipelineRasterizationStateCreateInfo rasterizer;
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.flags = 0;
    rasterizer.pNext = 0;
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
    plInfo.setLayoutCount = 0;
    plInfo.pSetLayouts = 0;
    plInfo.pushConstantRangeCount = 0;
    VkResult res;
    if((res = vkCreatePipelineLayout(lDevice, &plInfo,0,&pipelineLayout)) != VK_SUCCESS) {
        fprintf(stderr, "Failed to Create Pipeline: %d\n", res);
        exit(EXIT_FAILURE);
    }

    /**Consolidate Pipeline*/
    VkGraphicsPipelineCreateInfo pipeline;
    pipeline.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline.flags = 0;
    pipeline.pNext = 0;
    pipeline.stageCount = 2;
    pipeline.pStages = shaderStages;
    pipeline.pVertexInputState = &vInputInfo;
    pipeline.pInputAssemblyState = &inputAssembly;
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

    if(vkCreateGraphicsPipelines(lDevice, 0, 1, &pipeline, 0, &gPipeline) != VK_SUCCESS) {
        fprintf(stderr, "Failed to Create Graphics Pipeline\n");
        exit(1);
    }

    vkDestroyShaderModule(lDevice, vShader, 0);
    vkDestroyShaderModule(lDevice, fShader, 0);
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

    for(uint i = 0; i < swapChainSize; i++) {
        VkImageViewCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapChainImages[i];
        createInfo.flags = 0;
        createInfo.pNext = 0;
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapChainImageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;
        if(vkCreateImageView(lDevice, &createInfo, 0, &swapChainImageViews[i]) != VK_SUCCESS) {
            fprintf(stderr, "Failed to Create Images Views\n");
            exit(EXIT_FAILURE);
        }
    }
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

    /**Clean Objects*/
    cleanSwapChain();

    createSwapChain();
    createImageViews();
    createRenderPass();
    createGraphicsPipeline();
    createFrameBuffers();
    createCommandBuffers();

}

void cleanSwapChain() {
    /**Remove Frame Buffers*/
    for(uint i = 0; i < swapChainSize; i++) vkDestroyFramebuffer(lDevice, swapChainFrameBuffers[i], 0);

    vkFreeCommandBuffers(lDevice, commandPool, swapChainSize, commandBuffers);

    vkDestroyPipeline(lDevice, gPipeline, 0);
    vkDestroyPipelineLayout(lDevice, pipelineLayout, 0);
    vkDestroyRenderPass(lDevice, renderPass, 0);

    for(int i = 0; i < swapChainSize; i++) vkDestroyImageView(lDevice, swapChainImageViews[i], 0);

    vkDestroySwapchainKHR(lDevice, swapChain, 0);
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
        VkClearValue clearColor =  {{{0,0,0, 1}}};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;
        vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, gPipeline);
        vkCmdDraw(commandBuffers[i],3,1,0,0);
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

static void resizeCallback(GLFWwindow* window, int width, int height) {
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
    for(uint i = 0; i < swapChainSize; i++) vkDestroyImageView(lDevice, swapChainImageViews[i], 0);
    vkDestroyDevice(lDevice,0);
    vkDestroySurfaceKHR(vInstance, surface, 0);
    vkDestroyInstance(vInstance, 0);
    glfwDestroyWindow(window);
    glfwTerminate();

}

void runDisplay() {
    while (!glfwWindowShouldClose(window)) {
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
        } else if(presRes != VK_SUCCESS) {
            fprintf(stderr,"Failed to Present Swap Chain Image]n");
            return;
        }



        currentFrame = (currentFrame + 1)% MAX_FRAMES_IN_FLIGHT;
    }
    vkDeviceWaitIdle(lDevice);
}

void openDisplay() {
    initWindow(1024,576); /**Open Window*/
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
    createGraphicsPipeline();
    createFrameBuffers();
    createCommandPool();
    createCommandBuffers();
    createSyncObjects();
}