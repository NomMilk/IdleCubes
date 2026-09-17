#include <vulkan/vulkan.h>

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <cstring>
#include <optional>

class EngineApplication
{
	public:
		void Run()
		{
			InitVulkan();
			MainLoop();
			Cleanup();
		}

	private:
		GLFWwindow* window;
		const uint32_t WIDTH = 800;
		const uint32_t HEIGHT = 800;

		//validation layer
		VkDebugUtilsMessengerEXT debugMessenger;
		const std::vector<const char*> validationLayers =
		{
		    "VK_LAYER_KHRONOS_validation"
		};

		//const bool enableValidationLayers = false;
		const bool enableValidationLayers = true;
		
		//vulkan stuff
		VkInstance instance;

		VkSurfaceKHR surface;
		VkDevice device;
		VkQueue graphicsQueue;
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		VkPhysicalDeviceFeatures deviceFeatures{};

		void InitVulkan()
		{
			glfwInit();

			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
			
			window = glfwCreateWindow(WIDTH, HEIGHT, "Game Window", nullptr, nullptr);
			
			//setup
			CreateInstance();
			SetupDebugMessenger();
			PickPhysicalDevice();
			CreateLogicalDevice();
			CreateSurface();
		}

		void CreateSurface()
		{
			if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create window surface!");
			}
		}

		void CreateLogicalDevice()
		{
			std::optional<uint32_t> indices = FindQueueFamilies(physicalDevice);

			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = indices.value();
			queueCreateInfo.queueCount = 1;

			float queuePriority = 1.0f;
			queueCreateInfo.pQueuePriorities = &queuePriority;

			VkDeviceCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			createInfo.pQueueCreateInfos = &queueCreateInfo;
			createInfo.queueCreateInfoCount = 1;

			createInfo.pEnabledFeatures = &deviceFeatures;

			if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create logical device!");
			}

			vkGetDeviceQueue(device, indices.value(), 0, &graphicsQueue);
		}
		
		void PickPhysicalDevice()
		{
			uint32_t deviceCount = 0;
			vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

			if (deviceCount == 0)
			{
				throw std::runtime_error("failed to find GPUs with Vulkan support!");
			}

			std::vector<VkPhysicalDevice> devices(deviceCount);
			vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
			
			for(const auto& device: devices)
			{
				if(IsDeviceSuitable(device))
				{
					physicalDevice = device;
					break;
				}
			}

			if (physicalDevice == VK_NULL_HANDLE)
			{
				throw std::runtime_error("failed to find a suitable GPU!");
			}
		}

		bool IsDeviceSuitable(VkPhysicalDevice device)
		{
			//i'll add some bullshit to check for suitable devices when I'm less lazy
			std::optional<uint32_t> indices = FindQueueFamilies(device);
			return indices.has_value();
		}

		std::optional<uint32_t> FindQueueFamilies(VkPhysicalDevice device)
		{
			std::optional<uint32_t> indices;

			uint32_t queueFamilyCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

			std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

			for (int I = 0; I < queueFamilies.size(); I++)
			{
				if (queueFamilies[I].queueFlags & VK_QUEUE_GRAPHICS_BIT)
				{
					indices = I;
				}
				
				if(indices.has_value())
					return indices;
			}

			return indices;	
		}

		void SetupDebugMessenger()
		{
			if (!enableValidationLayers) return;

			VkDebugUtilsMessengerCreateInfoEXT createInfo;
			PopulateDebugMessengerCreateInfo(createInfo);

			if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
				throw std::runtime_error("failed to set up debug messenger!");
		}

		VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
		{
			auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
			if (func != nullptr)
			{
				return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
			}
			else
			{
				return VK_ERROR_EXTENSION_NOT_PRESENT;
			}
		}

		void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
		{
			auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
			if (func != nullptr)
			{
				func(instance, debugMessenger, pAllocator);
			}
		}

		void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
		{
			createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			createInfo.pfnUserCallback = DebugCallback;
		}

		void MainLoop()
		{
			while (!glfwWindowShouldClose(window))
			{
				glfwPollEvents();
			}
		}

		void Cleanup()
		{
			if(enableValidationLayers)
			{
				DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
			}

			vkDestroySurfaceKHR(instance, surface, nullptr);
			vkDestroyDevice(device, nullptr);
			vkDestroyInstance(instance, nullptr);
			glfwDestroyWindow(window);
			glfwTerminate();
		}

		//vulkan specific stuff
		void CreateInstance()		
		{
			if (enableValidationLayers && !CheckValidationLayerSupport())
			{
				throw std::runtime_error("validation layers requested, but not available!");
			}

			VkApplicationInfo appInfo{};
			appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			appInfo.pApplicationName = "Game";
			appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
			appInfo.pEngineName = "Fish Engine";
			appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
			appInfo.apiVersion = VK_API_VERSION_1_0;

			VkInstanceCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			createInfo.pApplicationInfo = &appInfo;

			auto extensions = GetRequiredExtensions();
			createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
			createInfo.ppEnabledExtensionNames = extensions.data();

			VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
			if (enableValidationLayers)
			{
				createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
				createInfo.ppEnabledLayerNames = validationLayers.data();

				PopulateDebugMessengerCreateInfo(debugCreateInfo);
				createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
			}
			else
			{
				createInfo.enabledLayerCount = 0;
				createInfo.pNext = nullptr;
			}

			VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);

			if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create instance!");
			}
		}

		bool CheckValidationLayerSupport()
		{
			uint32_t layerCount;
			vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

			std::vector<VkLayerProperties> availableLayers(layerCount);
			vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
			for (const char* layerName : validationLayers)
			{
				bool layerFound = false;

				for (const auto& layerProperties : availableLayers)
				{
					if (strcmp(layerName, layerProperties.layerName) == 0)
					{
						layerFound = true;
						break;
					}
				}

				if (!layerFound) {
					return false;
				}
			}

			return true;
		}

		std::vector<const char*> GetRequiredExtensions()
		{
			uint32_t glfwExtensionCount = 0;
			const char** glfwExtensions;
			glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

			std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

			if (enableValidationLayers)
			{
				extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			}

			return extensions;
		}

		//Vulkan called API
		static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback
		(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData) {

			std::cerr << "Validation Layer: " << pCallbackData->pMessage << std::endl;

			return VK_FALSE;
		}
};

int main()
{
	EngineApplication app;

	try
	{
		app.Run();
	} catch (const std::exception& _e) {
		std::cerr << _e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
