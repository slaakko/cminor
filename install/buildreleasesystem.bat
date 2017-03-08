@echo off
cminor clean system
cminor build --native --use-ms-link system
cminor clean --config=release system
cminor build --config=release --native --use-ms-link system
