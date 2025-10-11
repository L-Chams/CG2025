#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <Utils.h>
#include <fstream>
#include <vector>
#include <glm/glm.hpp>
#include <CanvasPoint.h>
#include <Colour.h>

#define WIDTH 320
#define HEIGHT 240

std::vector<float> interpolateSingleFloats(float from, float to, int numberOfValues)
{
	// create an evenly spaced list of floats between from and to, size numberOfValues
	std::vector<float> values;
	float spacing = (to - from) / (numberOfValues - 1);
	for (int i = 0; i < numberOfValues; i++)
	{
		values.push_back(from + i * spacing);
	}
	return values;
}

std::vector<glm::vec3> interpolateThreeElementValues(glm::vec3 from, glm::vec3 to, int numberOfValues)
{
	// similar to interpolateSingleFloats but this time with 3-element values
	std::vector<glm::vec3> values;
	std::vector<float> spacing;
	for (int i = 0; i < 3; i++)
	{
		spacing.push_back((to[i] - from[i]) / (numberOfValues - 1));
	}
	for (int i = 0; i < numberOfValues; i++)
	{
		values.push_back({(from.x + i * spacing[0]), (from.y + i * spacing[1]), (from.z + i * spacing[2])});
	}
	return values;
}

void draw(DrawingWindow &window)
{
	window.clearPixels();
	for (size_t y = 0; y < window.height; y++)
	{
		for (size_t x = 0; x < window.width; x++)
		{
			float red = rand() % 256;
			float green = 0;
			float blue = 0;
			uint32_t colour = (255 << 24) + (int(red) << 16) + (int(green) << 8) + int(blue);
			window.setPixelColour(x, y, colour);
		}
	}
}

void drawGrey(DrawingWindow &window)
{
	window.clearPixels();
	std::vector<float> values = interpolateSingleFloats(255, 0, window.width);
	for (size_t y = 0; y < window.height; y++)
	{
		for (size_t x = 0; x < window.width; x++)
		{
			float red = values[x];
			float green = values[x];
			float blue = values[x];
			uint32_t colour = (255 << 24) + (int(red) << 16) + (int(green) << 8) + int(blue);
			window.setPixelColour(x, y, colour);
		}
	}
}

void twoDimensionInterpolation(DrawingWindow &window)
{
	glm::vec3 topLeft(255, 0, 0);	   // red
	glm::vec3 topRight(0, 0, 255);	   // blue
	glm::vec3 bottomRight(0, 255, 0);  // green
	glm::vec3 bottomLeft(255, 255, 0); // yellow
	window.clearPixels();

	std::vector<glm::vec3> leftColumn = interpolateThreeElementValues(topLeft, bottomLeft, window.height);
	std::vector<glm::vec3> rightColumn = interpolateThreeElementValues(topRight, bottomRight, window.height);

	for (size_t y = 0; y < window.height; y++)
	{
		std::vector<glm::vec3> values = interpolateThreeElementValues(leftColumn[y], rightColumn[y], window.width);
		for (size_t x = 0; x < window.width; x++)
		{
			float red = values[x].x;
			float green = values[x].y;
			float blue = values[x].z;
			uint32_t colour = (255 << 24) + (int(red) << 16) + (int(green) << 8) + int(blue);
			window.setPixelColour(x, y, colour);
		}
	}
}

void barycentricTriangularInterpolation(DrawingWindow &window)
{
	glm::vec2 v0(0, window.height - 1);					   // bottom left
	glm::vec2 v1((window.width - 1) / 2, 0);			   // top
	glm::vec2 v2((window.width - 1), (window.height - 1)); // bottom right

	for (size_t y = 0; y < window.height; y++)
	{
		for (size_t x = 0; x < window.width; x++)
		{
			glm::vec3 coords = convertToBarycentricCoordinates(v0, v1, v2, glm::vec2(x, y));

			if (coords.x >= 0 && coords.y >= 0 && coords.z >= 0)
			{
				float red = coords.z * 255;
				float green = coords.x * 255;
				float blue = coords.y * 255;
				uint32_t colour = (255 << 24) + (int(red) << 16) + (int(green) << 8) + int(blue);
				window.setPixelColour(x, y, colour);
			}
		}
	}
}

