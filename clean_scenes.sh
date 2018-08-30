#!/bin/bash

find resources -name '*.entity' -type f -not -path '*/saves/*' -exec rm {} \;
find resources -name '*.scene' -type f -not -path '*/saves/*' -exec rm {} \;
find resources/scenes/ -type d -empty -delete
find resources/scenes/* -type d -not -path '*/entities*' -exec mkdir -p '{}/entities' \;