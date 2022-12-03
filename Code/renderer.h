// minimalistic code to draw a single triangle, this is not part of the API.
// TODO: Part 1b
#include "FSLogo.h"
#include "shaderc/shaderc.h" // needed for compiling shaders at runtime
#include <chrono>
#ifdef _WIN32 // must use MT platform DLL libraries on windows
#pragma comment(lib, "shaderc_combined.lib") 
#endif

// Load a shader file as a string of characters.
std::string ShaderAsString(const char* shaderFilePath) {
	std::string output;
	unsigned int stringLength = 0;
	GW::SYSTEM::GFile file; file.Create();
	file.GetFileSize(shaderFilePath, stringLength);
	if (stringLength && +file.OpenBinaryRead(shaderFilePath)) {
		output.resize(stringLength);
		file.Read(&output[0], stringLength);
	}
	else
		std::cout << "ERROR: Shader Source File \"" << shaderFilePath << "\" Not Found!" << std::endl;
	return output;
}
// Creation, Rendering & Cleanup
class Renderer
{
	// load in shader file base shaders
	std::string VShader3D = ShaderAsString("../VShader3D.hlsl");
	std::string VPixel3D = ShaderAsString("../PShader3D.hlsl");

	// TODO: Part 2b
#define MAX_SUBMESH_PER_DRAW 1024
	struct SHADER_MODEL_DATA
	{
		// globally shared model data
		GW::MATH::GVECTORF sunDirection; //												16 bytes
		GW::MATH::GVECTORF sunColor; // lighting info									16 bytes
		GW::MATH::GVECTORF sunAmbient;
		GW::MATH::GVECTORF cameraPosition;
		GW::MATH::GMATRIXF viewMatrix;//												64 bytes
		GW::MATH::GMATRIXF projectionMatrix; // viewing info							64 bytes
		// per sub-mesh transform and material data
		GW::MATH::GMATRIXF matricies[MAX_SUBMESH_PER_DRAW]; // world space tranforms	64 bytes
		OBJ_ATTRIBUTES materials[MAX_SUBMESH_PER_DRAW]; // color/texture of surface		80 bytes
		int pixelShaderReturn;
		// total byte size of 304
		// take them out of struct
		// make indiv std vec and matricies
	};
	// proxy handles
	GW::SYSTEM::GWindow win;
	GW::GRAPHICS::GVulkanSurface vlk;
	GW::CORE::GEventReceiver shutdown;

	// what we need at a minimum to draw a triangle
	VkDevice device = nullptr;
	VkBuffer vertexHandle = nullptr;
	VkDeviceMemory vertexData = nullptr;
	// TODO: Part 1g
	VkBuffer indexBuffer;
	VkDeviceMemory indexData;
	// TODO: Part 2c
	std::vector<VkBuffer> storageHandle;
	std::vector<VkDeviceMemory> storageData;
	VkShaderModule vertexShader = nullptr;
	VkShaderModule pixelShader = nullptr;
	// pipeline settings for drawing (also required)
	VkPipeline pipeline = nullptr;
	VkPipelineLayout pipelineLayout = nullptr;
	// TODO: Part 2e
	VkDescriptorSetLayout descriptorLayout = nullptr; // describes order of connection to shaders
	// TODO: Part 2f
	VkDescriptorPool descriptorPool = nullptr; // used to allocate descriptorSets (required)
	// TODO: Part 2g
		// TODO: Part 4f
	std::vector<VkDescriptorSet> descriptorSet;

	// TODO: Part 2a
	GW::MATH::GMATRIXF mWorldText;
	GW::MATH::GMATRIXF mWorldLabel;
	GW::MATH::GMATRIXF mCamera;
	GW::MATH::GMATRIXF mProjection;
	GW::MATH::GVECTORF lightDirection = { -1, -1, 2 };
	GW::MATH::GVECTORF lightColor;
	// TODO: Part 2b
	SHADER_MODEL_DATA shaderData;
	// TODO: Part 4g
	GW::MATH::GVECTORF sunAmbient;
	GW::MATH::GVECTORF cameraPosition;
	std::chrono::steady_clock::time_point elapsed;

