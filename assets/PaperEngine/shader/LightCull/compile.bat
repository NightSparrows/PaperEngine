dxc -T cs_6_0 -E main_cs -spirv -fspv-target-env=vulkan1.2 -D TARGET_VULKAN lightCull.hlsl -Fo lightCull.comp.spv
