libdazeus-irc
=============

A C++ wrapper around the libircclient library, to take as much IRC-related
tasks off your shoulders as possible. Define a network and its servers, and
libdazeus-irc will keep track of (re-)connections for you. It will also keep a
list of joined channels up-to-date, and will keep a list of identified users
up-to-date if you ask for it.

Build instructions
==================

    mkdir build
    cd build
    cmake ..
    make
