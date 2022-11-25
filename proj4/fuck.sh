#!/bin/bash

if [ ! -f files.txt ]; then touch files.txt; fi
rm -rf build
xargs rm -rf < files.txt
python setup.py install --record files.txt