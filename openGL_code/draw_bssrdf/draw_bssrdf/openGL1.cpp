/*
 * modified with an online code.
 * sample code with 3D scene. no shader.
 * use keyboard to control the user input. see function keyboard_callback
 */
#include <windows.h>
#include <string.h>
#include <GL\glew.h>
#include <GL\glut.h>
#include <iostream>
#include <fstream>
#include <math.h>
#define M_PI 3.14159265358979323846

#define SPHERE_RESOLUTION 64

#define RENDER_MODE_UI_ONLY 0
#define RENDER_MODE_2D      1
#define RENDER_MODE_3D      2
#define RENDER_MODE_COUNT   3

void dhsv2rgb(double h, double s, double v, 
              double *r, double *g, double *b) ;

typedef struct {
  float ti, g, r, ts, a,n; // useful information

}dataInput;
struct Colortype1{

  float rgb[3];
  };
float* imgoutput(int width, int height,  Colortype1 *pixel[]){

	 float* pixels = new float[width*height*3];
		for(int j=0;j<height;j++)
			for(int i=0;i<width;i++)
			{
				for(int x=0;x<3;x++){

					pixels[(j*width+i)*3+x]=pixel[width-1-i][j].rgb[x];
					//*colRange*2.0-colRange;
				}

			}
		return pixels;
}


GLfloat UpwardsScrollVelocity = -150.0;
float view=20.0;

char quote[7][80];
int numberOfQuotes=0,i;

int ww, wh;
int renderMode = RENDER_MODE_2D;

int prevMouseX, prevMouseY;
double view_rotx = 0.0, view_roty = 0.0;

GLuint shader3D;
GLuint brdfTexture;
float* displacements;

//*********************************************
//*  glutIdleFunc(timeTick);                  *
//*********************************************

void timeTick(void)
{
    if (UpwardsScrollVelocity< -600)
        view-=0.000011;
    if(view < 0) {view=20; UpwardsScrollVelocity = -10.0;}
    //  exit(0);
    UpwardsScrollVelocity -= 0.015;
  glutPostRedisplay();

}
dataInput userInput;
float* wholepic(dataInput userInput, int width,int height);
float* wholepicMono(dataInput userInput, int width,int height);
void PPMoutput(dataInput userInput, int width,int height);
void readFitData();
//FitData  lerp_user_input(dataInput user_input);

const int width=500;
const int height=500;

 void draw_legend(int l_height,int l_length,Colortype1 **legend_int)
{
	double r=0;
	double g = 0;
	double b = 0;
	double h = 0;
	double s = 0;
	double v = 0;
	for(int i=0;i<l_height;i++)
	{
		for(int j=0;j<l_length;j++)
		{
			double fj = j+1;
			double flength =l_length;
			h = fj/flength;
			s = 1;
			v = 1;
			dhsv2rgb(h,s,v,&r,&g,&b);
			legend_int[i][j].rgb[0] = r;
			legend_int[i][j].rgb[1] = g;
			legend_int[i][j].rgb[2] = b;
		}
	}

}
 
void DrawImg()
{

    glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	int l_height = 100;
	int l_length = 100;

    float* pixels = wholepic(userInput,width,height);
	Colortype1* legend_int[100];
	for(int i=0;i<l_height;i++)
		legend_int[i]=new Colortype1[l_length];

	draw_legend(l_height,l_length,legend_int);
	
	float *l_pixels = imgoutput(l_length,l_height,legend_int);

    glPixelStorei(GL_UNPACK_ALIGNMENT,2);
	glDrawPixels(width, height, GL_RGB,GL_FLOAT, pixels);

	glViewport(0, 0, width, height);
		
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0f, 1.0f, 0.0f, 1.0f);

	glMatrixMode(GL_MODELVIEW);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, brdfTexture);
	glColor3f(1.0f, 1.0f, 1.0f);

	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
	glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0f, 0.0f);
	glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0f, 1.0f);
	glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, 1.0f);
	glEnd();
	glFlush();

	glDisable(GL_TEXTURE_2D);
		
	glViewport(0, 0, ww, wh);

	glPixelStorei(GL_UNPACK_ALIGNMENT,3);
	glTranslatef(0,0,0);
	glDrawPixels(l_length,l_height,GL_RGB,GL_FLOAT,l_pixels);

}

