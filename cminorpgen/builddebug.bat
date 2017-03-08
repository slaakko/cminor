@echo off
cminor clean cminorpgen.cminors
cminor build --native --link-with-debug-machine --use-ms-link cminorpgen.cminors
cminor clean --config=release cminorpgen.cminors
cminor build --config=release --native --link-with-debug-machine --use-ms-link cminorpgen.cminors
