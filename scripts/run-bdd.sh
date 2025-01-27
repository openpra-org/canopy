#!/bin/bash

/home/earthperson/projects/canopy/cmake-build-release/src/scram-cli/scram-cli \
--verbosity 7 \
--probability \
$1 \
fixtures/generic-openpsa-models/models/Aralia/$2 | grep "sum-of-products"
