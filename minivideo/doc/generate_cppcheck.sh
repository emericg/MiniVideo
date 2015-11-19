
#!/bin/bash

if [[ $(which cppcheck) && $(which cppcheck-htmlreport) ]]; then

    if [[ ${PWD##*/} == 'doc' ]]; then
        if [[ ! -d cppcheck_temp ]]; then
            mkdir -p cppcheck_temp
        fi

        cppcheck --enable=all --xml-version=2 --suppressions-list=cppcheck_ignore_list.txt ../src/ 2> cppcheck_temp/err.xml
        cppcheck-htmlreport --file=cppcheck_temp/err.xml --report-dir=cppcheck_temp/ --source-dir=../src/
    else
        echo "Error, you must be in the minivideo/doc directory to run this script!"
    fi

else
    echo "Error, cppcheck is not available on your system!"
fi
