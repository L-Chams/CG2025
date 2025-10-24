#include <iostream>
#include <fstream>
#include <string>
#include <DrawingWindow.h>
#include <ModelTriangle.h>
#include <vector>
#include <map>
#include <Utils.h>
#include <CanvasPoint.h>
#include <CanvasTriangle.h>
#include <Colour.h>
#include <TextureMap.h>
#include <glm/glm.hpp>

#define WIDTH 320
#define HEIGHT 240

//create 2d array with demensions widthxheight
float depthBuffer[WIDTH][HEIGHT];

//initialise all values to 0
void initializeDepthBuffer(){

    for (int i = 0; i < WIDTH; i++)
    {
        for (int j = 0; j < HEIGHT; j++)
        {
            depthBuffer[i][j] = 0.0;
        }
    }
}

// return a vector of ModelTriangles from an .obj file
std::vector<ModelTriangle> processOBJFile(const std::string &filename, const std::map<std::string, Colour> &colourMap){

    std::ifstream inputFile(filename);
    if (!inputFile.is_open())
    {
        std::cerr << "Error opening file!" << std::endl;
        return {};
    }

    std::vector<ModelTriangle> triangles;
    std::vector<glm::vec3> verticesList;
    std::vector<glm::vec3> faceVertices;
    std::vector<std::string> colours;
    std::string colourName;

    std::string line;
    while (std::getline(inputFile, line))
    {
        char c = line[0];
        char delimiter = ' ';

        if (c == 'u')
        {
             // usemtl for colour
            std::vector<std::string> linesplit = split(line, delimiter);
            colourName = linesplit[1];
            //colours.push_back(colourName);

        }
        else if (c == 'v'){

            glm::vec3 vertices;
            // split line by delimiter
            std::vector<std::string> linesplit = split(line, delimiter);

            // add the three vertices to the glm::vec3
            vertices.x = 0.35*std::stof(linesplit[1]);
            vertices.y = 0.35*std::stof(linesplit[2]);
            vertices.z = 0.35*std::stof(linesplit[3]);
            verticesList.push_back(vertices);
            // v for vertices
            // loop through each line, check first letter of each line to see if v or f
        }
        else if (c == 'f'){

            // split line by delimiter
            std::vector<std::string> linesplit = split(line, delimiter);

            glm::vec3 face;
            face.x = std::stof(linesplit[1].substr(0, linesplit[1].find('/')));
            face.y = std::stof(linesplit[2].substr(0, linesplit[2].find('/')));
            face.z = std::stof(linesplit[3].substr(0, linesplit[3].find('/')));

            faceVertices.push_back(face);
            colours.push_back(colourName);
        }
    }
    // loop face vertices and make triangles
    inputFile.close();

    for (int i = 0; i < faceVertices.size(); i++)
    {
        ModelTriangle triangle(
            verticesList[(faceVertices[i].x) - 1],
            verticesList[(faceVertices[i].y) - 1],
            verticesList[(faceVertices[i].z) - 1],
            //get colour from the hashmap
            colourMap.at(colours[i])
        );
        triangles.push_back(triangle);
    }
    return triangles;
}

// returns a hashmap of colours from a .mtl file
std::map<std::string, Colour> loadPalette(const std::string &filename){

    std::ifstream inputFile(filename);
    if (!inputFile.is_open())
    {
        std::cerr << "Error opening palette file!" << std::endl;
        return {};
    }

    std::map<std::string, Colour> colours;
    std::string line;

    while (std::getline(inputFile, line))
    {
        char delimiter = ' ';
        char c = line[0];
        Colour colour;
        if (c == 'n')
        {
            std::vector<std::string> linesplit = split(line, delimiter);
            colour.name = linesplit[1];

            getline(inputFile, line); // skip Kd line for now
            linesplit = split(line, delimiter);
            std::string name = linesplit[0];
            colour.red = std::stof(linesplit[1]) * 255;
            colour.green = std::stof(linesplit[2]) * 255;
            colour.blue = std::stof(linesplit[3]) * 255;
            colours[colour.name] = colour;
        }
    }
    inputFile.close();

    return colours;
}

