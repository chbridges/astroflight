#!/bin/bash
if test $# -eq 0
then echo
    echo "This shell-script compiles a <filename>.cpp and links "
    echo "the X11, OpenGL and GLFW-Libs."
    echo "If you get this message, check for the <filename>."
    echo
    echo "Usage: my_ccp (filename, without qualifier .cpp)"
else if test ! -s "$1.cpp"
    then echo "no file "$1.cpp" or "$1.cpp" is empty"
    else echo "g++ -std=c++1z "$1.cpp" -lglfw3 -lGL -lX11 -lpthread -lXrandr -lXi -ldl "
        g++ -std=c++1z glad.c "$1.cpp" -o $1 -lstdc++fs -lglfw3 -lGL -lX11 -lpthread -lXrandr -lXi -ldl
        echo "strip $1"
        strip $1
        ./$1
    fi
fi      
