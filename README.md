# AstroFlight (working title)
A 2D puzzle game based on classical mechanics, astronomy and further principles of physics. Developed in terms of a computer graphics practical as a pure OpenGL project that doesn't use any external ressources such as textures.

Current development state: Alpha (Menus and a bigger level selection are missing)

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
    D           Toggle debug mode (FPS counter)
    T           Toggle trajectory
    C           Toggle center of mass
    R           Restart level
    N           Next level
    P           Pause
    +/-         Adjust game speed (German keyboard layout)
    Esc         Exit

## Possible upcoming features
- Main menu and level selection
- Particle effects
- Highscore system
- Unlockables
- Level editor
- Procedural level generation (supported by machine learning)

## Further planned improvisations
Better code structure, error handling and readme
