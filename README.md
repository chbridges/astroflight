# AstroFlight (working title)
A 2D puzzle game based on classical mechanics, astronomy and further principles of physics. Developed in terms of a computer graphics practical as a pure OpenGL project that doesn't use any external ressources such as textures.

Current development state: Pre-Alpha (essential gameplay features are missing)

## Compiling
Required libraries: GLFW3, GLAD, GLM, FreeType2

GLAD settings: gl 4.6, core profile, generate a loader

Put GLM into the working directory. As for the other libraries:

Linux: Download GLFW3, GLAD and FreeType2, compile & install them, compile AstroFlight via the compile.sh bash script.

Windows: Download the pre-compiled headers for VS or MinGW and link them in your IDE.

## Controls
    Arrows      Adjust rotation and launch speed
    Spacebar    Launch spacecraft, drop terraforming boxes
    Shift       Precision mode (hold)
    F           Toggle fullscreen/windowed mode
    W           Toggle wireframe mode
    T           Toggle trajectory
    P           Pause
    +/-         Adjust game speed
    R           Restart level
    N           Next level
    Esc         Exit

## Possible upcoming features
- Better GUI, main menu and level selection
- Stars in background
- Black holes
- Particle effects and lighting
- Score system
- Unlockables
- Level editor
- Language support (German, English, Russian)
- Procedural level generation (supported by machine learning)

## Further planned improvisations
Better code structure, error handling and readme
