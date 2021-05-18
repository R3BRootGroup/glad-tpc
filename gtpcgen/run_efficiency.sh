#!/bin/bash
root -b run_generator.C <<EOF
.q
EOF
cd ../macros/sim
root -b run_simHYDRA.C <<EOF
.q
EOF
cd ../Analysis
root -b run_analysis.C<<EOF
.q
EOF
cd ../../gtpcgen
