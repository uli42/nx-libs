#!/bin/bash

DIR=$1
find _patches_Xserver/$DIR -name xx\* | while read patch; do DEST=$(awk '/^diff / {print $4}' $patch); echo "$patch -> $DEST"; patch -p0 <$patch && rm $patch; done
find _patches_Xserver/ -type d -empty -delete
