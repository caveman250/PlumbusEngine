#pragma once

namespace plumbus
{
	struct VertexLayout;

	namespace base
	{
		class Material
		{
		public:
			Material(const char* vertShader, const char* fragShader) {}
			~Material() { Destroy(); }
			virtual void Setup(VertexLayout* layout) {}
			virtual void Destroy() {}
		};
	}

	enum VertexLayoutComponent
	{
		VERTEX_COMPONENT_POSITION = 0x0,
		VERTEX_COMPONENT_NORMAL = 0x1,
		VERTEX_COMPONENT_COLOR = 0x2,
		VERTEX_COMPONENT_UV = 0x3,
		VERTEX_COMPONENT_TANGENT = 0x4,
		VERTEX_COMPONENT_BITANGENT = 0x5,
		VERTEX_COMPONENT_DUMMY_FLOAT = 0x6,
		VERTEX_COMPONENT_DUMMY_VEC4 = 0x7
	};

	struct VertexLayout
	{
	public:
		std::vector<VertexLayoutComponent> components;

		VertexLayout()
		{
		}

		VertexLayout(std::vector<VertexLayoutComponent> components)
		{
			this->components = std::move(components);
		}

		uint32_t stride()
		{
			uint32_t res = 0;
			for (auto& component : components)
			{
				switch (component)
				{
				case VERTEX_COMPONENT_UV:
					res += 2 * sizeof(float);
					break;
				case VERTEX_COMPONENT_DUMMY_FLOAT:
					res += sizeof(float);
					break;
				case VERTEX_COMPONENT_DUMMY_VEC4:
					res += 4 * sizeof(float);
					break;
				default:
					// All components except the ones listed above are made up of 3 floats
					res += 3 * sizeof(float);
				}
			}
			return res;
		}
	};
}