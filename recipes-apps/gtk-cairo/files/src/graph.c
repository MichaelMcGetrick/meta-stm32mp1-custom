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

#include "graph.h"


#ifdef GTK
extern GdkRGBA bck_color;                 
extern GdkRGBA ori_line_color;
#endif

struct legend_info info[NUM_PLOTS];
char buf[255];
float *pData;
int refCnt = 1;
float *PLOT_BUFFER, *PLOT_COPY;

double vavg = 0.0;
double vamp = 0.0;
double time_div = 0.0;
double freq = 0.0;
double vavgCopy = 0.0;
double vampCopy = 0.0;
double time_divCopy = 0.0;
double freqCopy = 0.0;
double vmax = 0.0;
double vmin = 0.0;



//PLOTTING ATTRIBUTES
//Simulator incrementor
int j=0;
//Plotting control flag
bool plotStart = false;
bool legend_flg = true;
bool mSuppress = false;
float *dataX; 
float *dataY;


//GRAPH FUNCTIONS -------------------------------------------------------------
void plotTrace(int color)
{
	  	  
	  vmax = -100.0;
	  vmin = +100.0;
	  
	 
	 changePlotMode(PLOT1_MODE);
	 lineWidth ( PLOT1_LINE_WIDTH ); //for line plots
	 if(REFRESH_MODE == 0)
	 {
		 double sum = 0.0;
		 //int trial_window = 0.26 * DATA_LEN;//0.2 * DATA_LEN;//30; // Optimise for other frequency ranges
		 int trial_window = 0.02 * DATA_LEN;
		 int trial_cnt = 0;
		 float vmax_pos1 = 0;
		 float vmax_pos2 = 0;
		 bool vmax1_fix = true;
		 bool vmax2_fix = false;
		
		  //Redefine the data:
		  if (GRAPH_MODE == 0)
		  {
			   //Redefine plot with simulated / ref data 
			   trial_window = 0.01 * DATA_LEN;
			   simData();
		  }   
		 		 
		 for(int i= 0;i < DATA_LEN;i++)
		 { 
			
			if(color == PLOT1_COLOR)
			{
				plotSig(PLOT_BUFFER[i],i,color);
										
			}	
			if(color == BCK_COLOR)
			{
				plotSig(PLOT_COPY[i],(float) i,color);  
			}   
			
			sum += PLOT_BUFFER[i];
			if(PLOT_BUFFER[i] > vmax)
			{
				vmax = PLOT_BUFFER[i];	
			}
			if(PLOT_BUFFER[i] < vmin)
			{
				vmin = PLOT_BUFFER[i];	
			}
			 
			if (i > 1)
			{
				// Perform calculations for frequency measurement
				if (vmax1_fix)
				{
					if (trial_cnt == 0)
					{
						if (PLOT_BUFFER[i] < PLOT_BUFFER[i-1])
						{								
							vmax_pos1 = (float) i;
							trial_cnt ++;
						}
							
					}	
					else
					{
						if( trial_cnt == trial_window)
						{
							if ((PLOT_BUFFER[i] < PLOT_BUFFER[(int)vmax_pos1]) )
							{
									// We have fixed first vmax position - switch off flag
									vmax1_fix = false;
									vmax2_fix = true;
									trial_cnt = 0;
									
							}	
							else
							{
								// False alarm:
								trial_cnt = 0;	
								vmax_pos1 = 0;
							}	
						}	
						else
						{
							trial_cnt ++;
						}		
					}	
				}
				else if (vmax2_fix)
				{
					if (trial_cnt == 0)
					{
						if (PLOT_BUFFER[i] > PLOT_BUFFER[i-1])
						{								
							vmax_pos2 = (float) i;
							trial_cnt ++;
						}
							
					}	
					else
					{
						if( trial_cnt == trial_window)
						{
							if ((PLOT_BUFFER[i] > PLOT_BUFFER[(int)vmax_pos2]) )
							{
									// We have fixed half point of cycle - switch off flag
									vmax_pos2 = vmax_pos2 + (vmax_pos2-vmax_pos1);
									vmax1_fix = false;
									vmax2_fix = false;
									trial_cnt = 0;
									
							}	
							else
							{
								// False alarm:
								trial_cnt = 0;	
								vmax_pos2 = 0.0;
								
							}	
						}
						else
						{	
							trial_cnt ++;
						}		
					}
					
				}		
					
			}	 
				  
		 }// i
			  
		 vavg = sum / DATA_LEN;
		 vamp = (vmax - vmin) / 2.0;
		 time_div = ( (1.0 / MAX_SIG_FREQ) * NUM_CYCLES) / X_INT;	
		 // Handle frequency estimate:
		 float period;
		 if ( (vmax_pos2 - vmax_pos1) < 0.0)
		 {
			 freq = NAN;
		 }	 
		 else
		 {
			period =  ( (vmax_pos2 - vmax_pos1) * NUM_CYCLES) / (MAX_SIG_FREQ * DATA_LEN);
			if(period == 0)
			{
				freq = NAN;	
			}	
			else
			{
				freq = 1.0 / period;
			}	
		 }		 	  
					 
		 //temp redefinition
		 //time_div = vmax_pos1;
		 //freq = vmax_pos2;			 
					  
		 setLinePlotFlag(0); //Ensure we do not use last position of current plot to draw (lineplot)
	 
	  }//if(REFRESH_MODE == 0)
	  if(REFRESH_MODE == 1)
	 {
		 for(int i= 0;i < DATA_LEN;i++)
		 { 
			
			if(color == PLOT1_COLOR)
			{
				plotSig(PLOT_BUFFER[i],i,color);	
			}	
			if(color == BCK_COLOR)
			{
				plotSig(PLOT_COPY[i],(float) i,color);  
			}       
		 }
		  
		 setLinePlotFlag(0); //Ensure we do not use last position of current plot to draw (lineplot)
	 
	  }//if(REFRESH_MODE == 1)
	 
	 	 
     
}//plotTrace

void simData()
{
	//Define simulation (ref) data
	for(int i=0;i<DATA_LEN;i++)
	{
		float T = 200;
		float val = (2*3.142*(i+j))/T;
		PLOT_BUFFER[i] = sin(val);		
	}
}	

void refreshCanvas()
{ 
   plotTrace(BCK_COLOR);
   if(SHOW_GRID)
  {
      //graduations();
  }
  
  if(show_ori_line)
  {
#ifdef LIBGRAPH
      setcolor(ORI_LINE_COLOR);
#else
	//TBD for GTK
#endif      
      drawOriginLine();
  }    
    
  

   
   
}//refreshCanvas   


void refreshGraphBackground ()
{
 
	 drawGraphBackground ();
	
}// refreshGraphBackground	




void copyDataBuf()
{
	int k;
	for(k=0;k < DATA_LEN;k++)
	{
			PLOT_COPY[k] = PLOT_BUFFER[k];
	}	
	
}//copyDataBuf	

void copyLegendData()
{
	//Hold copies of previous version for graphic refresh purposes
	vavgCopy = vavg;
	vampCopy = vamp;	
	time_divCopy = time_div;
	freqCopy = freq;
}	


#ifdef LIBGRAPH
void removeGrid()
{
   graduations(BCK_COLOR);
}   
#endif
#ifdef GTK
void removeGrid()

{
   graduations(bck_color);
}   
#endif


//-----------------------------------------------------------------------
