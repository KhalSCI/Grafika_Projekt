#include "lodepng.h"
#include <vector>
#include <iostream>

int main() {
    // Create a 64x64 bright yellow texture
    unsigned width = 64, height = 64;
    std::vector<unsigned char> image(width * height * 4);
    
    // Fill with bright yellow (255, 255, 51, 255) - RGBA
    for (unsigned y = 0; y < height; y++) {
        for (unsigned x = 0; x < width; x++) {
            unsigned index = (y * width + x) * 4;
            image[index + 0] = 255; // Red
            image[index + 1] = 255; // Green
            image[index + 2] = 51;  // Blue (less blue for more yellow)
            image[index + 3] = 255; // Alpha
        }
    }
    
    // Save as PNG
    unsigned error = lodepng::encode("sun_yellow.png", image, width, height);
    if (error) {
        std::cout << "PNG save error: " << lodepng_error_text(error) << std::endl;
        return 1;
    }
    
    std::cout << "Created sun_yellow.png texture successfully!" << std::endl;
    return 0;
}
