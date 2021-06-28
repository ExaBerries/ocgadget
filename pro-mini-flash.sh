#!/bin/bash

set -eux

avrdude \
	-C/etc/avrdude.conf \
	-v \
	-patmega32u4 \
	-cavr109 \
	-P$1 \
	-b57600 \
	-D -Uflash:w:$2:i