void Draw3DMesh()
{
	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, width, height);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(
		0.0, 0.0, 3.0, 
		0.0, 0.0, 0.0, 
		0.0, 1.0, 0.0
	);
	glRotated(view_rotx, 1.0, 0.0, 0.0);
	glRotated(view_roty, 0.0, 1.0, 0.0); 
	glRotated(90.0, 1.0, 0.0, 0.0);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (double)width/(double)height, 0.1, 100.0); 

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, brdfTexture);
	glColor3f(1.0f, 1.0f, 1.0f);

	glBegin(GL_QUADS);
    for (int i = 0; i < SPHERE_RESOLUTION/2; i++)
    {
        for (int j = 0; j < 2*SPHERE_RESOLUTION; j++)
        {
            // theta: [-PI, PI]
            // phi: [0, PI/2]
            float theta1 = j*M_PI/SPHERE_RESOLUTION - M_PI;
            float theta2 = (j+1)*M_PI/SPHERE_RESOLUTION - M_PI;
            float phi1 = M_PI/2 - i*M_PI/SPHERE_RESOLUTION;
            float phi2 = M_PI/2 - (i+1)*M_PI/SPHERE_RESOLUTION;

			float displace;
			displace = displacements[
				max(min((int)floor((phi1/M_PI*sin(theta1)+0.5f)*height), height-1), 0)*width + 
					max(min((int)floor((phi1/M_PI*cos(theta1)+0.5f)*width), width-1), 0)];
            glTexCoord2f(phi1/M_PI*cos(theta1)+0.5f, phi1/M_PI*sin(theta1)+0.5f);
            glVertex3f(sin(theta1)*sin(phi1)*displace, cos(phi1)*displace, cos(theta1)*sin(phi1)*displace);
			
			displace = displacements[
				max(min((int)floor((phi2/M_PI*sin(theta1)+0.5f)*height), height-1), 0)*width + 
					max(min((int)floor((phi2/M_PI*cos(theta1)+0.5f)*width), width-1), 0)];
            glTexCoord2f(phi2/M_PI*cos(theta1)+0.5f, phi2/M_PI*sin(theta1)+0.5f);
            glVertex3f(sin(theta1)*sin(phi2)*displace, cos(phi2)*displace, cos(theta1)*sin(phi2)*displace);
			
			displace = displacements[
				max(min((int)floor((phi2/M_PI*sin(theta2)+0.5f)*height), height-1), 0)*width + 
					max(min((int)floor((phi2/M_PI*cos(theta2)+0.5f)*width), width-1), 0)];
            glTexCoord2f(phi2/M_PI*cos(theta2)+0.5f, phi2/M_PI*sin(theta2)+0.5f);
            glVertex3f(sin(theta2)*sin(phi2)*displace, cos(phi2)*displace, cos(theta2)*sin(phi2)*displace);
			
			displace = displacements[
				max(min((int)floor((phi1/M_PI*sin(theta2)+0.5f)*height), height-1), 0)*width + 
					max(min((int)floor((phi1/M_PI*cos(theta2)+0.5f)*width), width-1), 0)];
            glTexCoord2f(phi1/M_PI*cos(theta2)+0.5f, phi1/M_PI*sin(theta2)+0.5f);
            glVertex3f(sin(theta2)*sin(phi1)*displace, cos(phi1)*displace, cos(theta2)*sin(phi1)*displace);
        }
    }
	glEnd();
	glFlush();

	glMatrixMode(GL_MODELVIEW);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
}

//*********************************************
//* printToConsoleWindow()                *
//*********************************************

void printToConsoleWindow()
{
    int l,lenghOfQuote, i;

    for(  l=0;l<numberOfQuotes;l++)
    {
        lenghOfQuote = (int)strlen(quote[l]);

        for (i = 0; i < lenghOfQuote; i++)
        {
         // cout<<quote[l][i];
        }
        //  cout<<endl;
    }

}

void DrawUI()
{
	int l,lenghOfQuote, i;

	glMatrixMode(GL_MODELVIEW);
    glTranslatef(0.0, -100, UpwardsScrollVelocity);
    glRotatef(-20, 1.0, 0.0, 0.0);
    glScalef(0.1, 0.1, 0.1);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, 1.0, 1.0, 3200);

	glMatrixMode(GL_MODELVIEW);

    for(  l=0;l<numberOfQuotes;l++)
    {
        lenghOfQuote = (int)strlen(quote[l]);
        glPushMatrix();
        glTranslatef( 1000, (l*400), 0.0);
		glScalef(0.5,0.5,0);
		glLineWidth(0.1);
        for (i = 0; i < lenghOfQuote; i++)
        {
            glColor3f((UpwardsScrollVelocity/10)+300+(l*10),(UpwardsScrollVelocity/10)+300+(l*10),0.0);
			//glutStrokeWidth(GLUT_STROKE_ROMAN,10);
			glutStrokeCharacter(GLUT_STROKE_ROMAN, quote[l][i]);
			
        }
        glPopMatrix();
    }
}

//*********************************************
//* RenderToDisplay()                       *
//*********************************************

void RenderToDisplay()
{
    DrawUI();
    DrawImg();
}

void Render3D()
{
	DrawUI();
	Draw3DMesh();
}

//*********************************************
//* glutDisplayFunc(myDisplayFunction);       *
//*********************************************

void myDisplayFunction(void)
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0.0, 30.0, 100.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	if (renderMode == RENDER_MODE_UI_ONLY) DrawUI();
    if (renderMode == RENDER_MODE_2D) RenderToDisplay();
	else if (renderMode == RENDER_MODE_3D) Render3D();
    glutSwapBuffers();
}
//*********************************************
//* glutReshapeFunc(reshape);               *
//*********************************************

void reshape(int w, int h)
{
	ww = w; wh = h;
	glViewport(0, 0, w, h);
}

