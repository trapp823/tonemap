/*

	Thomas Rapp  tsrapp@g.clemson.edu

	Spatial Convolution

	

	Description: This program utilizes OpenGL to display and manipulate an image. The program also uses OpenImageIO's read and write image functionalities to read and write images from and to a specified file.

*/



#include <cstdlib>

#include <iostream>

#include <fstream>

#include <math.h>



#ifdef __APPLE__

#  include <GLUT/glut.h>

#else

#  include <GL/glut.h>

#endif



#include <OpenImageIO/imageio.h>

OIIO_NAMESPACE_USING



using namespace std;



struct pixel {

	float r, g, b, a;

};



bool allowWrite = false; //flag for write image functionality

bool toneConvolve = false;

int W, H; 					 //width and height resolutions of image

int channels; 				 //amount of color channels in image(3 or 4)

//ImageSpec spec;			 //image specification data(channels,resolution)

float* rawpix;   			 //raw pixel data from read_image, 1d array

float* rawpix2;  			 //raw pixel data sent to write_image, 1d array

char* writeFile;			 //filename to write to

char* filterFile;			 //file that contains filter information

pixel **mypix;			    //2d array for image manipulation

pixel **output;			 //output image data

int filtSize;

float **filtArray;

float sum;

float Lw;

float **logLwArray;

float logLd;

float Ld;

int switchCount = 1;

float gamma;



int readImage(char* file) {



	ImageInput *in = ImageInput::open(file);

	if (!in) {

		std::cerr << "Could not open file: " << geterror() << std::endl;

		exit(-1);

	}



	const ImageSpec spec = in->spec();

	W = spec.width;

	H = spec.height;

	channels = spec.nchannels;

	

	//allocate memory for data structures

 	rawpix = new float[W*H*channels];

 	rawpix2 = new float[W*H*channels];

   

   mypix = new pixel*[H];

   mypix[0] = new pixel[W*H];

   for (int i = 1; i < H; i++)

   	mypix[i] = mypix[i-1] + W;



   output = new pixel*[H];

   output[0] = new pixel[W*H];

   for (int i = 1; i < H; i++)

   	output[i] = output[i-1] + W;



   logLwArray = new float*[H];

   logLwArray[0] = new float[W*H];

   for (int i = 1; i < H; i++)

   	logLwArray[i] = logLwArray[i-1] + W;

	

	//read image data

	if (!in->read_image(TypeDesc::TypeFloat, rawpix)) {

      std::cerr << "Could not read pixels from " << file;

      std::cerr << ", error = " << in->geterror() << std::endl;

      delete in;

      exit(-1); 

   }

   

   //'flip' image data and store in 2d array, also add 4th channel if needed

	for (int row = 0; row < H; row++) {

		for (int col = 0; col < W; col++) {

			mypix[row][col].r = rawpix[channels*((H-row)*W + col)];

			mypix[row][col].g = rawpix[channels*((H-row)*W + col)+1];

			mypix[row][col].b = rawpix[channels*((H-row)*W + col)+2];

			mypix[row][col].a = 1.0; 

		}

	}



	//copy contents of original image into output for intial display

	for (int i = 0; i < H; i++) {

		for (int j = 0; j < W; j++) {

			output[i][j] = mypix[i][j];

		}

	}

   

   if (!in->close()) {

      std::cerr << "Error closing " << file;

      std::cerr << ", error = " << in->geterror() << std::endl;

      delete in;

      exit(-1);

   }

   delete in;



   return 0;

}



//read pixel data from display and write to specified file

//input: file to write to

int saveImage(char* filename) {

	ImageOutput *out = ImageOutput::create(filename);

	if (!out) {

		std::cerr << "Could not create: " << geterror();

		exit(-1);

	}



	//read pixel data from display, store in 2d array

	glReadPixels(0,0,W,H,GL_RGBA,GL_FLOAT,output[0]);

	

	//'flip' image data 

	for (int row = 0; row < H; row++) {

   	for (int col = 0; col < W; col++) {

   		mypix[row][col].r = output[H-row-1][col].r;

			mypix[row][col].g = output[H-row-1][col].g;

			mypix[row][col].b = output[H-row-1][col].b;

			mypix[row][col].a = output[H-row-1][col].a;

   	}

   }

   

	//open file according to image specs, write image data to file

	ImageSpec newspec (W,H,4,TypeDesc::TypeFloat);

	out->open(filename,newspec);

	out->write_image(TypeDesc::TypeFloat,mypix[0]);



	if (!out->close()) {

		std::cerr << "Error closing " << filename;

		std::cerr << ", error = " << out->geterror() << std::endl;

		delete out;

		exit(-1);

	}

	delete out;

	

	return 0;

}



