#!/bin/bash

workpath=`pwd`

dpkg -b ${workpath}/dispatch ${workpath}/dispatch.deb
