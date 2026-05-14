@echo off
D:\VulkanSDK\1.4.341.1\Bin\glslc.exe shaders\default.vert -o shaders\default.vert.spv
D:\VulkanSDK\1.4.341.1\Bin\glslc.exe shaders\default.frag -o shaders\default.frag.spv
D:\VulkanSDK\1.4.341.1\Bin\glslc.exe shaders\pointLight.vert -o shaders\pointLight.vert.spv
D:\VulkanSDK\1.4.341.1\Bin\glslc.exe shaders\pointLight.frag -o shaders\pointLight.frag.spv
D:\VulkanSDK\1.4.341.1\Bin\glslc.exe shaders\particle.vert -o shaders\particle.vert.spv
D:\VulkanSDK\1.4.341.1\Bin\glslc.exe shaders\particle.frag -o shaders\particle.frag.spv
echo Shaders compiled successfully!