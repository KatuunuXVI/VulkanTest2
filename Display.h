#ifndef CKDISPLAY
#define CKDISPLAY

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include "Extools.h"

/******************
 * Device Structs *
 ******************/

/**Structure for marking the Queue Families in Use*/
typedef struct QueueFamilies_T QueueFamilies;

/**Provides information about Swap Chain Features that are available*/
typedef struct SwapChainSupportDetails_T SwapChainSupportDetails;

/******************
 * Render Structs *
 ******************/

typedef struct positionVertex positionVertex;

typedef struct colorVertex colorVertex;

typedef struct vertexArrayObject vertexArrayObject;

#ifndef NDEBUG

/************************
 * Validation Functions *
 ************************/

/**Determines if Validation Layers are Available*/
bool checkValidationLayerSupport();


/**Initialize Debug Messenger Extension*/
VkResult CreateDebugUtilsMessengerEXT(VkInstance inst, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator);

/**Validation Layer Message Handling Function*/
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData);

/**Deallocates the Debug Messenger Extension*/
void DestroyDebugUtilsMessengerEXT(VkInstance inst, VkDebugUtilsMessengerEXT dMessenger, const VkAllocationCallbacks* pAllocator);

#endif

/********************
 * Device Functions *
 ********************/


/**Selects the optimal physical device available*/
void pickDevice();

/**Determines if a given physical device is compatible with the applications functionality*/
bool deviceSuitable(VkPhysicalDevice dev);

/**Checks the available Queue types from device*/
QueueFamilies findQueueFamilies(VkPhysicalDevice dev);

/**Determines if a physical device extension is supported*/
bool checkDeviceExtensionSupport(VkPhysicalDevice device);

/**Gathers info on the swap chain features supported*/
SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice dev);

/**Determines which features of the physical device will be used*/
void createLogicalDevice();


/********************
 * Render Functions *
 ********************/

/**Specifies the color and depth buffers and the samples to use for them*/
void createRenderPass();

/**Initialize Rendering Pipeline*/
void createGraphicsPipeline();

/**Populate Frame Buffers*/
void createFrameBuffers();

/**Create objects for handling synchronization*/
void createSynchronization();

/**Get Code for Vertex Shader*/
const char* loadVertexShader();

/**Get Code for FragmentShader*/
const char* loadFragmentShader();

/**Loads compiled Shaders*/
VkShaderModule createShaderModule(const char* filename);

/**Object that allows Physical Device to read Vertex Data*/
void createVertexBuffer();

unsigned int findMemoryType(unsigned typeFilter, VkMemoryPropertyFlags properties);

void createIndexBuffer();

void createUniformBuffers();

void updateUniformBuffer(uint currentImage);

/**Renders final output to screen*/
void draw();

/************************
 * Swap Chain Functions *
 ************************/
/**Creates Swap Chain*/
void createSwapChain();

/**Selects the optimal Surface Format available*/
VkSurfaceFormatKHR chooseSwapSurfaceFormat(const VkSurfaceFormatKHR* availableFormats, unsigned int formatCount);

/**Selects the Optimal Presentation Mode available*/
VkPresentModeKHR chooseSwapPresentMode(const VkPresentModeKHR* availableModes, unsigned int modeCount);

/**Queries the maximum Swap Chain Size*/
VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR* capabilities);

/**Creates the image settings to send to the swap chain*/
void createImageViews();

/**Generate new Swap Chain if Window is no longer compatible with current one*/
void reCreateSwapChain();

/**Clean old Swap Chain Objects before Recreation*/
void cleanSwapChain();


/*********************
 * Command Functions *
 *********************/

/**Initialize Objects for handling buffer synchronization*/

void createCommandPool();

void createCommandBuffers();

void createSyncObjects();



/********************
 * Window Functions *
 ********************/

/**Initialize Vulkan Context*/
void createVulkanInstance();

void createDescriptorSetLayout();

/**Opens the Window for User Interaction*/
void initWindow(int width, int height);

static void resizeCallback(GLFWwindow* window, int width, int height);

/*********************
 * General Functions *
 *********************/

/**Shuts down all display features*/
void closeDisplay();

/**Main Function to call in the loop*/
void runDisplay();

/**Initializes all the required display features*/
void openDisplay();
#endif
