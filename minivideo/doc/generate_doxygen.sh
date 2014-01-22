#!/bin/bash

if [[ $(which doxygen) ]]; then

    if [[ ${PWD##*/} == 'doc' ]]; then
        cd doxygen_temp/
        doxygen Doxyfile
    else
        echo "Error, you must be in the minivideo/doc directory to run this script!"
    fi

else
    echo "Error, doxygen is not available on your system!"
fi