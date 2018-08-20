#!/bin/bash
g++ -o sentry sentry_lookup.cpp libini.cpp
if [ $? != 0 ]; then
    echo "Error compiling . . ."
else
    echo "sentry successfully compiled!"
fi

curl --version &>/dev/null
if [ $? != 0 ]; then
    echo "Error: Missing runtime dependency: 'curl'."
fi

mail --version &>/dev/null
if [ $? != 0 ]; then
    echo "Error: Missing runtime dependency: 'mail'."
else
    echo "Be sure to properly configure your mail SMTP server!"
fi

