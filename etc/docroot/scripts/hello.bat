@echo off

echo ^<html^>
echo 	^<head^>
echo 		^<title^>Hello script^<^/title^>
echo 	^<^/head^>
echo 	^<body^>
echo 		Current time: ^<b^>
date /T
echo ^<^/b^>^<br^/^>
echo 		Network: ^<br^/^>
ipconfig
echo 	^<^/body^>
echo ^<^/html^>
