#!/bin/bash

PROJECT_NAME=ghoti

mkdir release/
find build/* -type f -not -path '*/obj/*' -exec cp {} release/ \;

cp -r doc/ release/
cp $PROJECT_NAME.json release/

cp -r resources/ release/
cp build_scripts/default_init.lua release/resources/scripts/init.lua

mkdir release/resources/temp/
mv release/resources/fonts/default_font.ttf release/resources/temp/
mv release/resources/images/widget_background.png release/resources/temp/
mv release/resources/models/box/ release/resources/temp/
mv release/resources/models/sphere/ release/resources/temp/
mv release/resources/models/cylinder/ release/resources/temp/
mv release/resources/models/hemisphere/ release/resources/temp/
rm -rf release/resources/temp/**/*.fbx
rm -rf release/resources/temp/**/*.json

rm -rf release/resources/audio/*
rm -rf release/resources/fonts/*
rm -rf release/resources/heightmaps/*
rm -rf release/resources/images/*
rm -rf release/resources/materials/*
rm -rf release/resources/models/*
rm -rf release/resources/saves/*
rm -rf release/resources/scenes/*
rm -rf release/resources/scripts/components/debug/
rm -rf release/resources/scripts/systems/debug/

mv release/resources/temp/default_font.ttf release/resources/fonts/
mv release/resources/temp/widget_background.png release/resources/images/
mv release/resources/temp/box/ release/resources/models/
mv release/resources/temp/sphere/ release/resources/models/
mv release/resources/temp/cylinder/ release/resources/models/
mv release/resources/temp/hemisphere/ release/resources/models/
rmdir release/resources/temp/

if [ "$1" != "windows" ]; then
	mv release/$PROJECT_NAME release/$PROJECT_NAME-bin
	cp build_scripts/$PROJECT_NAME release/
	cp -r lib/ release/
fi