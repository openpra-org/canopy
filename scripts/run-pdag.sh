#!/bin/bash

ACPP_VISIBILITY_MASK=cuda ACPP_DEBUG_LEVEL=0 ACPP_ADAPTIVITY_LEVEL=2 ACPP_ALLOCATION_TRACKING=1 \
/home/earthperson/projects/canopy/cmake-build-release/src/scram-cli/scram-cli \
--verbosity 7 \
--pdag \
--monte-carlo \
--probability \
--batch-size 131072 \
--sample-size 4 \
--num-trials 10 \
$1 \
"fixtures/generic-openpsa-models/models/Aralia/$2"