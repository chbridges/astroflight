# AstroFlight (working title)
A 2D puzzle game based on classical mechanics, astronomy and further principles of physics. Developed in terms of a computer graphics practical as a pure OpenGL project that doesn't use any external ressources such as textures.

Current development state: Pre-Alpha (essential gameplay features are missing)

## Required libraries
GLFW3, GLAD, GLM

GLAD settings: gl 4.6, core profile, generate a loader

Put GLM into the working directory.

Linux: Download GLFW3 and GLAD, compile & install using CMake and Make, compile via compile.sh bash script.

Windows: Download the pre-compiled headers for VS or MinGW and link them in your IDE.

## Controls
    Arrows  Rotate, Launch & Boost
    F       Toggle fullscreen/windowed mode
    W       Toggle wireframe mode
    R       Restart level
    N       Next level
    Esc     Exit

## Possible upcoming features
- Collision detection
- Black holes
- Terraforming
- Particle effects
- Score system
- Level editor
- Procedural level generation (supported by machine learning)

## Further planned improvisations
Better code structure, error handling and readme
