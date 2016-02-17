# tonemap
Procedure to tonemap an incorrectly exposed image.
Thomas Rapp

Spatial Convolution



Description: 

	This program is written in c++ and requires

	all compilation and packages associated with

	the language.

	

	This program utilizes OpenGL to display and 

	manipulate an image. The program also uses 

	OpenImageIO's read and write image functionalities 

	to read and write images from and to a specified file.

	Libraries are included in the program, however, one 

	must have the libraries downloaded for OpenGL, GLUT,

	and OpenImageIO.

	

	The program takes an overexposed/underexposed image 

	and attempts to create a better exposed image using

	tonemapping or, optionally, tonemapping with

	convolution.



Usage:

	A Makefile is provided. In the directory with the 

	source file, type command 'make' to compile program

	and create executable called 'tonemap.exe'.

	

	A gamma value is required in order to gamma correct

	the image. An optional '-c' flag is required to tell

	the program to perform tonemapping with convolution.

	An output image file location is required is saving

	is desired.

	

	To execute:

	" ./tonemap 'input image file' -g 'gamma value' 'optional -c' 'optional output image file' "



	Upon opening an image, following 

	key presses are allowed:

	's' = switch image between tonemapped

			original

	'w' = write image to user given

			file location

	'q' or 'ESC' = quit and terminate

						program
