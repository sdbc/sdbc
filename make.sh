#!/bin/sh

rm -rf build;
make build;
cd build;
cmake ../;

## libscbase.a
