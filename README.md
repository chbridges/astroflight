# AstroFlight (working title)
A 2D puzzle game based on classical mechanics, astronomy and further principles of physics, taking on a new approach to how gravity works in video games. Developed in terms of a computer graphics practical as a pure OpenGL project that doesn't use any external ressources such as textures, except for the GUI font.

Current development state: Alpha (Menus and a bigger level selection are missing)

**This is a learning project. Due to early problems and a resulting lack of structure and optimization, the code should probably be rewritten entirely and will therefore no longer be actively maintained.**

## Compiling
Required libraries: [GLFW3](https://www.glfw.org/), [GLAD](https://glad.dav1d.de/), [GLM](https://glm.g-truc.net/0.9.9/index.html), [FreeType2](https://www.freetype.org/)

GLAD settings: gl 3.3 or higher (4.6 has been used), core profile, generate a loader

Put GLM and glad.c into the working directory. As for the other libraries:

Linux: Download GLFW3, GLAD and FreeType2, compile & install them, compile AstroFlight via the compile.sh bash script.

Windows: Download the pre-compiled headers (GLFW3, GLAD, FreeType2) for VS or MinGW and link them in your IDE.

## Structure
    /builds/            Contains the latest builds for Win32 and Linux
    /gui/               Contains the GUI font and a copyright notice
    /levels/            Contains levels (plain text files *.lvl) and a .txt file documenting their structure
    /shaders/           Contains all fragment shaders (*.fsh) and vertex shaders (*.vsh) written in GLSL
    astroflight.cpp     Manages the window, inputs and ressources, renders the game
    game_objects.hpp    Provides the PointMass base class and all objects to be rendered
    gui.hpp             Provides a font renderer and functions to draw GUI boxes and text
    level.hpp           Provides the Level class including a level loader and physics engine management
    shader.hpp          Provides the Shader class compiling shader programs with given .fsh and .vsh files
    shapes.hpp          Provides the vertices for shapes to be drawn by OpenGL
    compile.sh          Compiles the code with all necessary links and flags on Linux

## Controls
    Arrows      Adjust rotation and launch speed
    Spacebar    Launch spacecraft, drop terraforming boxes
    Shift       Precision mode for arrow controls (hold)
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
    +/-         Adjust game speed (German keyboard layout), partially broken on Linux
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

## Known bugs
Wrong structure in a *.lvl file might cause the game not to load
