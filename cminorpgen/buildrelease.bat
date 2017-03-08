@echo off
cminor clean cminorpgen.cminors
cminor build --native --use-ms-link cminorpgen.cminors
cminor clean --config=release cminorpgen.cminors
cminor build --config=release --native --use-ms-link cminorpgen.cminors
