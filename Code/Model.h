// Object Oriented Approach to Data Driven Rendering

// Each model will have"
// - vertex/index buffer
// - A Parser
// - own Shader_Variable_Struct
// - storage / constant buffer
// - descriptorSet / gpuAddress to it

// Each model in the Level
// - Load data from .h2b file
// - populate uniforms
#pragma once


class Model
{
private:
	VkDevice device = nullptr;
	GW::GRAPHICS::GVulkanSurface surfaceReference;
	GW::SYSTEM::GWindow window;

	std::vector<VkBuffer> storageBuffer;
	std::vector<VkDeviceMemory> storageBufferData;
	VkBuffer vertexBuffer = nullptr;
	VkDeviceMemory vertexBufferData = nullptr;
	VkBuffer indexBuffer = nullptr;
	VkDeviceMemory indexBufferData = nullptr;
	VkShaderModule vertexShader = nullptr;
	VkShaderModule pixelShader = nullptr;
	VkPipeline pipeline = nullptr;
	VkPipelineLayout pipelineLayout = nullptr;
	VkDescriptorSetLayout descriptorLayout = nullptr; // describes order of connection to shaders
	VkDescriptorPool descriptorPool = nullptr; // used to allocate descriptorSets (required)
	std::vector<VkDescriptorSet> descriptorSet;
	unsigned max_frames;

	SHADER_MODEL_DATA shaderData;
	H2B::Parser readModel;
	ManageRenderer manager;

	unsigned vertexCount;
	unsigned indexCount;
	unsigned materialCount;
	unsigned meshCount;
	std::vector<Vertex> vertices;
	std::vector<unsigned> indices;
	std::vector<Material> materials;
	std::vector<Batch> batches;
	std::vector<Mesh> meshes;

	// load in shader file base shaders
	std::string VShader3D = manager.ShaderAsString("../VShader3D.hlsl");
	std::string VPixel3D = manager.ShaderAsString("../PShader3D.hlsl");

