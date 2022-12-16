#include <commdlg.h>

#ifdef _WIN32 // must use MT platform DLL libraries on windows
#pragma comment(lib, "shaderc_combined.lib") 
#endif

// Creation, Rendering & Cleanup
class Renderer
{
	ManageRenderer manager;

	// proxy handles
	VkDevice device = nullptr;
	GW::SYSTEM::GWindow win;
	GW::GRAPHICS::GVulkanSurface vlk;
	GW::CORE::GEventReceiver shutdown;


	float FOV;
	float AspectRatio;

	GW::MATH::GMATRIXF mProjection;
	GW::MATH::GVECTORF lightDirection = { -1, -1, 2 };
	GW::MATH::GVECTORF lightColor;
	GW::MATH::GVECTORF sunAmbient;

	GW::MATH::GMatrix math;
	GW::INPUT::GInput input;

	int numOfLevels = 2;
	std::vector<Model> activeLevel;

	bool isLevelLoaded = false;

	

public:
	
	bool LoadLevel(std::string pathtoGameLevel, std::string pathToH2BFiles, std::vector<Model>& modelStoage, GW::MATH::GMATRIXF mProjection)
	{
		CleanUpLevel(); // clear all the models from the CPU and GPU
		modelStoage.clear(); // clear the storage vector for the new models
		LoadLevelData matrixLoader = LoadLevelData();
		matrixLoader.LoadData(pathtoGameLevel, false); // loads text file
		int meshCount = matrixLoader.meshes.size();
		for (int i = 0; i < meshCount; ++i) // for each mesh in the text file
		{
			Model model = Model(); // create temp model
			bool isLoaded = model.LoadModel(pathToH2BFiles + matrixLoader.meshes[i].meshName, vlk, win, device); // load the .h2b file of that model
			if (isLoaded) // if opening the model worked
			{
				model.modelName = matrixLoader.meshes[i].meshName;
				GW::MATH::GVECTORF _sunDirection = { -1, -1, 2 };
				GW::MATH::GVECTORF _sunColor = { 0.9f, 0.9f, 1, 1 };
				GW::MATH::GVECTORF _sunAmbiant = { 0.25f, 0.25f, 0.35f };
				GW::MATH::GVECTORF _cameraPosition = mCamera.row4.xyzw();
				GW::MATH::GMATRIXF _projectionMatrix = mProjection;
				GW::MATH::GMATRIXF _matricies[MAX_SUBMESH_PER_DRAW];
				_matricies[0] = manager.LoaderMatrixToGMATRIXF(matrixLoader.meshes[i].meshMatrix);
				int _pixelShaderReturn = 5;
				mCamera = manager.LoaderMatrixToGMATRIXF(matrixLoader.cameras[0].cameraMatrix);
				math.InverseF(mCamera, mCamera);
				model.SetShaderModelData(_sunDirection, _sunColor, _sunAmbiant, _cameraPosition, _projectionMatrix, _matricies, _pixelShaderReturn);
				modelStoage.push_back(model); // add the model to the vector of models for this level
			}
			else// yell at machine
			{
				return false;
			}
		}
		return true; // congrats it works
	}
	void LevelRenderSetup(std::vector<Model>& modelStorage)
	{
		for (int i = 0; i < modelStorage.size(); ++i)
		{
			std::cout << "Setup STARTING for model: " + modelStorage[i].modelName << std::endl;
			modelStorage[i].ModelRenderSetup();
			std::cout << "Setup COMPLETE for model: " + modelStorage[i].modelName << std::endl;
		}
	}
	void DrawLevel(std::vector<Model>& modelStoage, VkCommandBuffer& commandBuffer, unsigned int currentBuffer, VkViewport& viewport, VkRect2D& scissor)
	{
		for (int i = 0; i < modelStoage.size(); ++i)
		{
			modelStoage[i].DrawModel(commandBuffer, currentBuffer, viewport, scissor);
		}
	}
	Renderer(GW::SYSTEM::GWindow _win, GW::GRAPHICS::GVulkanSurface _vlk)
	{
		win = _win;
		vlk = _vlk;
		unsigned int width, height;
		manager.GetDimensions(win, width, height);
		input.Create(_win);
		math.Create();


		math.IdentityF(mProjection);
		FOV = G_DEGREE_TO_RADIAN(65);
		AspectRatio = static_cast<float>(width) / height;
		math.ProjectionVulkanLHF(FOV, AspectRatio, 0.1f, 100, mProjection);

		lightColor = GW::MATH::GVECTORF{ 0.9f, 0.9f, 1, 1 };
		lightDirection = manager.VectorNormalize(lightDirection.x, lightDirection.y, lightDirection.z);
		sunAmbient = GW::MATH::GVECTORF{ 0.25f, 0.25f, 0.35f };

		/***************** CLEANUP / SHUTDOWN ******************/
		// GVulkanSurface will inform us when to release any allocated resources
		shutdown.Create(vlk, [&]() {
			if (+shutdown.Find(GW::GRAPHICS::GVulkanSurface::Events::RELEASE_RESOURCES, true)) {
				CleanUpLevel(); // unlike D3D we must be careful about destroy timing
			}
			});
	}
	void Render()
	{
		// what is the current client area dimensions?
		unsigned int width, height;
		manager.GetDimensions(win, width, height);

		// grab the current Vulkan commandBuffer
		unsigned int currentBuffer;
		vlk.GetSwapchainCurrentImage(currentBuffer);
		VkCommandBuffer commandBuffer;
		vlk.GetCommandBuffer(currentBuffer, (void**)&commandBuffer);

		// setup the pipeline's dynamic settings
		VkViewport viewport = { 0, 0, static_cast<float>(width), static_cast<float>(height), 0, 1 }; // x, y, width, height, minDepth, maxDepth
		VkRect2D scissor = { {0, 0}, {width, height} }; // offsets, dimensions
		if (isLevelLoaded)
			DrawLevel(activeLevel, commandBuffer, currentBuffer, viewport, scissor);
	}
	void LevelChanger()
	{
		OpenFileDialog();
	}
	void UpdateCamera()
	{
		auto currentTime = std::chrono::steady_clock::now();
		float secondsPassed = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - elapsed).count() / 100000.0f;
		elapsed = currentTime;


