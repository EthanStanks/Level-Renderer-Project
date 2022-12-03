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

struct Mesh
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
	
	vector<Mesh> meshes;
	vector<Light> lights;
	vector<Camera> cameras;

	LoadLevelData()
	{
		meshes = vector<Mesh>();
		lights = vector<Light>();
		cameras = vector<Camera>();
	}
	void LoadData(string fileName)
	{
		ReadFile(fileName);
	}
	
private:

	void PrintMatrix(Matrix _printMe)
	{
		cout << "<Matrix 4x4 (" << _printMe.row1[0] << ", " << _printMe.row1[1] << ", " << _printMe.row1[2] << ", " << _printMe.row1[3] << ")" << endl;
		cout << "            (" << _printMe.row2[0] << ", " << _printMe.row2[1] << ", " << _printMe.row2[2] << ", " << _printMe.row2[3] << ")" << endl;
		cout << "            (" << _printMe.row3[0] << ", " << _printMe.row3[1] << ", " << _printMe.row3[2] << ", " << _printMe.row3[3] << ")" << endl;
		cout << "            (" << _printMe.row4[0] << ", " << _printMe.row4[1] << ", " << _printMe.row4[2] << ", " << _printMe.row4[3] << ")" << endl;
	}
	void PrintFile(vector<Mesh> _meshes, vector<Light> _lights, vector<Camera> _cameras)
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
	void ReadFile(string fileName)
	{
		ifstream inputFile;
		//inputFile = ifstream(fileName);
		inputFile.open(fileName.c_str()); // tries to open the file

		if (inputFile.is_open()) // check to see if the file is open
		{
			while (!inputFile.eof()) // while the file hasn't reach the end
			{
				char buffer[256]; // container for the info being read in
				inputFile.getline(buffer, 256); // reads 256 bytes or until '/n'

				if (buffer[0] != '#') // is not a comment line
				{
					if (buffer[0] == 'M') // MESH
					{
						Mesh mesh;
						inputFile.getline(buffer, 256); // read the name of the mesh

						mesh.meshName = buffer; // takes the char and sets them to the name
						mesh.meshName.erase(remove_if(mesh.meshName.begin(), mesh.meshName.end(), isspace), mesh.meshName.end()); // remove unwanted white space in the name
						char bufferMatrixLine1[256];
						char bufferMatrixLine2[256];
						char bufferMatrixLine3[256];
						char bufferMatrixLine4[256];
						inputFile.getline(bufferMatrixLine1, 256); // read in the first matrix line
						inputFile.getline(bufferMatrixLine2, 256); // read in the second matrix line
						inputFile.getline(bufferMatrixLine3, 256); // read in the third matrix line
						inputFile.getline(bufferMatrixLine4, 256); // read in the fourth matrix line
						sscanf_s(bufferMatrixLine1, "<Matrix 4x4 (%f, %f,  %f, %f)", &mesh.meshMatrix.row1[0], &mesh.meshMatrix.row1[1], &mesh.meshMatrix.row1[2], &mesh.meshMatrix.row1[3]); // takes in each float from the line and stores them
						sscanf_s(bufferMatrixLine2, "            (%f, %f, %f, %f)", &mesh.meshMatrix.row2[0], &mesh.meshMatrix.row2[1], &mesh.meshMatrix.row2[2], &mesh.meshMatrix.row2[3]); // takes in each float from the line and stores them
						sscanf_s(bufferMatrixLine3, "            (%f, %f,  %f, %f)", &mesh.meshMatrix.row3[0], &mesh.meshMatrix.row3[1], &mesh.meshMatrix.row3[2], &mesh.meshMatrix.row3[3]); // takes in each float from the line and stores them
						sscanf_s(bufferMatrixLine4, "            (%f, %f,  %f, %f)>", &mesh.meshMatrix.row4[0], &mesh.meshMatrix.row4[1], &mesh.meshMatrix.row4[2], &mesh.meshMatrix.row4[3]); // takes in each float from the line and stores them
						meshes.push_back(mesh);
					}
					else if (buffer[0] == 'L') // LIGHT
					{
						Light light;
						inputFile.getline(buffer, 256); // read the name of the light
						light.lightName = buffer; // takes the char and sets them to the name
						light.lightName.erase(remove_if(light.lightName.begin(), light.lightName.end(), isspace), light.lightName.end()); // remove unwanted white space in the name

						char bufferMatrixLine1[256];
						char bufferMatrixLine2[256];
						char bufferMatrixLine3[256];
						char bufferMatrixLine4[256];
						inputFile.getline(bufferMatrixLine1, 256); // read in the first matrix line
						inputFile.getline(bufferMatrixLine2, 256); // read in the second matrix line
						inputFile.getline(bufferMatrixLine3, 256); // read in the third matrix line
						inputFile.getline(bufferMatrixLine4, 256); // read in the fourth matrix line
						sscanf_s(bufferMatrixLine1, "<Matrix 4x4 (%f, %f,  %f, %f)", &light.lightMatrix.row1[0], &light.lightMatrix.row1[1], &light.lightMatrix.row1[2], &light.lightMatrix.row1[3]); // takes in each float from the line and stores them
						sscanf_s(bufferMatrixLine2, "            (%f, %f, %f, %f)", &light.lightMatrix.row2[0], &light.lightMatrix.row2[1], &light.lightMatrix.row2[2], &light.lightMatrix.row2[3]); // takes in each float from the line and stores them
						sscanf_s(bufferMatrixLine3, "            (%f, %f,  %f, %f)", &light.lightMatrix.row3[0], &light.lightMatrix.row3[1], &light.lightMatrix.row3[2], &light.lightMatrix.row3[3]); // takes in each float from the line and stores them
						sscanf_s(bufferMatrixLine4, "            (%f, %f,  %f, %f)>", &light.lightMatrix.row4[0], &light.lightMatrix.row4[1], &light.lightMatrix.row4[2], &light.lightMatrix.row4[3]); // takes in each float from the line and stores them
						lights.push_back(light);
					}
					else if (buffer[0] == 'C') // CAMERA
					{
						Camera camera;
						inputFile.getline(buffer, 256); // read the name of the camera
						camera.cameraName = buffer; // takes the char and sets them to the name
						camera.cameraName.erase(remove_if(camera.cameraName.begin(), camera.cameraName.end(), isspace), camera.cameraName.end()); // remove unwanted white space in the name
						char bufferMatrixLine1[256];
						char bufferMatrixLine2[256];
						char bufferMatrixLine3[256];
						char bufferMatrixLine4[256];
						inputFile.getline(bufferMatrixLine1, 256); // read in the first matrix line
						inputFile.getline(bufferMatrixLine2, 256); // read in the second matrix line
						inputFile.getline(bufferMatrixLine3, 256); // read in the third matrix line
						inputFile.getline(bufferMatrixLine4, 256); // read in the fourth matrix line
						sscanf_s(bufferMatrixLine1, "<Matrix 4x4 (%f, %f,  %f, %f)", &camera.cameraMatrix.row1[0], &camera.cameraMatrix.row1[1], &camera.cameraMatrix.row1[2], &camera.cameraMatrix.row1[3]); // takes in each float from the line and stores them
						sscanf_s(bufferMatrixLine2, "            (%f, %f, %f, %f)", &camera.cameraMatrix.row2[0], &camera.cameraMatrix.row2[1], &camera.cameraMatrix.row2[2], &camera.cameraMatrix.row2[3]); // takes in each float from the line and stores them
						sscanf_s(bufferMatrixLine3, "            (%f, %f,  %f, %f)", &camera.cameraMatrix.row3[0], &camera.cameraMatrix.row3[1], &camera.cameraMatrix.row3[2], &camera.cameraMatrix.row3[3]); // takes in each float from the line and stores them
						sscanf_s(bufferMatrixLine4, "            (%f, %f,  %f, %f)>", &camera.cameraMatrix.row4[0], &camera.cameraMatrix.row4[1], &camera.cameraMatrix.row4[2], &camera.cameraMatrix.row4[3]); // takes in each float from the line and stores them
						cameras.push_back(camera);
					}
				}
			}
			inputFile.close();
			PrintFile(meshes, lights, cameras);
		}
	}
};
