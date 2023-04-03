#!/bin/bash

find . -type f -maxdepth 1 \! \( -name "NOTICE.md" -or -name "clean.sh" \) -exec rm -f {} +
find . -type d -maxdepth 1 \! \( -name "." -or -name ".tarballs" \) -exec rm -rf {} +