	void GeometryInitialization()
	{
		/***************** GEOMETRY INTIALIZATION ******************/
		// Grab the device & physical device so we can allocate some stuff
		VkPhysicalDevice physicalDevice = nullptr;
		surfaceReference.GetDevice((void**)&device);
		surfaceReference.GetPhysicalDevice((void**)&physicalDevice);
		unsigned int verticesSize = sizeof(Vertex) * vertices.size();
		unsigned uiBufferSize = verticesSize;
		unsigned uiOstensibleVertCount = uiBufferSize / sizeof(Vertex);

		GvkHelper::create_buffer(physicalDevice, device, verticesSize,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &vertexBuffer, &vertexBufferData);
		GvkHelper::write_to_buffer(device, vertexBufferData, vertices.data(), verticesSize);

		unsigned int sizeOfVectUINT = sizeof(std::vector<unsigned>);
		unsigned int indicesSize = sizeof(unsigned int) *  indices.size();
		GvkHelper::create_buffer(physicalDevice, device, indicesSize,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &indexBuffer, &indexBufferData);
		GvkHelper::write_to_buffer(device, indexBufferData, indices.data(), indicesSize);

		max_frames = 0;
		// to avoid per-frame resource sharing issues we give each "in-flight" frame its own buffer
		surfaceReference.GetSwapchainImageCount(max_frames);
		storageBuffer.resize(max_frames);
		storageBufferData.resize(max_frames);
		for (int i = 0; i < max_frames; ++i) {

			GvkHelper::create_buffer(physicalDevice, device, sizeof(SHADER_MODEL_DATA),
				VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &storageBuffer[i], &storageBufferData[i]);
			GvkHelper::write_to_buffer(device, storageBufferData[i], &shaderData, sizeof(SHADER_MODEL_DATA));
		}
	}
	void ShaderInitialization()
	{
		manager.InitializeShaders(device, vertexShader, pixelShader, VShader3D, VPixel3D);
	}
	void PipelineInitialization()
	{
		unsigned width, height;
		manager.GetDimensions(window, width, height);
		manager.InitializePipeline(surfaceReference, vertexShader, pixelShader, width, height, device, descriptorLayout, descriptorPool, descriptorSet, storageBuffer, max_frames, pipelineLayout, pipeline);
	}
	void ParserToModel(H2B::Parser& parser)
	{
		unsigned vertexCount = parser.vertexCount;
		unsigned indexCount = parser.indexCount;
		unsigned materialCount = parser.materialCount;
		unsigned meshCount = parser.meshCount;
		std::vector<Vertex> vertices;
		std::vector<unsigned> indices = parser.indices;
		std::vector<Material> materials;
		std::vector<Batch> batches;
		std::vector<Mesh> meshes;

		for (int i = 0; i < parser.vertices.size(); ++i)
		{
			Vertex temp;
			temp.pos[0] = parser.vertices[i].pos.x;
			temp.pos[1] = parser.vertices[i].pos.y;
			temp.pos[2] = parser.vertices[i].pos.z;
			temp.nrm[0] = parser.vertices[i].nrm.x;
			temp.nrm[1] = parser.vertices[i].nrm.y;
			temp.nrm[2] = parser.vertices[i].nrm.z;
			temp.uvw[0] = parser.vertices[i].uvw.x;
			temp.uvw[1] = parser.vertices[i].uvw.y;
			temp.uvw[2] = parser.vertices[i].uvw.z;
			//temp.uvw[2] = 1;
			vertices.push_back(temp);
		}
		for (int i = 0; i < parser.materials.size(); ++i)
		{
			Material temp;
			manager.H2BVectorToVector(temp.attrib.Kd, parser.materials[i].attrib.Kd);
			manager.H2BVectorToVector(temp.attrib.Ks, parser.materials[i].attrib.Ks);
			manager.H2BVectorToVector(temp.attrib.Ka, parser.materials[i].attrib.Ka);
			manager.H2BVectorToVector(temp.attrib.Tf, parser.materials[i].attrib.Tf);
			manager.H2BVectorToVector(temp.attrib.Ke, parser.materials[i].attrib.Ke);
			temp.attrib.d = parser.materials[i].attrib.d;
			temp.attrib.Ns = parser.materials[i].attrib.Ns;
			temp.attrib.sharpness = parser.materials[i].attrib.sharpness;
			temp.attrib.Ni = parser.materials[i].attrib.Ni;
			temp.attrib.illum = parser.materials[i].attrib.illum;
			// textures
			/*temp.name = parser.materials[i].name;
			temp.map_Kd = parser.materials[i].map_Kd;
			temp.map_Ks = parser.materials[i].map_Ks;
			temp.map_Ka = parser.materials[i].map_Ka;
			temp.map_Ke = parser.materials[i].map_Ke;
			temp.map_Ns = parser.materials[i].map_Ns;
			temp.map_d = parser.materials[i].map_d;
			temp.disp = parser.materials[i].disp;
			temp.decal = parser.materials[i].decal;
			temp.bump = parser.materials[i].bump;*/
			materials.push_back(temp);
		}
		for (int i = 0; i < parser.batches.size(); ++i)
		{
			Batch temp;
			manager.H2BBatchToBatch(temp, parser.batches[i]);
			batches.push_back(temp);
		}
		for (int i = 0; i < parser.meshes.size(); ++i)
		{
			Mesh temp;
			manager.H2BBatchToBatch(temp.drawInfo, parser.meshes[i].drawInfo);
			temp.name = parser.meshes[i].name;
			temp.materialIndex = parser.meshes[i].materialIndex;
			meshes.push_back(temp);
		}
		SetModelData(vertexCount, indexCount, materialCount, meshCount, vertices, indices, materials, batches, meshes);
	}

public:
	void ModelRenderSetup()
	{
		GeometryInitialization();
		ShaderInitialization();
		PipelineInitialization();
	}
	bool LoadModel(std::string h2bFilePath, GW::GRAPHICS::GVulkanSurface _surfaceReference, GW::SYSTEM::GWindow _window, VkDevice _device)
	{
		if (readModel.Parse(h2bFilePath.c_str()))
		{
			// tranfer from CPU(Parser) to GPU aka the API(VkBuffer)
			ParserToModel(readModel);
			surfaceReference = _surfaceReference;
			window = _window;
			device = _device;
			return true;
		}
		else
		{
			std::cout << h2bFilePath << " is not a valid file path. Please check the .h2b name associated with this path...\n" << std::endl;
			return false;
		}

	}
	void DrawModel(VkCommandBuffer commandBuffer, unsigned int currentBuffer, VkViewport viewport, VkRect2D scissor)
	{
		
		manager.PipelineDynamicSettings(commandBuffer, pipeline, viewport, scissor, VK_PIPELINE_BIND_POINT_GRAPHICS);

		manager.BindVertexInputDescripterSets(commandBuffer, device, vertexBuffer, indexBuffer, storageBufferData, currentBuffer, descriptorSet, pipelineLayout, shaderData);
		shaderData.viewMatrix = mCamera;
		VkShaderStageFlags stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		for (int i = 0; i < meshes.size(); ++i)
		{
			UINT push = { (UINT)meshes[i].materialIndex };
			manager.DrawMesh(commandBuffer, pipelineLayout, stageFlags, push, meshes[i].drawInfo.indexCount, meshes[i].drawInfo.indexOffset);
		}
	}
	void SetModelData(unsigned _vertexCount, unsigned _indexCount, unsigned _materialCount, unsigned _meshCount, std::vector<Vertex> _vertices, std::vector<unsigned> _indices,
		std::vector<Material> _materials, std::vector<Batch> _batches, std::vector<Mesh> _meshes)
	{
		vertexCount = _vertexCount;
		indexCount = _indexCount;
		materialCount = _materialCount;
		meshCount = _meshCount;
		vertices = _vertices;
		indices = _indices;
		materials = _materials;
		batches = _batches;
		meshes = _meshes;
	}
	void SetShaderModelData(GW::MATH::GVECTORF _sunDirection, GW::MATH::GVECTORF _sunColor, GW::MATH::GVECTORF _sunAmbiant, GW::MATH::GVECTORF _cameraPosition,
		GW::MATH::GMATRIXF _projectionMatrix, GW::MATH::GMATRIXF _matricies[MAX_SUBMESH_PER_DRAW], int _pixelShaderReturn)
	{
		shaderData.sunDirection = _sunDirection;
		shaderData.sunColor = _sunColor;
		shaderData.sunAmbient = _sunAmbiant;
		shaderData.cameraPosition = _cameraPosition;
		shaderData.viewMatrix = mCamera;
		shaderData.projectionMatrix = _projectionMatrix;
		for (int i = 0; i < sizeof(_matricies); ++i)
		{
			shaderData.matricies[i] = _matricies[i];
		}
		for (int i = 0; i < materials.capacity(); ++i)
		{
			shaderData.materials[i] = materials[i];
		}
		shaderData.pixelShaderReturn = _pixelShaderReturn;
	}
	void CleanUp()
	{
		// wait till everything has completed
		vkDeviceWaitIdle(device);
		// Release allocated buffers, shaders & pipeline
		vkDestroyBuffer(device, indexBuffer, nullptr);
		vkFreeMemory(device, indexBufferData, nullptr);
		// free the storage buffer and its handle
		for (int i = 0; i < storageBufferData.size(); ++i) {
			vkDestroyBuffer(device, storageBuffer[i], nullptr);
			vkFreeMemory(device, storageBufferData[i], nullptr);
		}
		vkDestroyBuffer(device, vertexBuffer, nullptr);
		vkFreeMemory(device, vertexBufferData, nullptr);
		vkDestroyShaderModule(device, vertexShader, nullptr);
		vkDestroyShaderModule(device, pixelShader, nullptr);
		vkDestroyDescriptorSetLayout(device, descriptorLayout, nullptr);
		vkDestroyDescriptorPool(device, descriptorPool, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		vkDestroyPipeline(device, pipeline, nullptr);
	}
};