		float valF1 = 0;
		input.GetState(G_KEY_F1, valF1);
		if (valF1 > 0)
		{
			LevelChanger();
		}
		if (true)
		{
			math.Create();
			math.IdentityF(mCameraCopy);
			math.InverseF(mCamera, mCameraCopy);

			const float cameraSpeed = 0.32f;
			const float fastSpeed = 0.64f;
			float moveXAmount = 0;
			float moveYAmount = 0;
			float moveZAmount = 0;
			float valSpace = 0;
			float valLeftShift = 0;
			float valW = 0;
			float valS = 0;
			float valD = 0;
			float valA = 0;
			float valR = 0;
			float valMouseXDelta = 0;
			float valMouseYDelta = 0;
			input.GetState(G_KEY_SPACE, valSpace);
			input.GetState(G_KEY_LEFTSHIFT, valLeftShift);
			input.GetState(G_KEY_W, valW);
			input.GetState(G_KEY_D, valD);
			input.GetState(G_KEY_A, valA);
			input.GetState(G_KEY_S, valS);
			input.GetState(G_KEY_R, valR);
			auto mouseDeltaResult = input.GetMouseDelta(valMouseXDelta, valMouseYDelta);


			// foward backwards side to side
			float frameSpeed = 0;
			if (valR == 0)
				frameSpeed = cameraSpeed * secondsPassed;
			else frameSpeed = fastSpeed * secondsPassed; // pressing r makes the camera fly around faster

			// up and down
			moveYAmount = valSpace - valLeftShift;
			mCameraCopy.row4.y += moveYAmount * frameSpeed;//* secondsPassed;


			moveZAmount = valW - valS;
			moveXAmount = valD - valA;
			GW::MATH::GVECTORF translateXZ = { moveXAmount * frameSpeed, 0, moveZAmount * frameSpeed };
			GW::MATH::GMATRIXF translateMatix;
			math.IdentityF(translateMatix);
			math.TranslateLocalF(translateMatix, translateXZ, translateMatix);
			math.MultiplyMatrixF(translateMatix, mCameraCopy, mCameraCopy);

			if (G_PASS(mouseDeltaResult) && mouseDeltaResult != GW::GReturn::REDUNDANT)
			{
				// rotating code goes in here
				unsigned int ScreenHeight, ScreenWidth;
				manager.GetDimensions(win, ScreenWidth, ScreenHeight);
				// tilt camera up and down
				float thumbSpeed = G_PI * secondsPassed;
				float totalPitch = FOV * valMouseYDelta / ScreenHeight;
				GW::MATH::GMATRIXF mPitch;
				math.IdentityF(mPitch);
				math.RotateXLocalF(mPitch, (totalPitch), mPitch);
				math.MultiplyMatrixF(mPitch, mCameraCopy, mCameraCopy);

				// turn left and right
				float totalYaw = FOV * AspectRatio * valMouseXDelta / ScreenWidth;
				GW::MATH::GMATRIXF mYaw;
				math.IdentityF(mYaw);
				math.RotateYLocalF(mYaw, (totalYaw), mYaw);
				GW::MATH::GVECTORF SavedPosition = mCameraCopy.row4.xyzw();
				math.MultiplyMatrixF(mCameraCopy, mYaw, mCameraCopy);
				mCameraCopy.row4.x = SavedPosition.x;
				mCameraCopy.row4.y = SavedPosition.y;
				mCameraCopy.row4.z = SavedPosition.z;
				mCameraCopy.row4.w = SavedPosition.w;
			}

			math.InverseF(mCameraCopy, mCamera);
		}
	}
