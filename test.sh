#!/bin/bash
# Script rápido: compila y ejecuta en un comando

make clean && make && bochs -f bochsrc -q
