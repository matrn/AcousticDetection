#!/bin/bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
echo "SCRIPT_DIR: ${SCRIPT_DIR}"
cd ${SCRIPT_DIR}

#
echo "########## Bundling Bootstrap CSS ##########"
cd src/bootstrap && bash bundle.sh
echo ""
#

cd ${SCRIPT_DIR}

#
echo "########## Bundling all files ##########"
mkdir -p dist/
python3 bundle.py
echo ""
#

echo "########## !DONE! ##########"
echo ""