void convolve() {

	float red;

	float green;

	float blue;



	//compute convolution

	for (int row = 0; row < H; row++) {

		for (int col = 0; col < W; col++) {

			red = 0.0; green = 0.0; blue = 0.0;

			

			for (int frow = -filtSize/2; frow <= filtSize/2; frow++) {

				for (int fcol = -filtSize/2; fcol <= filtSize/2; fcol++) {

					if (row+frow >= 0 && row+frow < H && col+fcol >= 0 && col+fcol < W) {

						red += filtArray[frow+(filtSize/2)][fcol+(filtSize/2)] * output[row+frow][col+fcol].r;

						green += filtArray[frow+(filtSize/2)][fcol+(filtSize/2)] * output[row+frow][col+fcol].g;

						blue += filtArray[frow+(filtSize/2)][fcol+(filtSize/2)] * output[row+frow][col+fcol].b;

					} 

				}

			}

			

			//clamp values

			red = std::max((float)0.0, std::min(red, (float)1.0));

			green = std::max((float)0.0, std::min(green, (float)1.0));

			blue = std::max((float)0.0, std::min(blue, (float)1.0));

			

			output[row][col].r = red;

			output[row][col].g = green;

			output[row][col].b = blue;

		}

	}

}



void tonemap() {

	float red, green, blue;

	

	for (int row = 0; row < H; row++) {

		for (int col = 0; col < W; col++) {

			red = output[row][col].r;

			green = output[row][col].g;

			blue = output[row][col].b;

			Lw = (1/61.0) * ((20.0 * red) + (40.0 * green) + blue);

			logLwArray[row][col] = log(Lw);

			logLd = gamma * logLwArray[row][col];

			Ld = exp(logLd);

			output[row][col].r = (Ld/Lw) * red;

			output[row][col].g = (Ld/Lw) * green;

			output[row][col].b = (Ld/Lw) * blue;

		}

	}



	if (toneConvolve) {

		

	}

}



//reset output pixels to original pixel data

void switchImages() {

	if (s % 2 == 0) {

		for (int i = 0; i < H; i++) {

			for (int j = 0; j < W; j++) {

				output[i][j] = mypix[i][j];

			}

		}

	} else {

		for (int i = 0; i < H; i++) {

			for (int j = 0; j < W; j++) {

				mypix[i][j] = output[i][j];

			}

		}

	}

}



//displays image to screen

void display() {

	glClear(GL_COLOR_BUFFER_BIT);

	glDrawPixels(W,H,GL_RGBA,GL_FLOAT,output[0]);

	glFlush();

}



void handleKey(unsigned char key, int x, int y) {

	switch(key) {					

		case 'q':

		case 'Q':

		case 27:

			exit(0);

			

		case 's':

		case 'S':

			switchCount++;

			switchImages();

			glutPostRedisplay();

			break;

			

		case 'w':

		case 'W':

			if (allowWrite)

				saveImage(writeFile);

			else

				std::cout << "The optional image output file was not provided." << std::endl;

			break;

			

		default:

			return;

	}

}



//initializes GL window and calls display and key event functions

//input: command line arguement values

int main(int argc, char** argv) {

	

	//checking for second file arguement in order to allow image write functionality

	if (argc == 4 || argc == 5 || argc == 6) {

		if (argc == 5) {

			writeFile = argv[4];

			allowWrite = true;

		} else if (argc == 6) {

			writeFile = argv[5];

			allowWrite = true;

		}

		if (argv[4] == "-c") {

			toneConvolve = true;

		}

		gamma = argv[3];

	} else {

		std::cerr << "Usage: ./tonemap 'input image file' -g 'gamma' 'optional -c' 'optional output image file'" << std::endl;

		exit(-1);

	}	



	filtArray = new float*[filtSize];

   filtArray[0] = new float[filtSize*filtSize];

   for (int i = 1; i < filtSize; i++)

   	filtArray[i] = filtArray[i-1] + filtSize;



	//store data from file and compute sum

   for (int row = 0; row < filtSize; row++) {

		for (int col = 0; col < filtSize; col++) {

			filter >> filtArray[row][col];

			if (filtArray[row][col] > 0)

				sum += filtArray[row][col];

		}

	}

	

	//reflect filter

	for (int row = 0; row < filtSize; row++) {

		for (int col = row+1; col < filtSize; col++) {

			float temp = filtArray[row][col];

			filtArray[row][col] = filtArray[col][row];

			filtArray[col][row] = temp;

		}

	}



	//scale filter

	for (int row = 0; row < filtSize; row++) {

		for (int col = 0; col < filtSize; col++) {

			filtArray[row][col] /= sum;

		}

	}

	

	//readImage data with first file arguement and initialize GL display window

	readImage(argv[2]);

	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);

	glutInitWindowSize(W,H);

	glutCreateWindow("Assignment 4");



	//facilitate functionality: display, keyboard presses, mouse clicks

	glutDisplayFunc(display);

	glutKeyboardFunc(handleKey);

	

	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();

	gluOrtho2D(0, W, 0, H);

	glClearColor(1,1,1,1);

	

	glutMainLoop();

	return 0;	

}