//*********************************************
//* int main()                                *
//*********************************************

void keyboard_callback( unsigned char key, int x, int y )
{
	if(key=='o') {
		PPMoutput(userInput, width,height);
		return;
	}

	renderMode = RENDER_MODE_UI_ONLY;

	switch ( key ) {

	   case 'y':
					userInput.ti-=M_PI/24;
					sprintf(quote[0],"theta i=%.1f",userInput.ti*180/M_PI);
			    	break;

	   case 't':
					userInput.ti+=M_PI/24;
					sprintf(quote[0],"theta i=%.1f",userInput.ti*180/M_PI);
					break;
	    case 'd':
				userInput.ts-=M_PI/24;
				sprintf(quote[1],"theta s=%.1f",userInput.ts*180/M_PI);
		    	break;

		case 's':
				userInput.ts+=M_PI/24;
				sprintf(quote[1],"theta s=%.1f",userInput.ts*180/M_PI);
				break;
		case 'r':
			userInput.r-=0.1;
			sprintf(quote[2],"r=%g",userInput.r);
	    	break;

		case 'e':
			userInput.r+=0.1;
			sprintf(quote[2],"r=%g",userInput.r);
			break;
		case 'b':
			if(userInput.a <= 0.01)
				userInput.a = userInput.a;
			else
			userInput.a-=0.1;

			sprintf(quote[3],"a=%g",userInput.a);
			break;

		case 'a':
			if(userInput.a+0.1 > 0.99)
				userInput.a = userInput.a;
			else
			userInput.a+=0.1;
			sprintf(quote[3],"a=%g",userInput.a);
			break;
		case 'h':
					if(userInput.g <= -0.9)
						userInput.g = userInput.g;
					else
					userInput.g-=0.1;

					sprintf(quote[4],"g=%g",userInput.g);
					break;

		case 'g':
					if(userInput.g >=0.99)
						userInput.g = userInput.g;
					else
					userInput.g+=0.1;

					sprintf(quote[4],"g=%g",userInput.g);
					break;

		case 'n':
					userInput.n-=0.1;
					sprintf(quote[5],"n=%g",userInput.n);
					break;

		case 'f':
					userInput.n+=0.1;
					sprintf(quote[5],"n=%g",userInput.n);
					break;
		case '2':
			renderMode = RENDER_MODE_2D;
			break;
		case '3':
			renderMode = RENDER_MODE_3D;
			break;

		case 033:  // octal ascii code for ESC

			exit( 0 );
			break;

    }

	if (renderMode == RENDER_MODE_2D || renderMode == RENDER_MODE_3D)
	{
		delete[] displacements;
		displacements = wholepicMono(userInput,width,height);

		glBindTexture(GL_TEXTURE_2D, brdfTexture);
		float* colorData = wholepic(userInput, width, height);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_FLOAT, colorData);
		delete[] colorData;
	}

	glutPostRedisplay();
}

void mouse_clicked(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		prevMouseX = x;
		prevMouseY = y;
	}
}

void mouse_dragged(int x, int y) 
{
	if (prevMouseX >= 0 && prevMouseY >= 0)
	{
		double thetaY, thetaX;
		thetaY = 360.0 *(x - prevMouseX)/ww;    
		thetaX = 360.0 *(y - prevMouseY)/wh;
		view_rotx += thetaX;
		view_roty += thetaY;
		if (view_rotx > 90.0) view_rotx = 90.0;
		else if (view_rotx < -90.0) view_rotx = -90.0;
	}
	prevMouseX = x;
	prevMouseY = y;
	glutPostRedisplay();
}

void initialize()
{
	displacements = wholepicMono(userInput,width,height);

	glGenTextures(1, &brdfTexture);
	glBindTexture(GL_TEXTURE_2D, brdfTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	float* colorData = wholepic(userInput, width, height);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_FLOAT, colorData);
	delete[] colorData;
}

void cleanup()
{
	glDeleteTextures(1, &brdfTexture);
	delete[] displacements;
}

int main( int argc, char **argv )
{
	readFitData();

	userInput.ti=60*M_PI/180;
	userInput.ts=60*M_PI/180;
	userInput.r=0.8;
	userInput.a=0.9;
	userInput.g=-0.3;
	userInput.n=1.4;
	glutInit( &argc, argv );
	strcpy(quote[0],"theta i=60");
    strcpy(quote[1],"theta s=60");
    strcpy(quote[2],"r=0.8");
    strcpy(quote[3],"a=0.9");
    strcpy(quote[4],"g=-0.3");
    strcpy(quote[5],"n=1.4");
    numberOfQuotes=6;

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(700, 500);
    glutCreateWindow("BSSRDF");
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glLineWidth(3);

    glutDisplayFunc(myDisplayFunction);
    glutReshapeFunc(reshape);
    glutKeyboardFunc( keyboard_callback );
	glutMouseFunc(mouse_clicked);
	glutMotionFunc(mouse_dragged);
   // glutIdleFunc(timeTick);

	initialize();
    glutMainLoop();
	cleanup();

    return 0;
}

