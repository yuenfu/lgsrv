#!/bin/sh
./mltar -z -o www.c www-pages
xmkmf && make clean all
