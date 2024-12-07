#ifndef GRAPH_H
#define GRAPH_H

/*
 =======================================================================
 Name        : graph.c
 Author      : Michael McGetrick
 Version     :
 Copyright   : 
 Description : Graph module to set up graphing for animated plotting 
 	 	 	       	 	 	         
 	 	 	    
 =======================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <stdint.h>
#include <signal.h>

#include <fcntl.h> 		//File Control Definitions
#include <time.h>

#include "graphic_lx.h"
//#include "graph.h"
#include "mcp3008.h"
#include <math.h>


//float SAMPLE_RATE;

//USER DEFINES: ------------------------------------------------------------
#define SAMPLE_MODE		0			//0: Delay; 1: fast looping - no delay
#define SAMPLE_RATE	   2000.0f //1000.0f   //Define required value for bsm delay usage
											 //Fast looping (below figures based on metric tests):
											 //9500.0f if SPI Clock is 0.68MHz
											 //35000.0f if SPI Clock is 1.37MHz
											 	
#define MAX_SIG_FREQ		200.0f   //Signal bandwidth or frequency for single frequency
#define NUM_CYCLES			3.0f		//Number of Cycles to display (maximum frequency) 
#define DISPLAY_LEN  		100 //100 //(int) round( (SAMPLE_RATE/MAX_SIG_FREQ)*NUM_CYCLES )  //SAMPLE_RATE //(int)((float)SAMPLE_RATE/(float)MAX_SIG_FREQ)*(float)NUM_CYCLES   //Num samples to display in trace
#define DATA_LEN  		100 //	DISPLAY_LEN //1000 //This is also length of x axis on plot
#define REFRESH_MODE		0	//Plot refresh mode:   0: Per new sample; 1: Data buffer complete
#define REFRESH_CNT     	1.5*SAMPLE_RATE   //Count at which to refresh graph
#define GRAPH_MODE		1   	//0: Simulation data; 1: Real data from Serial Port      
#define DATA_SKIP		0//500  //Define the number of incoming data samples to ignore before string plotting 
							//Avoids any sprious data coming through on initial startup/reset
							
#define DC_OFFSET		0 //-1.6//-1.65 + 0.20//-2.25//-2.5//Define offset to match dc offset provided in MCU							
									//There appears to be 0.25 difference in expected value (could be down to resistor error tolerance)

//GTK Specific
#define GPH_REFRESH_RATE	1 //1 //25 //60 //2//5 //60 Hz


//GRAPH PLOTTING:
//#define TRACE_COLOR   YELLOW//2             //Color of plot trace:2->green


//GRAPH PLOTTING: -----------------------------------------------------------------------------
#define X_WIN_SIZE   800 //600//800      //NB: These appear to be fixed for Linux implementation
#define Y_WIN_SIZE   400   

#define GRAPH_TITLE  "EEG SIMULATION SIGNAL"
#define X_LABEL      "Time"
#define Y_LABEL      "VOLTS"

#define BCK_COLOR       BLACK             //Color of graph background (Options:0: black bck,white grid; 15: White bck, black grid)
#define ORI_LINE_COLOR	WHITE //8 //DARKGRAY				//Colour of the horizontal origin line 
#ifdef GTK
extern GdkRGBA bck_color;                 
extern GdkRGBA ori_line_color;
extern GdkRGBA ver_marker_color;
#endif

//Legend data boxes
#define LEGEND_NUM_BOXES	4
#define LEGEND_LEFT			38			// Start left position for legend boxes 
#define LEGEND_TOP			446		// Top position for legend boxex
#define LEGEND_BCK_COLOR	GRAY



#define NUM_PLOTS 	1    //This is the number of plots appearing on the graph
#define PLOT_MODE    LINE //CIRCLE_FILLED//CIRCLE_FILLED //SQUARE_BORDER //2
#define LOG_MODE    0

#define PLOT1_MODE	LINE
#define PLOT2_MODE	LINE
#define PLOT3_MODE	SQUARE_BORDER
#define PLOT4_MODE	SQUARE_BORDER


#define PLOT1_COLOR   YELLOW//1              
#define PLOT2_COLOR   BLACK//1
#define PLOT3_COLOR   BLUE//1
#define PLOT4_COLOR   BLUE//4					


// Defines for Line Plot widths (pixels)
#define PLOT1_LINE_WIDTH	3
#define PLOT2_LINE_WIDTH	5
#define PLOT3_LINE_WIDTH	1
#define PLOT4_LINE_WIDTH	1

//Define for symbol sizes (circle, square, triangle)
#define SYMBOL_SIZE			0


#define PLOT1_SUPPRESS	false // Suppress the plot output
#define PLOT2_SUPPRESS	true // Suppress the plot output
#define PLOT3_SUPPRESS	true // Suppress the plot output
#define PLOT4_SUPPRESS	true // Suppress the plot output


extern struct legend_info info[NUM_PLOTS];

//NB - Ensure we do not define 0 for log plots
#define Y_MIN        	-1.0
#define Y_MAX        	3.0
#define X_MIN        	0.0//0.90
#define X_MAX        	DISPLAY_LEN


#define X_INT        	10
#define Y_INT        	4 //10
#define G_INT        	8


#define SHOW_GRID    	true  //Show gradations 
#define X_GRAD       	true  // Show X graduations 
#define Y_GRAD       	true  // Show Y graduations
#define X_SHOW       	true  // Show X-axis scale
#define Y_SHOW       	true  // Show Y-axis scale
#define X_LABEL_PREC		1		//X-axis decimal precision for number labelling
#define Y_LABEL_PREC		1		//Y-axis decimal precision for number labelling
#define ORI_LINE     	false    //Draw the x axis origin line
#define BITMAP       	false


//Audio files:
#define AUDIOFILE      		"test.wav"
#define INI_AUDIOFILE   	"ini.wav"
#define NUM_CHANS       	1
#define AUDIO_SAMPLE_RATE   11000
#define BITS_PER_SAMPLE 	16
#define BITS_PER_SAMPLE 	16


//Legend
#define LEGEND_CAPTION1			"Vavg:                "
#define LEGEND_CAPTION2			"Vamp:                "
#define LEGEND_CAPTION3			"Ti / Di:             "
#define LEGEND_CAPTION4			"Freq:                "
#define LEGEND_PLOT_SYM1 		LINE
#define LEGEND_SYM1_COLOR		RED
#define LEGEND_PLOT_SYM2 		LINE
#define LEGEND_SYM2_COLOR		CYAN
#define LEGEND_PLOT_SYM3 		CIRCLE_FILLED
#define LEGEND_SYM3_COLOR		CYAN
#define LEGEND_PLOT_SYM4 		CIRCLE_FILLED
#define LEGEND_SYM4_COLOR		CYAN


//bool legend_flg = true;  //Flag to include legend
extern bool legend_flg;  //Flag to include legend

// Draw vertical Marker (a vertical line at a specific x position)
#define DRAW_VER_MARKER		false
#define MARKER_WIDTH		 5 //Width in pixels 
#define V_MARKER_POS		100.00
#define V_MARKER_COLOR		YELLOW




							
//---------------------------------------------------------------------------------------------------------------------

extern char buf[255];
//float PLOT_BUFFER[DATA_LEN], PLOT_COPY[DATA_LEN];
extern float *PLOT_BUFFER, *PLOT_COPY;
extern float *pData;
extern int refCnt;

extern double vavg;
extern double vamp;
extern double vmax;
extern double vmin;
extern double time_div;
extern double freq ;
extern double vavgCopy,vampCopy,time_divCopy,freqCopy;


//PLOTTING ATTRIBUTES
//Simulator incrementor
extern int j;
//Plotting control flag
extern bool m_bPlot;
extern bool mSuppress;
//extern float dataX[DATA_LEN],dataY[DATA_LEN];
extern float *dataX, *dataY;
extern bool plotStart;


//FUNCTION DECLARATIONS -------------------------
void plotTrace(int color);
void simData();
void refreshCanvas();
void removeGrid();
void copyDataBuf();
void copyLegendData();
void refreshGraphBackground ();


//-----------------------------------------------------------------------
#endif
