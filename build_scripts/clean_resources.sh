#!/bin/bash

find resources/ -name '*.entity' -type f -not -path '*/saves/*' -exec rm {} \;
find resources/ -name '*.scene' -type f -not -path '*/saves/*' -exec rm {} \;
find resources/ -type d -empty -delete
find resources/scenes/* -type d -not -path '*/entities*' -exec mkdir -p '{}/entities' \;

mkdir -p resources/audio/
mkdir -p resources/heightmaps/
mkdir -p resources/images/
mkdir -p resources/materials/
mkdir -p resources/saves/
mkdir -p resources/scenes/