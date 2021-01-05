#pragma once
#include <memory>

namespace plumbus::vk
{
    class Instance;
    typedef std::shared_ptr<Instance> InstanceRef;

    class Device;
    typedef std::shared_ptr<Device> DeviceRef;

    class SwapChain;
    typedef std::shared_ptr<SwapChain> SwapChainRef;

    class CommandBuffer;
    typedef std::shared_ptr<CommandBuffer> CommandBufferRef;

    class FrameBuffer;
    typedef std::shared_ptr<FrameBuffer> FrameBufferRef;

	class DescriptorSet;
	typedef std::shared_ptr<DescriptorSet> DescriptorSetRef;

    class DescriptorPool;
    typedef std::shared_ptr<DescriptorPool> DescriptorPoolRef;

    class DescriptorSetLayout;
    typedef std::shared_ptr<DescriptorSetLayout> DescriptorSetLayoutRef;

	class PipelineCache;
	typedef std::shared_ptr<PipelineCache> PipelineCacheRef;

    class PipelineLayout;
    typedef std::shared_ptr<PipelineLayout> PipelineLayoutRef;

    class Pipeline;
    typedef std::shared_ptr<Pipeline> PipelineRef;

    class Material;
    typedef std::shared_ptr<Material> MaterialRef;

    class MaterialInstance;
    typedef std::shared_ptr<MaterialInstance> MaterialInstanceRef;

    class Shadow;
    typedef std::shared_ptr<Shadow> ShadowRef;

    class ShadowDirectional;
    typedef std::shared_ptr<ShadowDirectional> ShadowDirectionalRef;

    class ShadowOmniDirectional;
    typedef std::shared_ptr<ShadowOmniDirectional> ShadowOmniDirectionalRef;
}