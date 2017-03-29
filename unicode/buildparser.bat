@echo off
cminor run --native C:\Users\Seppo\cminor\cminorpgen\assembly\debug\cminorpgen.cminora -F UnicodeData.pp
cminor build --native UnicodeBinBuilder.cminorp