std::vector<CanvasPoint> interpolate2Coords(CanvasPoint p1, CanvasPoint p2){

	std::vector<CanvasPoint> pointsBetween;
	float x_dist = p2.x - p1.x;
	float y_dist = p2.y - p1.y;
    float depth_dist = p2.depth - p1.depth;

	float numberOfSteps = fmax(abs(x_dist), abs(y_dist));
	float x_spacing = x_dist / numberOfSteps;
	float y_spacing = y_dist / numberOfSteps;
    float depth_spacing = depth_dist / numberOfSteps;

	for (int i = 0; i <= numberOfSteps; i++)
	{
		float x = p1.x + (x_spacing * i);
		float y = p1.y + (y_spacing * i);
        float depth = p1.depth + (depth_spacing * i);

        //std::cout << "Depth at step " << i << ": " << depth << std::endl;
		pointsBetween.push_back(CanvasPoint(x, y, depth));
	}
	return pointsBetween;
}

void drawLine(DrawingWindow &window, CanvasPoint from, CanvasPoint to, Colour colour_param){

    uint32_t colour = (255 << 24) + (int(colour_param.red) << 16) + (int(colour_param.green) << 8) + int(colour_param.blue);
    if (from.x == to.x && from.y == to.y)
    {
        window.setPixelColour(from.x, from.y, colour);
        return;
    }
    //std::cout << "From depth: " << from.depth << ", To depth: " << to.depth << std::endl;
    std::vector<CanvasPoint> pointsBetween = interpolate2Coords(from, to);
    for (int i = 0; i < pointsBetween.size(); i++){
        //check if depth is greater than current depth buffer value
        if (pointsBetween[i].depth > depthBuffer[int((pointsBetween[i].y))][int((pointsBetween[i].x))] ) {
            //update depth buffer
            depthBuffer[int((pointsBetween[i].y))][int((pointsBetween[i].x))]= pointsBetween[i].depth;
        } else {
            continue; //skip drawing this pixel
        }
        window.setPixelColour((pointsBetween[i].x), (pointsBetween[i].y), colour);
    }
}


void strokedTriangle(DrawingWindow &window, CanvasTriangle triangle, Colour colour){

    drawLine(window, triangle.v0(), triangle.v1(), colour);
    drawLine(window, triangle.v1(), triangle.v2(), colour);
    drawLine(window, triangle.v2(), triangle.v0(), colour);
}


// returns the 2D CanvasPoint postion at which the model vertex should be projected onto the image plane
CanvasPoint projectVertexOntoCanvasPoint(glm::vec3 cameraPostion, float focalLength, glm::vec3 vertexPosition, DrawingWindow &window){

    CanvasPoint pointOnImage;
    glm::vec3 relativePosition = cameraPostion - vertexPosition;
    float scale = 160.0f;

    // u = - f * (x/z) + W/2
    // v =   f * (y/z) + H/2
    pointOnImage.x = scale * (focalLength * (-relativePosition.x / relativePosition.z))  + WIDTH / 2;
    pointOnImage.y = scale * (focalLength * (relativePosition.y / relativePosition.z)) + HEIGHT / 2;
    pointOnImage.depth = 1/(relativePosition.z);

    return pointOnImage;
}

void renderPointCloud(DrawingWindow &window, const std::vector<ModelTriangle> triangles, glm::vec3 cameraPos, float focalLength)
{
    for (int i = 0; i < triangles.size(); i++)
    {
        // Project the 3D vertices onto the 2D canvas
        for (int j = 0; j < 3; j++)
        {
            CanvasPoint point = projectVertexOntoCanvasPoint(
                cameraPos,                // Camera position
                focalLength,              // Focal length
                triangles[i].vertices[j], // Vertex position
                window                    // Drawing window
            );
            // Draw the triangle on the canvas
            uint32_t colour = (255 << 24) + (255 << 16) + (255 << 8) + int(255);
            window.setPixelColour(point.x, point.y, colour);
        }
    }
}

void renderWireframe(DrawingWindow &window, const std::vector<ModelTriangle> triangles, glm::vec3 cameraPos, float focalLength){
    std::vector<CanvasTriangle> projectedTriangles;
    for (int i = 0; i < triangles.size(); i++)
    {
        // Project the 3D vertices onto the 2D canvas
        CanvasPoint projectedVertices[3];
        for (int j = 0; j < 3; j++)
        {
            projectedVertices[j] = projectVertexOntoCanvasPoint(
                cameraPos,                // Camera position
                focalLength,              // Focal length
                triangles[i].vertices[j], // Vertex position
                window                    // Drawing window
            );
        }

        // Draw the triangle on the canvas
        CanvasTriangle canvasTriangle(projectedVertices[0], projectedVertices[1], projectedVertices[2]);
        //need to see which triangles have the greater depth to draw first


        strokedTriangle(window, canvasTriangle, triangles[i].colour);
    }
}

