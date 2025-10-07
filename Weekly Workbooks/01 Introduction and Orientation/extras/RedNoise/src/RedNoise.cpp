#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <Utils.h>
#include <fstream>
#include <vector>
#include <glm/glm.hpp>
#include <CanvasPoint.h>

#define WIDTH 320
#define HEIGHT 240

std::vector<float> interpolateSingleFloats(float from, float to, int numberOfValues) {
	//create an evenly spaced list of floats between from and to, size numberOfValues
	std::vector<float> values;
	float spacing = (to - from)/(numberOfValues-1);
	for (int i = 0; i < numberOfValues; i++) {
		values.push_back(from + i*spacing);
	}
	return values;
}

std::vector<glm::vec3> interpolateThreeElementValues(glm::vec3 from, glm::vec3 to, int numberOfValues){
	//similar to interpolateSingleFloats but this time with 3-element values
	std::vector<glm::vec3> values;
	std:: vector<float> spacing;
	for (int i = 0; i < 3; i++) {
		spacing.push_back((to[i] - from[i])/(numberOfValues-1));
	}
	for(int i = 0; i < numberOfValues; i++){
		values.push_back({(from.x +i*spacing[0]), (from.y+i*spacing[1]), (from.z+i*spacing[2])});
	}
	return values;
}



void draw(DrawingWindow &window) {
	window.clearPixels();
	for (size_t y = 0; y < window.height; y++) {
		for (size_t x = 0; x < window.width; x++) {
			float red = rand()%256;
			float green = 0;
			float blue = 0;
			uint32_t colour = (255 << 24) + (int(red) << 16) + (int(green) << 8) + int(blue);
			window.setPixelColour(x, y, colour);
		}
	}
}

void drawGrey(DrawingWindow &window) {
	window.clearPixels();
	std::vector<float>  values = interpolateSingleFloats(255, 0, window.width);
	for (size_t y = 0; y < window.height; y++) {
		for (size_t x = 0; x < window.width; x++) {
			float red = values[x];
			float green = values[x];
			float blue = values[x];
			uint32_t colour = (255 << 24) + (int(red) << 16) + (int(green) << 8) + int(blue);
			window.setPixelColour(x, y, colour);
		}
	}
}

void twoDimensionInterpolation(DrawingWindow &window) {
	glm::vec3 topLeft(255, 0, 0);        // red
	glm::vec3 topRight(0, 0, 255);       // blue
	glm::vec3 bottomRight(0, 255, 0);    // green
	glm::vec3 bottomLeft(255, 255, 0);   // yellow
	window.clearPixels();

	std::vector<glm::vec3> leftColumn = interpolateThreeElementValues(topLeft, bottomLeft, window.height);
	std::vector<glm::vec3> rightColumn = interpolateThreeElementValues(topRight, bottomRight, window.height);

	for (size_t y = 0; y < window.height; y++) {
		std::vector<glm::vec3> values = interpolateThreeElementValues(leftColumn[y], rightColumn[y], window.width);
		for (size_t x = 0; x < window.width; x++) {
			float red =values[x].x;
			float green= values[x].y;
			float blue= values[x].z;
			uint32_t colour = (255 << 24) + (int(red) << 16) + (int(green) << 8) + int(blue);
			window.setPixelColour(x, y, colour);
		}
	}
}

void barycentricTriangularInterpolation(DrawingWindow &window) {
	glm::vec2 v0(0,window.height-1); //bottom left
	glm::vec2 v1((window.width-1)/2, 0); //top
	glm::vec2 v2((window.width-1), (window.height-1)); //bottom right

	for (size_t y = 0; y < window.height; y++) {
		for (size_t x = 0; x < window.width; x++) {
		glm::vec3 coords = convertToBarycentricCoordinates(v0, v1, v2, glm::vec2(x,y));

			if (coords.x >= 0 && coords.y >=0 && coords.z >=0){
				float red =coords.z *255;
				float green= coords.x*255;
				float blue= coords.y*255;
				uint32_t colour = (255 << 24) + (int(red) << 16) + (int(green) << 8) + int(blue);
				window.setPixelColour(x, y, colour);
			}
	}
}
}

// void drawLine(DrawingWindow %window,float from, to) {

// }

void handleEvent(SDL_Event event, DrawingWindow &window) {
	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_LEFT) std::cout << "LEFT" << std::endl;
		else if (event.key.keysym.sym == SDLK_RIGHT) std::cout << "RIGHT" << std::endl;
		else if (event.key.keysym.sym == SDLK_UP) std::cout << "UP" << std::endl;
		else if (event.key.keysym.sym == SDLK_DOWN) std::cout << "DOWN" << std::endl;
	} else if (event.type == SDL_MOUSEBUTTONDOWN) {
		window.savePPM("output.ppm");
		window.saveBMP("output.bmp");
	}
}

int main(int argc, char *argv[]) {
	DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
	SDL_Event event;
	while (true) {
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window);
		barycentricTriangularInterpolation(window);
		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		window.renderFrame();
	}
}