@echo off
cminor run cminorpgen/assembly/debug/cminorpgen.cminora -F ../system/System.Text.Parsing/StdLib.pp
cminor run cminorpgen/assembly/debug/cminorpgen.cminora -F code/Code.pp
cminor run cminorpgen/assembly/debug/cminorpgen.cminora -F syntax/Syntax.pp
cminorbuildsys
cminor build cminorpgen.cminors
