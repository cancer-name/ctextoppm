#!/usr/bin/env sh

./a.out 0008b450.tex color.ppm alpha.pbm
pnmtopng color.ppm -alpha alpha.pbm >texture.png
