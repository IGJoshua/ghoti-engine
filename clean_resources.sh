#!/bin/bash

find resources/materials/ -type d -empty -delete
find resources/models/ -type d -empty -delete
find resources -name '*.entity' -type f -not -path '*/saves/*' -exec rm {} \;
find resources -name '*.scene' -type f -not -path '*/saves/*' -exec rm {} \;
find resources/scenes/ -type d -empty -delete
find resources/scenes/* -type d -not -path '*/entities*' -exec mkdir -p '{}/entities' \;

mkdir -p resources/audio/
mkdir -p resources/heightmaps/
mkdir -p resources/images/