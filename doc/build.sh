#! /bin/bash

../../sphinx-v5/bin/sphinx-build -b html . _build -a -d _doctrees -n -E

./sphinxlint.py --disable "trailing-whitespace" --enable default-role