private:
	void OpenFileDialog()
	{
		if (MessageBox(NULL, L"Would you like to load a new level?", L"Hot-Swap Level", MB_YESNO) == IDYES)
		{
			// If they say yes to loading a new level
			// common dialog box structure, setting all fields to 0 is important
			OPENFILENAME ofn = { 0 };
			TCHAR szFile[260] = { 0 };
			// Initialize remaining fields of OPENFILENAME structure
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = NULL;
			ofn.lpstrFile = szFile;
			ofn.nMaxFile = sizeof(szFile);
			ofn.lpstrFilter = TEXT("All\0*.*\0Text\0*.TXT\0");
			ofn.nFilterIndex = 1;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			ofn.lpstrInitialDir = NULL;
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
			std::cout << "----------------- Level Loader has been hit -----------------\n\n" << std::endl;
			if (GetOpenFileName(&ofn) == TRUE)
			{
				std::cout << "Retrieved Level File from open file (OpenFileDialog() -> renderer.h)" << std::endl;
				// use ofn.lpstrFile here
				std::wstring filePath = szFile;
				//setup converter
				using convert_type = std::codecvt_utf8<wchar_t>;
				std::wstring_convert<convert_type, wchar_t> converter;

				//use converter (.to_bytes: wstr->str, .from_bytes: str->wstr)
				std::string converted_str = converter.to_bytes(filePath);
				OpenLevel(converted_str);
			}
		}
	}
	void OpenLevel(string filePath)
	{
		ifstream inputFile;
		string pathtoMatrixData = exetension + "MatrixTextFiles/";
		string pathtoH2BFolder = exetension + "H2B/";
		string levelName;
		inputFile.open(filePath.c_str());
		std::cout << "Attempting to open " + filePath + " (OpenLevel()->renderer.h)" << std::endl;
		if (inputFile.is_open())
		{
			std::cout << "Successfully opened " + filePath + " (OpenLevel()->renderer.h)" << std::endl;
			while (!inputFile.eof())
			{
				char buffer[256];
				inputFile.getline(buffer, 256);
				if (buffer[0] == '{')
				{
					int i = 1;
					for (i; i < sizeof(buffer); ++i)
					{
						if (buffer[i] == ':') break;
						else
							levelName = levelName + buffer[i];
					}
					++i;
					for (i; i < sizeof(buffer); ++i)
					{
						if (buffer[i] == ':')
						{
							pathtoMatrixData = pathtoMatrixData + ".txt";
							break;
						}
						else
							pathtoMatrixData = pathtoMatrixData + buffer[i];
					}
					++i;
					for (i; i < sizeof(buffer); ++i)
					{
						if (buffer[i] == '}')
						{
							pathtoH2BFolder = pathtoH2BFolder + "/";
							break;
						}
						else
							pathtoH2BFolder = pathtoH2BFolder + buffer[i];
					}
				}
			}
			inputFile.close();
			std::cout << "Closed " + filePath + " (OpenLevel()->renderer.h)" << std::endl;
			cout << pathtoMatrixData + "\n" + pathtoH2BFolder << std::endl;
			std::cout << "\n\n----------------- Starting to Load Level -----------------" << std::endl;
			if (LoadLevel(pathtoMatrixData, pathtoH2BFolder, activeLevel, mProjection))
			{
				std::cout << "----------------- Level Loaded (Model Count: ";
				std::cout << activeLevel.size(); 
				std::cout << ")------------------ " << std::endl;
				std::cout << "\n\n----------------- Setup Level Renderer Start -----------------" << std::endl;
				LevelRenderSetup(activeLevel);
				std::cout << "----------------- Setup Level Renderer Complete -----------------\n" << std::endl;
				isLevelLoaded = true;
				std::cout << "\n----------------- " + levelName + " Level Loaded and Rendered -----------------" << std::endl;
			}
			else {
				std::cout << "Failed to load level.\nCheck file paths relatated to level " + levelName + "." << std::endl;
			}

		}
		else std::cout << "Failed to open " + filePath + " (OpenLevel()->renderer.h)" << std::endl;
	}
	void CleanUpLevel()
	{
		for (int i = 0; i < activeLevel.size(); ++i)
		{
			activeLevel[i].CleanUp();
		}
	}
};
