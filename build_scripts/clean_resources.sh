#!/bin/bash

find resources/ -name '*.entity' -type f -not -path '*/saves/*' -exec rm {} \;
find resources/ -name '*.scene' -type f -not -path '*/saves/*' -exec rm {} \;
find resources/ -type d -empty -delete
find resources/scenes/* -mindepth 0 -maxdepth 0 -type d -not -path '*/entities*' -exec mkdir -p '{}/entities' \;

mkdir -p resources/audio/
mkdir -p resources/cubemaps/
mkdir -p resources/heightmaps/
mkdir -p resources/materials/
mkdir -p resources/particles/
mkdir -p resources/saves/
mkdir -p resources/scenes/