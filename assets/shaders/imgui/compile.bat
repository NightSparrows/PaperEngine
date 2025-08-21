dxc -T vs_6_0 -E main_vs -spirv -fspv-target-env=vulkan1.2 -D TARGET_VULKAN shader.hlsl -Fo shader.vert.spv
dxc -T ps_6_0 -E main_ps -spirv -fspv-target-env=vulkan1.2 -D TARGET_VULKAN shader.hlsl -Fo shader.frag.spv