void drawLine(DrawingWindow &window, CanvasPoint from, CanvasPoint to, Colour colour_param)
{
	float x_dist = to.x - from.x;
	float y_dist = to.y - from.y;

	float numberOfSteps = fmax(abs(x_dist), abs(y_dist));
	float x_spacing = x_dist / numberOfSteps;
	float y_spacing = y_dist / numberOfSteps;
	uint32_t colour = (255 << 24) + (int(colour_param.red) << 16) + (int(colour_param.green) << 8) + int(colour_param.blue);
	for (int i = 0; i < numberOfSteps; i++)
	{
		float x = from.x + (x_spacing * i);
		float y = from.y + (y_spacing * i);

		window.setPixelColour(round(x), round(y), colour);
	}
}

void strokedTriangle(DrawingWindow &window, CanvasTriangle triangle, Colour colour)
{
	drawLine(window, triangle.v0(), triangle.v1(), colour);
	drawLine(window, triangle.v1(), triangle.v2(), colour);
	drawLine(window, triangle.v2(), triangle.v0(), colour);
}

void fillFlatTopTriangle(DrawingWindow &window, CanvasTriangle triangle, Colour colour_param)
{
	// Get the vertices of the triangle
	CanvasPoint topLeft = triangle.v0();
	CanvasPoint topRight = triangle.v1();
	CanvasPoint bottom = triangle.v2();

	// Ensure topLeft is left of topRight
	if (topLeft.x > topRight.x)
		std::swap(topLeft, topRight);

	float invslopeLeft = (bottom.x - topLeft.x) / (bottom.y - topLeft.y);
	float invslopeRight = (bottom.x - topRight.x) / (bottom.y - topRight.y);

	float currentXLeft = bottom.x;
	float currentXRight = bottom.x;

	//draw lines top to bottom, left to right
	for(int i = bottom.y; i > topLeft.y; i--)
	{
		drawLine(window, CanvasPoint(currentXLeft, i), CanvasPoint(currentXRight, i), colour_param);
		currentXLeft -= invslopeLeft;
		currentXRight -= invslopeRight;
	}

}

