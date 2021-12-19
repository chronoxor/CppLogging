@echo off
SET curdir=%~dp0
python "%curdir:~0,-1%\hashlog-map.py" %*
