<h1>Dev4 Vulkan Renderer Project</h1>
This is a graphics renderer project created in Vulkan with an Object Oriented approach to Data Driven Rendering. The main goal of this project is to create software designed to draw the visual aspects of a game level using the Vulkan API.

<h3>Getting Started</h3>
To get started, clone the repository and open the project in your preferred IDE. You will also need to have Vulkan installed on your system.

Prerequisites
<ul>
<li>Vulkan SDK</li>
<li>C++ Compiler</li>
</ul>
<h3>Running the Application</h3>
To run the application, build the project and run the executable file. The renderer can only render in debug mode currently.

<h3>Controls</h3>
<ul>
<li>(W): Forwards</li>
<li>(S): Backwards</li>
<li>(A): Left</li>
<li>(D): Right</li>
<li>(Space): Up</li>
<li>(Left_Shift): Down</li>
<li>(Mouse): Tilts up and down / Turns left and right</li>
<li>(R): Speeds Up Camera Speed</li>
<li>(F1): Load saved levels</li>
</ul>
<h3>Loading a Level</h3>
<ul>
<li>Press F1</li>
<li>Select a Level.txt file from the SavedLevels folder</li>
</ul>
<h3>Project Features</h3>
<ul>
<li>One or More Blender Game Levels to export from (.blend)</li>
<li>Exporting OBJ Names & Locations from your Levels (GameLevel.txt)</li>
<li>Run-time file I/O parsing of exported level information (GameLevel.txt)</li>
<li>Reading binary model data for all referenced models (*.h2b + h2bParser.h)</li>
<li>Transfering model geometry, material and matrix data to GPU</li>
<li>One model imported from GameLevel.txt is drawing correctly</li>
<li>Working 3D Fly-through Camera</li>
<li>All models successfully drawn at proper location, orientation & scale</li>
<li>All models have correct OBJ material colors showing</li>
<li>Directional light with ambient term and specular highlights</li>
<li>Support for Hot-Swapping multiple levels without a restart</li>
</ul>
<h3>Credits</h3>
LowPoly Models by @Quaternius
https://quaternius.com/

<h3>Additional Information</h3>
This project was created as a part of Project & Portfolio IV. The primary goal of this project is to apply what you have learned so far about computer graphics to create a renderer using Vulkan. The project is evaluated based on the rubric provided, and completing it will demonstrate your understanding of computer graphics and your ability to create a game level. 