void fillFlatBottomTriangle(DrawingWindow &window, CanvasTriangle triangle, Colour colour_param)
{
	// Get the vertices of the triangle
	CanvasPoint bottomLeft = triangle.v0();
	CanvasPoint bottomRight = triangle.v1();
	CanvasPoint top = triangle.v2();

	// Ensure bottomLeft is left of bottomRight
	if (bottomLeft.x > bottomRight.x)
		std::swap(bottomLeft, bottomRight);

	float invslopeLeft = (bottomLeft.x - top.x) / (bottomLeft.y - top.y);
	float invslopeRight = (bottomRight.x - top.x) / (bottomRight.y - top.y);

	float currentXLeft = top.x;
	float currentXRight = top.x;

	//draw lines top to bottom, left to right
	for (int i = top.y; i <= bottomLeft.y; i++)
	{
		drawLine(window, CanvasPoint(currentXLeft, i), CanvasPoint(currentXRight, i), colour_param);
		currentXLeft += invslopeLeft;
		currentXRight += invslopeRight;
	}


}
void drawFilledTriangle(DrawingWindow &window, CanvasTriangle triangle, Colour colour_param)
{
	// sort vertices by vertical pos - top to bottom
	CanvasPoint v0 = triangle.v0();
	CanvasPoint v1 = triangle.v1();
	CanvasPoint v2 = triangle.v2();

	if (v0.y > v1.y)
		std::swap(v0, v1);
	if (v0.y > v2.y)
		std::swap(v0, v2);
	if (v1.y > v2.y)
		std::swap(v1, v2);

	CanvasPoint top = v0;    // smallest Y = top of screen
	CanvasPoint middle = v1;
	CanvasPoint bottom = v2; // largest Y = bottom of screen

	// check for flat top or flat bottom
	if (top.y == middle.y) // flat top
	{
		CanvasTriangle flatTopTriangle(middle, top, bottom);
		//topleft, topright, bottom
		fillFlatTopTriangle(window, flatTopTriangle, colour_param);
		return;
	}
	else if (middle.y == bottom.y) // flat bottom
	{
		CanvasTriangle flatBottomTriangle(bottom, middle, top);
		//bottomleft, bottomright, top
		fillFlatBottomTriangle(window, flatBottomTriangle, colour_param);
		return;
	}
	else
	{
		// Calculate the extra point on the edge from bottom to top at middle.y
		// Using the formula: x = x1 + (y - y1) * (x2 - x1) / (y2 - y1)
		float extraPointX = bottom.x + (middle.y - bottom.y) * (top.x - bottom.x) / (top.y - bottom.y);
		CanvasPoint extraPoint(extraPointX, middle.y);

		// divide triangle into 2 triangles

		//bottomleft, bottomright, top
		CanvasTriangle flatBottomTriangle(middle, extraPoint, top);

		//topleft, topright, bottom
		CanvasTriangle flatTopTriangle(middle, extraPoint, bottom);

		// fill top triangle
		fillFlatTopTriangle(window, flatTopTriangle, colour_param);

		// fill bottom triangle
		fillFlatBottomTriangle(window, flatBottomTriangle, colour_param);

	}
}

CanvasPoint randCoord()
{
	int randX = rand() % WIDTH;
	int randY = rand() % HEIGHT;
	return CanvasPoint(randX, randY);
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
		else if (event.key.keysym.sym == SDLK_u)
		{
			Colour randColour(rand() % 256, rand() % 256, rand() % 256);
			CanvasPoint v0 = randCoord();
			CanvasPoint v1 = randCoord();
			CanvasPoint v2 = randCoord();
			strokedTriangle(window, CanvasTriangle(v0, v1, v2), randColour);
		}
		else if (event.key.keysym.sym == SDLK_f)
		{
			Colour randColour(rand() % 256, rand() % 256, rand() % 256);
			CanvasPoint v0 = randCoord();
			CanvasPoint v1 = randCoord();
			CanvasPoint v2 = randCoord();
			drawFilledTriangle(window, CanvasTriangle(v0, v1, v2), randColour);
			strokedTriangle(window, CanvasTriangle(v0, v1, v2), Colour(255, 255, 255));
		}
	}
	else if (event.type == SDL_MOUSEBUTTONDOWN)
	{
		window.savePPM("output.ppm");
		window.saveBMP("output.bmp");
	}
}

int main(int argc, char *argv[])
{
	DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
	SDL_Event event;
	// top
	CanvasPoint topLeft(0, 0);
	CanvasPoint topMiddle(WIDTH / 2, 0);
	CanvasPoint topRight(WIDTH, 0);
	// middle
	CanvasPoint middleLeft(0, HEIGHT / 2);
	CanvasPoint middle(WIDTH / 2, HEIGHT / 2);
	CanvasPoint middleRight(WIDTH, HEIGHT / 2);

	CanvasPoint thirdLeft(WIDTH / 3, HEIGHT / 2);
	CanvasPoint thirdRight(2 * (WIDTH / 3), HEIGHT / 2);
	// bottom
	CanvasPoint bottomLeft(0, HEIGHT);
	CanvasPoint bottomMiddle(WIDTH / 2, HEIGHT);
	CanvasPoint bottomRight(WIDTH, HEIGHT);

	while (true)
	{
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event))
			handleEvent(event, window);
		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		window.renderFrame();
	}
}