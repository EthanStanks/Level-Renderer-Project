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
		modelStoage.clear();
		LoadLevelData matrixLoader;
		matrixLoader.LoadData(pathtoGameLevel, false); // loads text file
		int meshCount = matrixLoader.meshes.size();
		for (int i = 0; i < meshCount; ++i) // for each mesh in the text file
		{
			Model model; // create temp model
			bool isLoaded = model.LoadModel(pathToH2BFiles + matrixLoader.meshes[i].meshName, vlk, win, device); // load the .h2b file of that model
			if (isLoaded) // if opening the model worked
			{
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
			modelStorage[i].ModelRenderSetup();
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

		std::cout << "Press F1 to Load a Level" << std::endl;

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
	// hot swapping multiple levels without restart
	void NewLevel()
	{
		string pathtoMatrixData;
		string pathtoH2BFolder;
		string levelName;
		while (true)
		{
			std::cout << "Enter the name of the matrix text file without the extension. Ex.'GameLevel'" << std::endl;
			string pathTextFile = "../";
			string input;
			std::cin >> input;
			pathTextFile = pathTextFile + input + ".txt";
			std::cout << "Does this file path look correct? ->\t" << pathTextFile << std::endl;
			std::cout << "a) Yes\tb) No" << std::endl;
			char charInput;
			std::cin >> charInput;
			if (charInput == 'a' || charInput == 'A')
			{
				pathtoMatrixData = input;
				break;
			}
			else if (charInput == 'b' || charInput == 'B')
			{
				system("CLS");
			}
			else cout << "\nInput is incorrect...\nAccepted input is 'a'/'A' or 'b'/'B'\n\n";
		}
		system("CLS");
		while (true)
		{
			std::cout << "Enter the name of the folder that stores this level's .H2B files. Ex.'MainBuilding'" << std::endl;
			string pathH2BFolder = "../H2B/";
			string input;
			std::cin >> input;
			pathH2BFolder = pathH2BFolder + input + "/";
			std::cout << "Does this file path look correct? ->\t" << pathH2BFolder << std::endl;
			std::cout << "a) Yes\tb) No" << std::endl;
			char charInput;
			cin >> charInput;
			if (charInput == 'a' || charInput == 'A')
			{
				pathtoH2BFolder = input;
				break;
			}
			else if (charInput == 'b' || charInput == 'B')
			{
				system("CLS");
			}
			else cout << "\nInput is incorrect...\nAccepted input is 'a'/'A' or 'b'/'B'\n\n";
		}
		system("CLS");
		while (true)
		{
			std::cout << "Enter a name for this level (No Spaces). Ex.'MyMainLevel'" << std::endl;
			string input;
			std::cin >> input;
			std::cout << "Is this the correct name you'd like to name this level? ->\t" << input << std::endl;
			std::cout << "a) Yes\tb) No" << std::endl;
			char charInput;
			cin >> charInput;
			if (charInput == 'a' || charInput == 'A')
			{
				levelName = input;
				break;
			}
			else if (charInput == 'b' || charInput == 'B')
			{
				system("CLS");
			}
			else cout << "\nInput is incorrect...\nAccepted input is 'a'/'A' or 'b'/'B'\n\n";
		}
		std::ofstream writer;
		writer.open("../SavedLevel.txt", ios::app);
		if (writer)
		{
			string writeLine = "\n{" + levelName + ":" + pathtoMatrixData + ":" + pathtoH2BFolder + "}";
			writer << writeLine;
			system("CLS");
			cout << "Succesfully saved Level." << std::endl;
			writer.close();
		}
		else std::cout << "Error opening: ../SavedLevel.txt";
		pathtoMatrixData = "../" + pathtoMatrixData + ".txt";
		pathtoH2BFolder = "../H2B/" + pathtoH2BFolder + "/";
		if (LoadLevel(pathtoMatrixData, pathtoH2BFolder, activeLevel, mProjection))
		{
			LevelRenderSetup(activeLevel);
			isLevelLoaded = true;
			cout <<"\n" + levelName + " has been successfully loaded." << std::endl;
		}
		else std::cout << "Failed to load new level.\nPress F1 to Load a Level" << std::endl;
	}
	void SavedLevels()
	{
		vector<SavedLevel> levelOptions;
		ifstream inputFile;
		inputFile.open("../SavedLevel.txt");
		if (inputFile.is_open())
		{
			while (!inputFile.eof())
			{
				char buffer[256];
				inputFile.getline(buffer, 256);
				if (buffer[0] == '{')
				{
					SavedLevel level;
					int i = 1;
					for (i; i < sizeof(buffer); ++i)
					{
						if (buffer[i] == ':') break;
						else
							level.levelName = level.levelName + buffer[i];
					}
					++i;
					for (i; i < sizeof(buffer); ++i)
					{
						if (buffer[i] == ':')
						{
							level.pathtoMatrixData = level.pathtoMatrixData + ".txt";
							break;
						}
						else
							level.pathtoMatrixData = level.pathtoMatrixData + buffer[i];
					}
					++i;
					for (i; i < sizeof(buffer); ++i)
					{
						if (buffer[i] == '}')
						{
							level.pathtoH2BFolder = level.pathtoH2BFolder + "/";
							break;
						}
						else
							level.pathtoH2BFolder = level.pathtoH2BFolder + buffer[i];
					}
					levelOptions.push_back(level);
				}
			}
			inputFile.close();
			if (levelOptions.size() == 0) {
				system("CLS");
				std::cout << "There are currently no saved levels." << std::endl;
				std::cout << "Press F1 to Load a Level" << std::endl;
			}
			else {
				system("CLS");
				char userInput;
				while (true)
				{
					std::cout << "Select a saved level to load:" << std::endl;
					int levelCount = levelOptions.size();
					char lastChar;
					for (int i = 0; i < levelCount; ++i)
					{
						lastChar = char(i + 97);
						std::cout << lastChar << ") " + levelOptions[i].levelName << std::endl;
					}
					std::cout << "\n";
					char input;
					std::cin >> input;
					if (int(input) < 97 || int(input) > int(lastChar)) {
						system("CLS");
						if (levelCount > 1)
						{
							std::cout << "Incorrect input. Enter a value between 'a-" + lastChar << "'." <<std::endl;
						}
						else if (levelCount == 1) std::cout << "Your only option is 'a'.";
					}
					else
					{
						userInput = input;
						break;
					}
				}
				int loadIndex = (int)userInput - 97;
				if (LoadLevel(levelOptions[loadIndex].pathtoMatrixData, levelOptions[loadIndex].pathtoH2BFolder, activeLevel, mProjection))
				{
					LevelRenderSetup(activeLevel);
					isLevelLoaded = true;
					cout << "\n" + levelOptions[loadIndex].levelName + " has been successfully loaded." << std::endl;
				}
				else {
					std::cout << "Failed to load level.\nCheck file paths relatated to level " + levelOptions[loadIndex].levelName + "." << std::endl;
				}
			}
		}
		else
		{
			std::cout << "Could not open: ../SavedLevel.txt\nIf no levels are saved create a new one!" << std::endl;
			return false;
		}

		return true;
	}
	void LevelChanger()
	{
		system("CLS");
		isLevelLoaded = false;
		char loadInput;
		while (true)
		{
			std::cout << "Would you like to LOAD a:\na) NEW LEVEL\tb) SAVED LEVEL" << std::endl;
			char input;
			std::cin >> input;
			if (input == 'a' || input == 'A' || input == 'b' || input == 'B')
			{
				loadInput = input;
				break;
			}
			else
			{
				std::cout << "\nInput is incorrect...\nAccepted input is 'a'/'A' or 'b'/'B'\n\n";
			}
		}
		system("CLS");
		if (loadInput == 'a' || loadInput == 'A')
		{
			NewLevel();
		}
		else if (loadInput == 'b' || loadInput == 'B')
		{
			SavedLevels();
		}

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
		if (false)
		{


			math.Create();
			math.IdentityF(mCameraCopy);
			math.InverseF(mCamera, mCameraCopy);

			const float cameraSpeed = 0.03f;
			const float fastSpeed = 0.08f;
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
			input.GetMouseDelta(valMouseXDelta, valMouseYDelta);

			// up and down
			moveYAmount = valSpace - valLeftShift;
			mCameraCopy.row4.y += moveYAmount * cameraSpeed * secondsPassed;

			// foward backwards side to side
			float frameSpeed = 0;
			if (valR == 0)
				frameSpeed = cameraSpeed * secondsPassed;
			else frameSpeed = fastSpeed * secondsPassed; // pressing r makes the camera fly around faster

			moveZAmount = valW - valS;
			moveXAmount = valD - valA;
			GW::MATH::GVECTORF translateXZ = { moveXAmount * frameSpeed, 0, moveZAmount * frameSpeed };
			GW::MATH::GMATRIXF translateMatix;
			math.IdentityF(translateMatix);
			math.TranslateLocalF(translateMatix, translateXZ, translateMatix);
			math.MultiplyMatrixF(translateMatix, mCameraCopy, mCameraCopy);

			unsigned int ScreenHeight, ScreenWidth;
			manager.GetDimensions(win, ScreenWidth, ScreenHeight);
			// tilt camera up and down
			float thumbSpeed = G_PI * secondsPassed;
			float totalPitch = FOV * valMouseYDelta / ScreenHeight * (-1 * thumbSpeed);
			GW::MATH::GMATRIXF mPitch;
			math.IdentityF(mPitch);
			math.RotateXLocalF(mPitch, G_DEGREE_TO_RADIAN(totalPitch), mPitch);
			math.MultiplyMatrixF(mPitch, mCameraCopy, mCameraCopy);

			// turn left and right
			float totalYaw = FOV * AspectRatio * valMouseXDelta / ScreenWidth * thumbSpeed;
			GW::MATH::GMATRIXF mYaw;
			math.IdentityF(mYaw);
			math.RotateYLocalF(mYaw, G_DEGREE_TO_RADIAN(totalYaw), mYaw);
			GW::MATH::GVECTORF SavedPosition = mCameraCopy.row4.xyzw();
			math.MultiplyMatrixF(mCameraCopy, mYaw, mCameraCopy);
			mCameraCopy.row4.x = SavedPosition.x;
			mCameraCopy.row4.y = SavedPosition.y;
			mCameraCopy.row4.z = SavedPosition.z;
			mCameraCopy.row4.w = SavedPosition.w;

			math.InverseF(mCameraCopy, mCamera);
		}
	}
private:
	void CleanUpLevel()
	{
		for (int i = 0; i < activeLevel.size(); ++i)
		{
			activeLevel[i].CleanUp();
		}
	}
};
