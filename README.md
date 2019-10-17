# AstroFlight (working title)
A 2D puzzle game based on classical mechanics, astronomy and further principles of physics. Developed in terms of a computer graphics practical as a pure OpenGL project that doesn't use any external ressources such as textures (except for a single font).

Current development state: Alpha (Menus and a bigger level selection are missing)

**This is a learning project. Due to early problems and a resulting lack of structure and optimization, the code should probably be rewritten entirely and will therefore no longer be actively maintained.**

## Compiling
Required libraries: GLFW3, GLAD, GLM, FreeType2

GLAD settings: gl 3.3 or higher, core profile, generate a loader

Put GLM and glad.c into the working directory. As for the other libraries:

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
    O           Toggle gravity gradient
    G           Toggle GUI
    R           Restart level
    N           Next level
    P           Pause
    +/-         Adjust game speed (German keyboard layout)
    Esc         Exit

## Possible upcoming features
- Controls overview in GUI
- Main menu and level selection
- Highscore system
- Score system rework (bonus points for landing near the flag, punishments for "cheating")
- Unlockables
- Level editor
- Procedural level generation (perhaps supported by machine learning)
- Improved trajectory for dynamic levels using e.g. Euler's method
- Particle effects

## Further planned improvisations
Better code structure, error handling and performance optimization