void barycentricFillTriangle(DrawingWindow &window, CanvasTriangle triangle, Colour colour_param){
	CanvasPoint v0 = triangle.v0();
	CanvasPoint v1 = triangle.v1();
	CanvasPoint v2 = triangle.v2();
    //check that the triangle is within the window bounds

	glm::vec2 v0_vec2 (v0.x, v0.y);
	glm::vec2 v1_vec2 (v1.x, v1.y);
	glm::vec2 v2_vec2 (v2.x, v2.y);
	uint32_t colour = (255 << 24) + (int(colour_param.red) << 16) + (int(colour_param.green) << 8) + int(colour_param.blue);

	const int minX = std::min(std::min(v0.x, v1.x), v2.x);
	const int maxX = std::max(std::max(v0.x, v1.x), v2.x);
	const int minY = std::min(std::min(v0.y, v1.y), v2.y);
	const int maxY = std::max(std::max(v0.y, v1.y), v2.y);


	for (int y = minY; y <= maxY; y++)
	{

		for (int x = minX; x <= maxX; x++)
		{
           glm::vec3 coords = convertToBarycentricCoordinates(v0_vec2, v1_vec2, v2_vec2, glm::vec2(x, y));

            if (coords.x >= 0 && coords.y >= 0 && coords.z >= 0)
			{
                float depth = ((coords.x * v1.depth) + (coords.y * v2.depth) + (coords.z * v0.depth));
                if (y >=0 && y < HEIGHT && x >=0 && x < WIDTH && depth > depthBuffer[y][x])
                {
                    depthBuffer[y][x] = depth;
                    window.setPixelColour(x, y, colour);
                }
			}
		}
	}
}

void rasterisedRender(DrawingWindow &window, const std::vector<ModelTriangle> triangles, glm::vec3 cameraPos, float focalLength){
    for (int i = 0; i < triangles.size(); i++)
    {
        // Project the 3D vertices onto the 2D canvas
        CanvasPoint projectedVertices[3];
        for (int j = 0; j < 3; j++)
        {
            projectedVertices[j] = projectVertexOntoCanvasPoint(
                cameraPos,                // Camera position
                focalLength,              // Focal length
                triangles[i].vertices[j], // Vertex position
                window                    // Drawing window
            );
        }
        // Draw the triangle on the canvas
        CanvasTriangle canvasTriangle(projectedVertices[0], projectedVertices[1], projectedVertices[2]);
        std::cout << "Rasterising triangle " << i << std::endl;
        barycentricFillTriangle(window, canvasTriangle, triangles[i].colour);
}
}

void handleEvent(SDL_Event event, DrawingWindow &window)
{

    if (event.type == SDL_KEYDOWN)
    {
        if (event.key.keysym.sym == SDLK_LEFT)
            std::cout << "LEFT" << std::endl;
        else if (event.key.keysym.sym == SDLK_RIGHT)
            std::cout << "RIGHT" << std::endl;
        else if (event.key.keysym.sym == SDLK_UP)
            std::cout << "UP" << std::endl;
        else if (event.key.keysym.sym == SDLK_DOWN)
            std::cout << "DOWN" << std::endl;

        else if (event.key.keysym.sym == SDLK_ESCAPE)
            window.exitCleanly();
    }
    else if (event.type == SDL_MOUSEBUTTONDOWN)
    {
        window.savePPM("output.ppm");
        window.saveBMP("output.bmp");
    }
}

int main(int argc, char *argv[]){
    DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
    SDL_Event event;
    glm::vec3 cameraPos = glm::vec3(0.0, 0.0, 4.0);
    float focalLength = 2.0;
    initializeDepthBuffer();
    std::map<std::string, Colour> colourMap = loadPalette("/home/leonie/CG2025/Weekly Workbooks/01 Introduction and Orientation/extras/RedNoise/src/cornell-box.mtl");
    std::vector<ModelTriangle> OBJContents = processOBJFile("/home/leonie/CG2025/Weekly Workbooks/01 Introduction and Orientation/extras/RedNoise/src/cornell-box.obj", colourMap);
    //renderPointCloud(window, OBJContents, cameraPos, focalLength);
    //renderWireframe(window, OBJContents, cameraPos, focalLength);
    rasterisedRender(window, OBJContents, cameraPos, focalLength);
    while (true)
    {
        // We MUST poll for events - otherwise the window will freeze !
        if (window.pollForInputEvents(event))
            handleEvent(event, window);
        // Need to render the frame at the end, or nothing actually gets shown on the screen !
        window.renderFrame();
    }
}