@echo off
cminor clean system
cminor build --native --link-with-debug-machine --use-ms-link system
cminor clean --config=release system
cminor build --config=release --native --link-with-debug-machine --use-ms-link system
