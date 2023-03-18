#!/bin/bash

$(cd src/bootstrap && bash bundle.sh)
mkdir -p dist/
python3 bundle.py