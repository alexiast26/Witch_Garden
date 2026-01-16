# Witch's Garden - 3D Interactive Environment

An immersive 3D scene developed in C++ using OpenGL 4.1. This project simulates a mystical nocturnal garden, featuring advanced lighting models, atmospheric effects, and interactive elements.



## Key Features

- **Advanced Lighting:** Implementation of the Blinn-Phong lighting model with a directional moonlight and 7 point lights (candles/lanterns) featuring quadratic attenuation.
- **Shadow Mapping:** Real-time shadow generation using depth maps for enhanced spatial realism.
- **Atmospheric Fog:** Exponential Squared Fog implementation to create a mysterious nocturnal mood and improve depth perception.
- **Interactive Elements:** Procedural animations for scene objects (e.g., rotating cat, pouring teapot) triggered by user input.
- **Skybox:** A seamless 360-degree starry night sky using cube mapping.
- **Optimized for HiDPI:** Custom framebuffer management to support Retina and 4K displays.



## Built With

- **Language:** C++11
- **Graphics API:** OpenGL 4.1
- **Libraries:**
  - **GLFW:** Window management and input handling.
  - **GLEW:** OpenGL extension loading.
  - **GLM:** Mathematics (matrices, vectors).
  - **Assimp:** 3D model loading (.obj, .mtl).
  - **irrKlang:** 3D spatial audio system.

## Getting Started

### Prerequisites
- A GPU supporting at least **OpenGL 4.1**.
- Visual Studio 2019 or newer (for Windows).
- Updated graphics drivers.

## Audio Setup
The project uses the **irrKlang** audio engine. Due to GitHub's file size restrictions (files over 100MB), the high-quality `.wav` background track is excluded from the repository via `.gitignore`.

**To enable sound in your local build:**
1. Create an `audio/` folder in the project root.
2. Place a file named `background.wav` (or your preferred audio file) inside it.
4. The application will automatically detect and loop the audio on startup.

### Installation & Running
1. **Clone the repository:**
   ```bash
   git clone [https://github.com/alexiast26/Witch_Garden.git](https://github.com/alexiast26/Witch_Garden.git)
