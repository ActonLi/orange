#!/bin/bash

git filter-branch --force --index-filter 'git rm --cached --ignore-unmatch ./libs/libdwarf/dwarfdump/dwarfdump' --prune-empty --tag-name-filter cat -- --all
git push origin master --force
git push origin master --force --tags
rm -rf .git/refs/original/
git reflog expire --expire=now --all
git gc --prune=now
git gc --aggressive --prune=now


