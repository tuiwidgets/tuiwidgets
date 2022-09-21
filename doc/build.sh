#! /bin/bash

if [ ! -e ../../sphinx-v5/bin/sphinx-build ]; then
    echo "pleas install www.sphinx-doc.org to ../../sphinx-v5"
    exit 1
fi

../../sphinx-v5/bin/sphinx-build -b html . _build -a -d _doctrees -n -E "$@"

./sphinxlint.py --disable "trailing-whitespace" --enable default-role