	GW::MATH::GMatrix math;
	GW::INPUT::GInput input;
public:
	struct Vertex {
		float pos[3];
		float uvw[3];
		float nrm[3];
	};
	void ReplaceData1C(Vertex(&verts)[3885])
	{
		for (int i = 0; i < 3885; ++i)
		{
			verts[i].pos[0] = FSLogo_vertices[i].pos.x;
			verts[i].pos[1] = FSLogo_vertices[i].pos.y;
			verts[i].pos[2] = FSLogo_vertices[i].pos.z;

			verts[i].uvw[0] = FSLogo_vertices[i].uvw.x;
			verts[i].uvw[1] = FSLogo_vertices[i].uvw.y;
			verts[i].uvw[2] = FSLogo_vertices[i].uvw.z;

			verts[i].nrm[0] = FSLogo_vertices[i].nrm.x;
			verts[i].nrm[1] = FSLogo_vertices[i].nrm.y;
			verts[i].nrm[2] = FSLogo_vertices[i].nrm.z;
		}
	}

	float VectorMagnitude(float x, float y, float z) // the length of the vector
	{
		return sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
	}

	GW::MATH::GVECTORF VectorNormalize(float x, float y, float z)
	{

		float magnitude = VectorMagnitude(x,y,z);
		GW::MATH::GVECTORF vec = { x / magnitude , y / magnitude, z / magnitude };
		return vec;
	}
	Renderer(GW::SYSTEM::GWindow _win, GW::GRAPHICS::GVulkanSurface _vlk)
	{
		win = _win;
		vlk = _vlk;
		unsigned int width, height;
		win.GetClientWidth(width);
		win.GetClientHeight(height);
		// TODO: Part 2a
		input.Create(_win);
		GW::GReturn test;
		math.Create();
		math.IdentityF(mWorldText);
		math.IdentityF(mWorldLabel);
		math.IdentityF(mProjection);

		test = math.ProjectionVulkanLHF(1.13446f, static_cast<float>(width) / height, 0.1f, 100, mProjection);

		math.IdentityF(mCamera);
		GW::MATH::GVECTORF cameraPosition = { 0.75f, 0.25f, -1.5f };
		GW::MATH::GVECTORF cameraLookAt = { 0.15f, 0.75f, 0 };
		GW::MATH::GVECTORF cameraUp = { 0, 1, 0 };
		math.LookAtLHF(cameraPosition, cameraLookAt, cameraUp, mCamera);

		lightColor = GW::MATH::GVECTORF { 0.9f, 0.9f, 1, 1 };

		// TODO: Part 2b
		shaderData.projectionMatrix = mProjection;
		shaderData.viewMatrix = mCamera;
		shaderData.materials[0] = FSLogo_materials[0].attrib;
		shaderData.materials[1] = FSLogo_materials[1].attrib;
		lightDirection = VectorNormalize(lightDirection.x, lightDirection.y, lightDirection.z);
		shaderData.sunDirection = lightDirection;
		shaderData.sunColor = lightColor;
		shaderData.matricies[0] = mWorldText;
		shaderData.matricies[1] = mWorldLabel;
		shaderData.cameraPosition = cameraPosition;
		sunAmbient = GW::MATH::GVECTORF{ 0.25f, 0.25f, 0.35f};
		shaderData.sunAmbient = sunAmbient;
		shaderData.pixelShaderReturn = 6;
		// TODO: part 3b

		/***************** GEOMETRY INTIALIZATION ******************/
		// Grab the device & physical device so we can allocate some stuff
		VkPhysicalDevice physicalDevice = nullptr;
		vlk.GetDevice((void**)&device);
		vlk.GetPhysicalDevice((void**)&physicalDevice);

		// TODO: Part 1c
		// Create Vertex Buffer
		Vertex verts[3885];
		ReplaceData1C(verts);

		unsigned uiBufferSize = sizeof(verts);
		unsigned uiOstensibleVertCount = uiBufferSize / sizeof(Vertex);

		// Transfer triangle data to the vertex buffer. (staging would be prefered here)
		GvkHelper::create_buffer(physicalDevice, device, sizeof(FSLogo_vertices),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &vertexHandle, &vertexData);
		GvkHelper::write_to_buffer(device, vertexData, FSLogo_vertices, sizeof(FSLogo_vertices));
		// TODO: Part 1g
		GvkHelper::create_buffer(physicalDevice, device, sizeof(FSLogo_indices),
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &indexBuffer, &indexData);
		GvkHelper::write_to_buffer(device, indexData, FSLogo_indices, sizeof(FSLogo_indices));
		// TODO: Part 2d
		unsigned max_frames = 0;
		// to avoid per-frame resource sharing issues we give each "in-flight" frame its own buffer
		vlk.GetSwapchainImageCount(max_frames);
		storageHandle.resize(max_frames);
		storageData.resize(max_frames);
		// go to 2375
		for (int i = 0; i < max_frames; ++i) {

			GvkHelper::create_buffer(physicalDevice, device, sizeof(SHADER_MODEL_DATA),
				VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &storageHandle[i], &storageData[i]);
			GvkHelper::write_to_buffer(device, storageData[i], &shaderData, sizeof(SHADER_MODEL_DATA));
		}
		/***************** SHADER INTIALIZATION ******************/
		// Intialize runtime shader compiler HLSL -> SPIRV
		shaderc_compiler_t compiler = shaderc_compiler_initialize();
		shaderc_compile_options_t options = shaderc_compile_options_initialize();
		shaderc_compile_options_set_source_language(options, shaderc_source_language_hlsl);
		shaderc_compile_options_set_invert_y(options, false); // TODO: Part 2i
#ifndef NDEBUG
		shaderc_compile_options_set_generate_debug_info(options);
#endif
		// Create Vertex Shader
		shaderc_compilation_result_t result = shaderc_compile_into_spv( // compile
			compiler, VShader3D.c_str(), VShader3D.length(),
			shaderc_vertex_shader, "main.vert", "main", options);
		if (shaderc_result_get_compilation_status(result) != shaderc_compilation_status_success) // errors?
			std::cout << "Vertex Shader Errors: " << shaderc_result_get_error_message(result) << std::endl;
		GvkHelper::create_shader_module(device, shaderc_result_get_length(result), // load into Vulkan
			(char*)shaderc_result_get_bytes(result), &vertexShader);
		shaderc_result_release(result); // done
		// Create Pixel Shader
		result = shaderc_compile_into_spv( // compile
			compiler, VPixel3D.c_str(), VPixel3D.length(),
			shaderc_fragment_shader, "main.frag", "main", options);
		if (shaderc_result_get_compilation_status(result) != shaderc_compilation_status_success) // errors?
			std::cout << "Pixel Shader Errors: " << shaderc_result_get_error_message(result) << std::endl;
		GvkHelper::create_shader_module(device, shaderc_result_get_length(result), // load into Vulkan
			(char*)shaderc_result_get_bytes(result), &pixelShader);
		shaderc_result_release(result); // done
		// Free runtime shader compiler resources
		shaderc_compile_options_release(options);
		shaderc_compiler_release(compiler);

		/***************** PIPELINE INTIALIZATION ******************/
		// Create Pipeline & Layout (Thanks Tiny!)
		VkRenderPass renderPass;
		vlk.GetRenderPass((void**)&renderPass);
		VkPipelineShaderStageCreateInfo stage_create_info[2] = {};
		// Create Stage Info for Vertex Shader
		stage_create_info[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stage_create_info[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		stage_create_info[0].module = vertexShader;
		stage_create_info[0].pName = "main";
		// Create Stage Info for Fragment Shader
		stage_create_info[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stage_create_info[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		stage_create_info[1].module = pixelShader;
		stage_create_info[1].pName = "main";
		// Assembly State
		VkPipelineInputAssemblyStateCreateInfo assembly_create_info = {};
		assembly_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		assembly_create_info.primitiveRestartEnable = false;
		// TODO: Part 1e
		// Vertex Input State
		VkVertexInputBindingDescription vertex_binding_description = {};
		vertex_binding_description.binding = 0;
		vertex_binding_description.stride = sizeof(Vertex);
		vertex_binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		VkVertexInputAttributeDescription vertex_attribute_description[3] = {
			{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 }, //pos
			{ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 3 }, //uvw
			{ 2, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 6 }, //nrm
		};

		VkPipelineVertexInputStateCreateInfo input_vertex_info = {};
		input_vertex_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		input_vertex_info.vertexBindingDescriptionCount = 1;
		input_vertex_info.pVertexBindingDescriptions = &vertex_binding_description;
		input_vertex_info.vertexAttributeDescriptionCount = 3;
		input_vertex_info.pVertexAttributeDescriptions = vertex_attribute_description;
		// Viewport State (we still need to set this up even though we will overwrite the values)
		VkViewport viewport = {
			0, 0, static_cast<float>(width), static_cast<float>(height), 0, 1
		};
		VkRect2D scissor = { {0, 0}, {width, height} };
		VkPipelineViewportStateCreateInfo viewport_create_info = {};
		viewport_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport_create_info.viewportCount = 1;
		viewport_create_info.pViewports = &viewport;
		viewport_create_info.scissorCount = 1;
		viewport_create_info.pScissors = &scissor;
		// Rasterizer State
		VkPipelineRasterizationStateCreateInfo rasterization_create_info = {};
		rasterization_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterization_create_info.rasterizerDiscardEnable = VK_FALSE;
		rasterization_create_info.polygonMode = VK_POLYGON_MODE_FILL;
		rasterization_create_info.lineWidth = 1.0f;
		rasterization_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterization_create_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterization_create_info.depthClampEnable = VK_FALSE;
		rasterization_create_info.depthBiasEnable = VK_FALSE;
		rasterization_create_info.depthBiasClamp = 0.0f;
		rasterization_create_info.depthBiasConstantFactor = 0.0f;
		rasterization_create_info.depthBiasSlopeFactor = 0.0f;
		// Multisampling State
		VkPipelineMultisampleStateCreateInfo multisample_create_info = {};
		multisample_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisample_create_info.sampleShadingEnable = VK_FALSE;
		multisample_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisample_create_info.minSampleShading = 1.0f;
		multisample_create_info.pSampleMask = VK_NULL_HANDLE;
		multisample_create_info.alphaToCoverageEnable = VK_FALSE;
		multisample_create_info.alphaToOneEnable = VK_FALSE;
		// Depth-Stencil State
		VkPipelineDepthStencilStateCreateInfo depth_stencil_create_info = {};
		depth_stencil_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depth_stencil_create_info.depthTestEnable = VK_TRUE;
		depth_stencil_create_info.depthWriteEnable = VK_TRUE;
		depth_stencil_create_info.depthCompareOp = VK_COMPARE_OP_LESS;
		depth_stencil_create_info.depthBoundsTestEnable = VK_FALSE;
		depth_stencil_create_info.minDepthBounds = 0.0f;
		depth_stencil_create_info.maxDepthBounds = 1.0f;
		depth_stencil_create_info.stencilTestEnable = VK_FALSE;
		// Color Blending Attachment & State
		VkPipelineColorBlendAttachmentState color_blend_attachment_state = {};
		color_blend_attachment_state.colorWriteMask = 0xF;
		color_blend_attachment_state.blendEnable = VK_FALSE;
		color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
		color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_DST_COLOR;
		color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
		color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;
		color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;
		VkPipelineColorBlendStateCreateInfo color_blend_create_info = {};
		color_blend_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		color_blend_create_info.logicOpEnable = VK_FALSE;
		color_blend_create_info.logicOp = VK_LOGIC_OP_COPY;
		color_blend_create_info.attachmentCount = 1;
		color_blend_create_info.pAttachments = &color_blend_attachment_state;
		color_blend_create_info.blendConstants[0] = 0.0f;
		color_blend_create_info.blendConstants[1] = 0.0f;
		color_blend_create_info.blendConstants[2] = 0.0f;
		color_blend_create_info.blendConstants[3] = 0.0f;
		// Dynamic State 
		VkDynamicState dynamic_state[2] = {
			// By setting these we do not need to re-create the pipeline on Resize
			VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR
		};
		VkPipelineDynamicStateCreateInfo dynamic_create_info = {};
		dynamic_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamic_create_info.dynamicStateCount = 2;
		dynamic_create_info.pDynamicStates = dynamic_state;

		// TODO: Part 2e
		VkDescriptorSetLayoutBinding descriptor_layout_binding = {};
		descriptor_layout_binding.binding = 0;
		descriptor_layout_binding.descriptorCount = 1;
		descriptor_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		descriptor_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		descriptor_layout_binding.pImmutableSamplers = nullptr;
		VkDescriptorSetLayoutCreateInfo descriptor_create_info = {};
		descriptor_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptor_create_info.flags = 0;
		descriptor_create_info.bindingCount = 1;
		descriptor_create_info.pBindings = &descriptor_layout_binding;
		descriptor_create_info.pNext = nullptr;
		// Descriptor layout
		VkResult r = vkCreateDescriptorSetLayout(device, &descriptor_create_info,
			nullptr, &descriptorLayout);
		// TODO: Part 2f
		VkDescriptorPoolCreateInfo descriptorpool_create_info = {};
		descriptorpool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		VkDescriptorPoolSize descriptorpool_size = { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, max_frames };
		descriptorpool_create_info.poolSizeCount = 1;
		descriptorpool_create_info.pPoolSizes = &descriptorpool_size;
		descriptorpool_create_info.maxSets = max_frames;
		descriptorpool_create_info.flags = 0;
		descriptorpool_create_info.pNext = nullptr;
		vkCreateDescriptorPool(device, &descriptorpool_create_info, nullptr, &descriptorPool);
			// TODO: Part 4f
		// TODO: Part 2g
		VkDescriptorSetAllocateInfo descriptorset_allocate_info = {};
		descriptorset_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorset_allocate_info.descriptorSetCount = 1;
		descriptorset_allocate_info.pSetLayouts = &descriptorLayout;
		descriptorset_allocate_info.descriptorPool = descriptorPool;
		descriptorset_allocate_info.pNext = nullptr;
		descriptorSet.resize(max_frames);
		for (int i = 0; i < max_frames; ++i) {
			vkAllocateDescriptorSets(device, &descriptorset_allocate_info, &descriptorSet[i]);
		}
			// TODO: Part 4f
		// TODO: Part 2h
		VkWriteDescriptorSet write_descriptorset = {};
		write_descriptorset.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write_descriptorset.descriptorCount = 1;
		write_descriptorset.dstArrayElement = 0;
		write_descriptorset.dstBinding = 0;
		write_descriptorset.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		for (int i = 0; i < max_frames; ++i) {
			write_descriptorset.dstSet = descriptorSet[i];
			VkDescriptorBufferInfo dbinfo = { storageHandle[i], 0, VK_WHOLE_SIZE };
			write_descriptorset.pBufferInfo = &dbinfo;
			vkUpdateDescriptorSets(device, 1, &write_descriptorset, 0, nullptr);
		}
			// TODO: Part 4f
		// Descriptor pipeline layout
		VkPipelineLayoutCreateInfo pipeline_layout_create_info = {};
		pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		// TODO: Part 2e
		pipeline_layout_create_info.setLayoutCount = 1;
		pipeline_layout_create_info.pSetLayouts = &descriptorLayout;
		// TODO: Part 3c
		VkShaderStageFlags stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		VkPushConstantRange pPushConstantRange = { stageFlags, 0, sizeof(unsigned) };
		pipeline_layout_create_info.pushConstantRangeCount = 1;
		pipeline_layout_create_info.pPushConstantRanges = &pPushConstantRange;
		vkCreatePipelineLayout(device, &pipeline_layout_create_info,
			nullptr, &pipelineLayout);
		// Pipeline State... (FINALLY) 
		VkGraphicsPipelineCreateInfo pipeline_create_info = {};
		pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipeline_create_info.stageCount = 2;
		pipeline_create_info.pStages = stage_create_info;
		pipeline_create_info.pInputAssemblyState = &assembly_create_info;
		pipeline_create_info.pVertexInputState = &input_vertex_info;
		pipeline_create_info.pViewportState = &viewport_create_info;
		pipeline_create_info.pRasterizationState = &rasterization_create_info;
		pipeline_create_info.pMultisampleState = &multisample_create_info;
		pipeline_create_info.pDepthStencilState = &depth_stencil_create_info;
		pipeline_create_info.pColorBlendState = &color_blend_create_info;
		pipeline_create_info.pDynamicState = &dynamic_create_info;
		pipeline_create_info.layout = pipelineLayout;
		pipeline_create_info.renderPass = renderPass;
		pipeline_create_info.subpass = 0;
		pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
		vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1,
			&pipeline_create_info, nullptr, &pipeline);

		/***************** CLEANUP / SHUTDOWN ******************/
		// GVulkanSurface will inform us when to release any allocated resources
		shutdown.Create(vlk, [&]() {
			if (+shutdown.Find(GW::GRAPHICS::GVulkanSurface::Events::RELEASE_RESOURCES, true)) {
				CleanUp(); // unlike D3D we must be careful about destroy timing
			}
			});
	}
	void Render()
	{

		float val1 = 0;
		float val2 = 0;
		float val3 = 0;
		float val4 = 0;
		float val5 = 0;
		float val6 = 0;
		input.GetState(G_KEY_F1, val1);
		input.GetState(G_KEY_F2, val2);
		input.GetState(G_KEY_F3, val3);
		input.GetState(G_KEY_F4, val4);
		input.GetState(G_KEY_F5, val5);
		input.GetState(G_KEY_F6, val6);

		if (val1 > 0)
			shaderData.pixelShaderReturn = 1;
		else if (val2 > 0) shaderData.pixelShaderReturn = 2;
		else if (val3 > 0) shaderData.pixelShaderReturn = 3;
		else if (val4 > 0) shaderData.pixelShaderReturn = 4;
		else if (val5 > 0) shaderData.pixelShaderReturn = 5;
		else if (val6 > 0) shaderData.pixelShaderReturn = 6;

		auto currentTime = std::chrono::steady_clock::now();
		float secondsPassed = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - elapsed).count() / 1000000.0f;
		elapsed = currentTime;
		// TODO: Part 4d
		float fDeltaTime = secondsPassed;
		math.RotateYLocalF(shaderData.matricies[1], fDeltaTime, shaderData.matricies[1]); // rotate world matrix 2 degrees slowly over time

		// grab the current Vulkan commandBuffer
		unsigned int currentBuffer;
		vlk.GetSwapchainCurrentImage(currentBuffer);
		VkCommandBuffer commandBuffer;
		vlk.GetCommandBuffer(currentBuffer, (void**)&commandBuffer);
		
		// what is the current client area dimensions?
		unsigned int width, height;
		win.GetClientWidth(width);
		win.GetClientHeight(height);
		// setup the pipeline's dynamic settings
		VkViewport viewport = {
			0, 0, static_cast<float>(width), static_cast<float>(height), 0, 1
		};
		VkRect2D scissor = { {0, 0}, {width, height} };
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

		// now we can draw
		VkShaderStageFlags stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexHandle, offsets);
		// TODO: Part 1h
		vkCmdBindIndexBuffer(commandBuffer, indexBuffer, offsets[0], VK_INDEX_TYPE_UINT32);
		// TODO: Part 4d
		// TODO: Part 2i
		GvkHelper::write_to_buffer(device, storageData[currentBuffer], &shaderData, sizeof(SHADER_MODEL_DATA));
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout, 0, 1, &descriptorSet[currentBuffer], 0, nullptr);
		// TODO: Part 3b
		for (int i = 0; i < FSLogo_meshcount; ++i) // draw first mesh
		{
			UINT push = { (UINT)i };
			// TODO: Part 3d
			vkCmdPushConstants(commandBuffer, pipelineLayout, stageFlags, 0, sizeof(push), &push);
			vkCmdDrawIndexed(commandBuffer, FSLogo_meshes[i].indexCount, 1, FSLogo_meshes[i].indexOffset, 0, 0);
		}

	}
	void UpdateCamera()
	{
		
	}
private:
	void CleanUp()
	{
		// wait till everything has completed
		vkDeviceWaitIdle(device);
		// Release allocated buffers, shaders & pipeline
		// TODO: Part 1g
		vkDestroyBuffer(device, indexBuffer, nullptr);
		vkFreeMemory(device, indexData, nullptr);
		// TODO: Part 2d
		// free the storage buffer and its handle
		for (int i = 0; i < storageData.size(); ++i) {
			vkDestroyBuffer(device, storageHandle[i], nullptr);
			vkFreeMemory(device, storageData[i], nullptr);
		}
		vkDestroyBuffer(device, vertexHandle, nullptr);
		vkFreeMemory(device, vertexData, nullptr);
		vkDestroyShaderModule(device, vertexShader, nullptr);
		vkDestroyShaderModule(device, pixelShader, nullptr);
		// TODO: Part 2e
		vkDestroyDescriptorSetLayout(device, descriptorLayout, nullptr);
		vkDestroyDescriptorPool(device, descriptorPool, nullptr);
		// TODO: part 2f
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		vkDestroyPipeline(device, pipeline, nullptr);
	}
};
