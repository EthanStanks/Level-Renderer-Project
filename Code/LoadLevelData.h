#pragma once
// includes
#include <iostream>
#include <fstream>
#include <vector>
#include <cctype>

using namespace std;

struct Matrix
{
	float row1[4];
	float row2[4];
	float row3[4];
	float row4[4];
};

struct TempMesh
{
	Matrix meshMatrix;
	string meshName;
};

struct Light
{
	Matrix lightMatrix;
	string lightName;
};

struct Camera
{
	Matrix cameraMatrix;
	string cameraName;
};


class LoadLevelData
{
public:
	
	vector<TempMesh> meshes;
	vector<Light> lights;
	vector<Camera> cameras;

	LoadLevelData()
	{
		meshes =  vector<TempMesh>();
		lights =  vector<Light>();
		cameras =  vector<Camera>();
	}
	void LoadData(string fileName, bool isPrintToConsole)
	{
		ReadFile(fileName, isPrintToConsole);
	}
	~LoadLevelData()
	{
		ClearData();
	}
private:
	void ClearData()
	{
		vector<TempMesh> temp1;
		meshes.clear();
		meshes = temp1;

		vector<Light> temp2;
		lights.clear();
		lights = temp2;

		vector<Camera> temp3;
		cameras.clear();
		cameras = temp3;

	}
	void PrintMatrix(Matrix _printMe)
	{
		cout << "<Matrix 4x4 (" << _printMe.row1[0] << ", " << _printMe.row1[1] << ", " << _printMe.row1[2] << ", " << _printMe.row1[3] << ")" << endl;
		cout << "            (" << _printMe.row2[0] << ", " << _printMe.row2[1] << ", " << _printMe.row2[2] << ", " << _printMe.row2[3] << ")" << endl;
		cout << "            (" << _printMe.row3[0] << ", " << _printMe.row3[1] << ", " << _printMe.row3[2] << ", " << _printMe.row3[3] << ")" << endl;
		cout << "            (" << _printMe.row4[0] << ", " << _printMe.row4[1] << ", " << _printMe.row4[2] << ", " << _printMe.row4[3] << ")" << endl;
	}
	void PrintFile(vector<TempMesh> _meshes, vector<Light> _lights, vector<Camera> _cameras)
	{
		for (int i = 0; i < _cameras.size(); ++i)
		{
			cout << "CAMERA" << endl;
			cout << _cameras[i].cameraName << endl;
			PrintMatrix(_cameras[i].cameraMatrix);
		}
		for (int i = 0; i < _lights.size(); ++i)
		{
			cout << "LIGHT" << endl;
			cout << _lights[i].lightName << endl;
			PrintMatrix(_lights[i].lightMatrix);
		}
		for (int i = 0; i < _meshes.size(); ++i)
		{
			cout << "MESH" << endl;
			cout << _meshes[i].meshName << endl;
			PrintMatrix(_meshes[i].meshMatrix);
		}
	}
	void RetrieveName(string& _tempName, bool isMesh)
	{
		if (isMesh)
		{
			bool hitEnd = false;
			for (int i = 0; i < _tempName.size(); ++i)
			{
				if (_tempName[i] == '.' || hitEnd)
				{
					hitEnd = true;
					_tempName[i] = ' ';
				}
			}
			_tempName.erase(remove_if(_tempName.begin(), _tempName.end(), isspace), _tempName.end()); // remove unwanted white space in the name
			_tempName = _tempName + ".h2b";
		}
	}
	void RetrieveMatrix(Matrix& _tempMatrix, ifstream& _inputFile)
	{
		char bufferMatrixLine1[256];
		char bufferMatrixLine2[256];
		char bufferMatrixLine3[256];
		char bufferMatrixLine4[256];
		_inputFile.getline(bufferMatrixLine1, 256); // read in the first matrix line
		_inputFile.getline(bufferMatrixLine2, 256); // read in the second matrix line
		_inputFile.getline(bufferMatrixLine3, 256); // read in the third matrix line
		_inputFile.getline(bufferMatrixLine4, 256); // read in the fourth matrix line
		sscanf_s(bufferMatrixLine1, "<Matrix 4x4 (%f, %f,  %f, %f)", &_tempMatrix.row1[0], &_tempMatrix.row1[1], &_tempMatrix.row1[2], &_tempMatrix.row1[3]); // takes in each float from the line and stores them
		sscanf_s(bufferMatrixLine2, "            (%f, %f, %f, %f)", &_tempMatrix.row2[0], &_tempMatrix.row2[1], &_tempMatrix.row2[2], &_tempMatrix.row2[3]); // takes in each float from the line and stores them
		sscanf_s(bufferMatrixLine3, "            (%f, %f,  %f, %f)", &_tempMatrix.row3[0], &_tempMatrix.row3[1], &_tempMatrix.row3[2], &_tempMatrix.row3[3]); // takes in each float from the line and stores them
		sscanf_s(bufferMatrixLine4, "            (%f, %f,  %f, %f)>", &_tempMatrix.row4[0], &_tempMatrix.row4[1], &_tempMatrix.row4[2], &_tempMatrix.row4[3]); // takes in each float from the line and stores them
	}
	void ReadFile(string fileName, bool isPrintToConsole)
	{
		ifstream inputFile;
		//inputFile = ifstream(fileName);
		inputFile.open(fileName.c_str()); // tries to open the file
		std::cout << "Attempting to open " + fileName + " (ReadFile()->LoadLevelData.h)" << std::endl;
		if (inputFile.is_open()) // check to see if the file is open
		{
			std::cout << "Sucessfully opened " + fileName + " (ReadFile()->LoadLevelData.h)" << std::endl;
			while (!inputFile.eof()) // while the file hasn't reach the end
			{
				char buffer[256]; // container for the info being read in
				inputFile.getline(buffer, 256); // reads 256 bytes or until '/n'

				if (buffer[0] != '#') // is not a comment line
				{
					if (buffer[0] == 'M') // MESH
					{
						TempMesh mesh;
						inputFile.getline(buffer, 256); // read the name of the mesh
						mesh.meshName = buffer;
						RetrieveName(mesh.meshName, true); // takes the temp string and sets them to the name
						RetrieveMatrix(mesh.meshMatrix, inputFile);
						meshes.push_back(mesh);
					}
					else if (buffer[0] == 'L') // LIGHT
					{
						Light light;
						inputFile.getline(buffer, 256); // read the name of the light
						light.lightName = buffer;
						RetrieveName(light.lightName, false); // takes the temp string and sets them to the name
						RetrieveMatrix(light.lightMatrix, inputFile);
						lights.push_back(light);
					}
					else if (buffer[0] == 'C') // CAMERA
					{
						Camera camera;
						inputFile.getline(buffer, 256); // read the name of the camera
						camera.cameraName = buffer;
						RetrieveName(camera.cameraName, false); // takes the temp string and sets them to the name
						RetrieveMatrix(camera.cameraMatrix, inputFile);
						cameras.push_back(camera);
					}
				}
			}
			inputFile.close();
			std::cout << "Closed " + fileName + " (ReadFile()->LoadLevelData.h)" << std::endl;
			if(isPrintToConsole) PrintFile(meshes, lights, cameras);
		}
		else std::cout << "Failed to open " + fileName + " (ReadFile()->LoadLevelData.h)" << std::endl;
	}
};