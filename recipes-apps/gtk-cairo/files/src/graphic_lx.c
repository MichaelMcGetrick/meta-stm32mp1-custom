/*-----------------------------------------------------------------------------
   AUTHOR:            M.J. McGetrick
                      (c) Copyright 2020 M.J. McGetrick
   DATE:
   DESCRIPTION:       Graphic_lx
                      Suite of functions for simple graph plotting using
                      the Linux Turbo C graphics library.
    
------------------------------------------------------------------------------*/
#include "graphic_lx.h"


#include <stdio.h>
#include <math.h>
#include <string.h>

#ifdef LIBGRAPH
//---------------------------------------------------------------------------
  void Graphic_Init(int xFormSize,int yFormSize,int pMode,int logMode,
               char *xlab, char *ylab, char *gphtitle,
               float yMin, float yMax, float xMin, float xMax,
               int xInt, int yInt, bool xGrad, bool yGrad, int XAxisPrec, int YAxisPrec,
               bool grid, int gInt,
               bool xshow,bool yshow,bool ori, int ori_color, bool bitmap,
               int monobcolor,char *audfile, char *audInifile,
               int chans,int samrate, int bps)
{
   
   
     
   
   //Initialise Turbo C Graphics Library
   int gd = DETECT,gm;
   initgraph(&gd,&gm,NULL);
   
      
   X_FORM_SIZE = xFormSize;
   Y_FORM_SIZE = yFormSize;
   
     
  m_bAnimated = false;
  m_bResize = false;
  
   //Set scales ranges 
   ymin1 = yMin;
   ymax1 = yMax;
   xmin = xMin;
   xmax = xMax;


   //Save linear definitions (in case transformed to log values):
   xminLinear = xmin;
   xmaxLinear = xmax;

   //Get flag for plotting harmonics on frequency spectrum
   m_bHarmonic = false;

   //Set audio player members:
   strcpy(m_szAudioIniFile,audInifile);
   strcpy(m_sFile,audfile);
   m_iNumChannels = chans;
   m_iSampleRate = samrate;
   m_iBitsPerSample = bps;

   
   //Check for bitmap option:
   Bitmap = bitmap;

   //Define plot mode:
   plotMode = pMode;  //0:line, 1:pixel, 2:circle, 3:rectangle, 4:triangle
   logPlotMode = logMode;  //0: default - no log plotting
                          //1: log plot: x axis
                          //2: log plot: y axis
                          //3: log plot: x and y axis

        
      
   image = 1;

   
   xincrement = 1; //Default value for pixel plot advances
   cirSize = 5;    //Initialise circle size for circle plots
   
   
   setLinePlotFlag(0);

   
   //Set  axis numbering defaults:
   showXAxisNums = xshow;
   showYAxisNums = yshow;
   

   //Set title:
   strcpy(Title,gphtitle);
   //Set axis labels:
   strcpy(xLabel,xlab);
   strcpy(yLabel,ylab);

   //Set flags for graduations:
   xgrad = xGrad;
   ygrad = yGrad;
   xnoDivs = xInt;
   ynoDivs = yInt;

   showGrid = grid;
   //showXGradLines = xshow;
   //showYGradLines = yshow;
	showXGradLines = xGrad;
   showYGradLines = yGrad;

   if(logPlotMode == 1)
   {
      xmin = log10(xmin);
      if(xmax!= 0)
         xmax = log10(xmax);
   }
   if(logPlotMode == 2)
   {
      ymin1 = log10(ymin1);
      if(ymax1!= 0)
         ymax1 = log10(ymax1);
   }
   if(logPlotMode == 3)
   {
      ymin1 = log10(ymin1);
      if(ymax1!= 0)
         ymax1 = log10(ymax1);
      xmin = log10(xmin);
      if(xmax!= 0)
         xmax = log10(xmax);
   }
   
   
   //Set up test rectangle:
   //rectangle(0,0, X_FORM_SIZE,Y_FORM_SIZE);
   //Initialise background:
   setbkcolor(WHITE);
   
  //Set up display graphic area:  -this is the outer rectangle for entire graph (plot + labels)
  setupGraphSize();


   
   //Color background for mono graph:
   setMonoGraphBackground(monobcolor);
   m_cMonoBackCol = monobcolor;

   
   setupWindow(image,xmargin,ymargin,X_FORM_SIZE-xmargin,
               Y_FORM_SIZE-ymargin,xLabel,yLabel);


    
  counter1 = 0.0;
  
  // Define decimal precision for number labelling on x and axes:
  char prec = XAxisPrec + '0';
  xfield[0] = '5';
  xfield[1] = '.';
  xfield[2] = prec;
  
  prec = YAxisPrec + '0';
  yfield[0] = '5';
  yfield[1] = '.';
  yfield[2] = prec;
  
    
  //Draw graduations:
  if(showGrid)
  {
      if(m_cMonoBackCol == BLACK)
      {
         graduations(DARKGRAY);
      }
      if(m_cMonoBackCol == WHITE)
      {
         graduations(BLACK);
      }   
  }
  
      
  // Set default for the line width plot (in pixels)
  line_width = 1;    
  lineFlag = 0; //Initiliase for start of line plots
  
      
  //Draw axis scales:
  if(!Bitmap) axisScales();
  
  //Draw origin line:
  show_ori_line = ori;
  
  if(show_ori_line)
  {
      setcolor(ori_color);   
      drawOriginLine();
  }    
       

}

//---------------------------------------------------------------------------
void setupGraphSize()
{
      //Set up outer rectangle window:
      
      xmargin = 30;
      ymargin = 35;

      left = xmargin;
      left1 = left;
      top = ymargin;
      top1 = top;
      right = X_FORM_SIZE-xmargin;
      right1 = right;
      bottom = Y_FORM_SIZE-ymargin;
      bottom1 = bottom;
       


}//setupGraphSize

//---------------------------------------------------------------------------
void setMonoGraphBackground(int color)
{
   
   //Set draw color:
   if(color == 0)
   {
      setcolor(color);
      rectangle(xmargin,ymargin,X_FORM_SIZE-xmargin,Y_FORM_SIZE-ymargin);
      //Set fill color:
      floodfill(xmargin+10, ymargin+10,color);
   }
   if(color == 15)
   {    
      setcolor(0);   //Provide black border
      rectangle(xmargin,ymargin,X_FORM_SIZE-xmargin,Y_FORM_SIZE-ymargin);
      //Set fill color:
      setcolor(color);
      floodfill(xmargin+10, ymargin+10,color);
    }

}//setMonoGraphBackGround

//---------------------------------------------------------------------------
void setupWindow(int image,int left,int top,int right,
                             int bottom, char *xlabel, char *ylabel)
{
   
   //Print title and axis labels:
   title(Title);
   xAxisText(image,xlabel);
   yAxisText(image,ylabel);


}//setupWindow


//---------------------------------------------------------------------------
void title(char *text)
{
   
   int len = X_FORM_SIZE;
   int txtWidth = textwidth(text);
   int centralPos = (len-txtWidth)/2;
   int horPos = top-25;
   
   //Print graph title 
   setfontcolor(DARKGRAY);
   outtextxy(centralPos,horPos,text);

}//title

//---------------------------------------------------------------------------
void xAxisText(int image,char *text)
{

   int len = X_FORM_SIZE;
   int txtWidth = textwidth(text);
   int centralPos = (len-txtWidth)/2;
   int horPos = bottom1+18;
   
   //Print x axis label 
   setfontcolor(DARKGRAY);
   outtextxy(centralPos,horPos,text);



}//xAxisText

//---------------------------------------------------------------------------
void yAxisText(int image,char *text)
{
      int len = bottom1-top1;
      int txtHeight = textheight(text);
      int verlen = txtHeight*strlen(text);
      int centralPos = (len-verlen)/2;
      int pos = centralPos + ymargin;
      
      //Print y axis label
      char str[2];
      str[1] = '\0';
      setfontcolor(DARKGRAY);
      for(int i=0;i<strlen(text);i++)
      {
         str[0] = text[i];
         
         outtextxy(1,pos,str);
         pos = pos+15;
      }//i
      
}//yAxisText



//---------------------------------------------------------------------------
void tagOrigin(int image,int left,int posOrig)
{

   //Mark horizontal position of origin:
   setcolor(0);
   moveto(left-5,C1);
   lineto(left,C1);

}//tagOrigin

//---------------------------------------------------------------------------
void graduations(int color)
{
   int Image = 1;   //Set for mono graph plot:
   
   setcolor(color);
   
   if(showXGradLines)
   {
      //Obtain vertical graduations:
      if(logPlotMode == 1 || logPlotMode == 3)
      {
         //log graduations:
         for(int i=xmin;i<=xmax-1; i+=1)
         {
             int x = positionPixel(Image,xmin,xmax,i,1);
             pixelVerLine(x,4);
             for(int n=2;n<10;n++)
             {
               //float pos = log10( n*pow10(i) );
               float pos = log10( n*pow(10,i) );
               x = positionPixel(Image,xmin,xmax,pos,1);
               pixelVerLine(x,4);
             }//n
         }//i

      }
      else
      {  //linear graduations
         if(xgrad)
         {
            float interval = (xmax-xmin)/xnoDivs;
            float pos = xmin;
            for(int i=0;i<=xnoDivs;i++)
            {
               int x = positionPixel(Image,xmin,xmax,pos,1);
               pixelVerLine(x,4);
               pos += interval;
            }//i
         }//if

      }
   }//if(showXGradLines)
   
   if(showYGradLines)
   {
       //Obtain horizontal graduations:
      if(logPlotMode == 2 || logPlotMode == 3)
      {
         for(int i=ymin1;i<=ymax1-1;i+=1)
         {
            int y = positionPixel(Image,ymin1,ymax1,i,0);
            pixelHorLine(y,3);
            for(int n=2;n<10;n++)
            {
               //float pos = log10( n*pow10(i) );
               float pos = log10( n*pow(10,i) );
               y = positionPixel(Image,ymin1,ymax1,pos,0);
               pixelHorLine(y,3);
            }//n
         }//i
      }
      else
      {
         if(ygrad)
         {
            float interval = (ymax1-ymin1)/ynoDivs;
            float pos = ymin1;
            for(int i=0;i<=ynoDivs;i++)
            {
               int y = positionPixel(Image,ymin1,ymax1,pos,0);
               pixelHorLine(y,3);
               pos += interval;
            }//i
         }//if
      }
   }//if(showYGradLines

}//graduations

//---------------------------------------------------------------------------
void plotSig(float ySig, float xSig,int color)
{
   if(logPlotMode == 1) xSig = log10(xSig);
   if(logPlotMode == 2) ySig = log10(ySig);
   if(logPlotMode == 3)
   {
      xSig = log10(xSig);
      ySig = log10(ySig);
   }
   int ypixel = positionPixel(1,ymin1,ymax1,ySig,0);
   int xpixel = positionPixel(1,xmin,xmax,xSig,1);
   
   setcolor(color);
   if(plotMode == TRIANGLE_BORDER)
   {
		triangle(xpixel,ypixel,cirSize,TRIANGLE_BORDER);	
	}	
	if(plotMode == TRIANGLE_FILLED)
   {
		triangle(xpixel,ypixel,cirSize,TRIANGLE_FILLED);	
	}	
   if(plotMode == CIRCLE_BORDER)
   {
		circle(xpixel,ypixel,cirSize);
		
	}	
   if(plotMode == CIRCLE_FILLED)
   {
      circle(xpixel,ypixel,cirSize);
      floodfill(xpixel+1,ypixel+1,color);
                            
   }
   if(plotMode == SQUARE_BORDER)
   {
      rectangle(xpixel-cirSize,ypixel-cirSize,xpixel+cirSize,ypixel+cirSize);
                              
   }
   if(plotMode == SQUARE_FILLED)
   {
      rectangle(xpixel-cirSize,ypixel-cirSize,xpixel+cirSize,ypixel+cirSize);
      floodfill(xpixel-cirSize+1,ypixel-cirSize+1,color);                        
   }
    
   if(plotMode == PIXEL)
   {
      moveto(xpixel,ypixel);
      lineto(xpixel,ypixel+1);
   }
   if(plotMode == LINE)
   {
      if(lineFlag>0)
      {
         // Check we have an odd number (for symmetry)
         if ( (line_width % 2) == 0 )
         {
				// Reduce width by 1 pixel
				line_width -= 1;	
			}	
         int span = (line_width - 1) / 2.0;
         
         for(int i = -span; i < span+1; i++)
			{
				moveto(sigStore1+i,sigStore2+i);
				lineto(xpixel+i,ypixel+i);
			}
				
      }
      sigStore1 = xpixel;
      sigStore2 = ypixel;
      lineFlag = 1;
   }
   if(ySig>ymaxVal1) ymaxVal1 = ySig;

}//plotSig  (for single screen plotting)


//---------------------------------------------------------------------------
void lineWidth(int width)
{
	line_width = width;
}

//---------------------------------------------------------------------------
void triangle(int x, int y, int size, int type)
{
	int a = (int) (2.0*size)/1.732;	
	
	//Define vertices of triangle:
	int points[] = {x-a,y+size,x+a,y+size,x,y-size,x-a,y+size};
	if(type == TRIANGLE_BORDER)
	{
		drawpoly(4,points);
	}
	if(type == TRIANGLE_FILLED)
	{
	  fillpoly(4,points);	
	}
	
}	

//---------------------------------------------------------------------------
int positionPixel(int image,float ymin,float ymax,float sig,
                              int pixelMode)
{
   int X,y;
   float R = ymax/(ymax-ymin);
   if(ymax==0) R = ymin/(ymin-ymax);
   if(pixelMode == 0)
   {                    //positioning along y axis
      if(image == 1) X = bottom1 - top1;
      else if(image == 2)
        X = bottom2 - top2;
      if(ymax==0) y = (fabs(sig)/(-ymin))*R*X;
      else
         y = (fabs(sig)/ymax)*R*X;
      if(sig>=0.0)
      {
         if(ymax != 0) y = (R*X)-y;
      }
      else
      {
         if(ymax != 0)
            y = y+(R*X);
      }
      y += ymargin;
   }//if
   if(pixelMode == 1)
   {                //positioning along x axis
      float R = ymax/(ymax-ymin);
      //if(ymax==0) R = ymin/(ymin-ymax);
      R = 1-R;
      if(image == 1) X = right1 - left1;
      else if(image == 2)
         X = right2 - left2;
      float orig = R*X;
      if(sig>=0.0)
      {
         if(ymax==0) y = (fabs(sig)/(-ymin))*(X-orig);
         else
            y = orig + (fabs(sig)/ymax)*(X-orig);
      }
      if(sig<0.0)
      {
         y = orig - (fabs(sig)/fabs(ymin))*orig;
      }
      y+= xmargin;
   }

   return y;

}//positionPixel


void plotVerLine(float ySig,float xSig)
{

   //This routine plots a vertical line (along +yaxis) of given height at x position specified

   //if(logPlotMode == 1) xSig = log10(xSig);
   if(logPlotMode == 2) ySig = log10(ySig);
   if(logPlotMode == 3)
   {
      xSig = log10(xSig);
      ySig = log10(ySig);
   }

   int yminpixel = positionPixel(1,ymin1,ymax1,0.0,0);
   int ymaxpixel = positionPixel(1,ymin1,ymax1,ySig,0);
   int xpixel = positionPixel(1,xmin,xmax,xSig,1);

   setcolor(15);
   moveto(xpixel,yminpixel);
   lineto(xpixel,ymaxpixel);
   

}//plotVerLine



//---------------------------------------------------------------------------
void pixelVerLine(int x, int pixelIncr)
{
   for(int j=top;j<=bottom;j+=pixelIncr)
   {
      moveto(x,j);
      lineto(x,j+1);
   }//j

}//pixelVerLine


//---------------------------------------------------------------------------
void pixelHorLine(int y, int pixelIncr)
{
   for(int j=left;j<=right;j+=pixelIncr)
   {
      moveto(j,y);
      lineto(j+1,y);
   }//j

}//pixelVerLine


//---------------------------------------------------------------------------
void axisScales()
{ 
   xAxisScale(); 
   yAxisScale(); //Only for mono-trace
}

//---------------------------------------------------------------------------
void xAxisScale()
{
   if(showXAxisNums)
   {
      char numtext[3], powtext[3];

      sprintf(numtext,"%d",10);
      int horPos = (bottom-top)+14;

      setfontcolor(DARKGRAY);
      //Obtain x axis scale:
      if(logPlotMode == 1 || logPlotMode == 3)
      {
         for(int i=xmin;i<=xmax;i+=1)
         {
            int x = positionPixel(1,xmin,xmax,i,1);
            sprintf(powtext,"%d",i);
            x = x-30;
            outtextxy(x+15,horPos+25,numtext);
            outtextxy(x+28,horPos+20,powtext);
         }//i

      }
      else
      {
         char floatnumtext[6], field[6];
         char *s1 = "%", *s2 = "f";
         strcpy(field,s1);
         strcat(field,xfield);
         strcat(field,s2);

         float interval = (xmax-xmin)/xnoDivs;
         float pos = xmin;
         for(int i=0;i<=xnoDivs;i++)
         {
            int x = positionPixel(1,xmin,xmax,pos,1);
            sprintf(floatnumtext,field,pos);
            x = x-30;
				outtextxy(x+15,horPos+25,floatnumtext);
				pos += interval;

         }//i

      }
   }//if(ShowXAxisNums)

}//xAxisScale


//---------------------------------------------------------------------------
void yAxisScale()
{

   if(showYAxisNums)
   {
      char numtext[3], powtext[3];

      sprintf(numtext,"%d",10);
      int verPos = left-85;

      setfontcolor(DARKGRAY);
      //Obtain y axis scale:
      if(logPlotMode == 2 || logPlotMode == 3)
      {
         for(int i = ymin1;i<=ymax1;i+=1)
         {
            int y = positionPixel(1,ymin1,ymax1,i,0);
            sprintf(powtext,"%d",i);
            outtextxy(verPos+60,y-5,numtext);
            outtextxy(verPos+75,y-10,powtext);
         }//i
      }
      else
      {
         char floatnumtext[6], field[6];
         char *s1 = "%", *s2 = "f";
         strcpy(field,s1);
         strcat(field,yfield);
         strcat(field,s2);

         float interval = (ymax1-ymin1)/ynoDivs;
         float pos = ymin1;
         for(int i=0;i<=ynoDivs;i++)
         {
            sprintf(floatnumtext,field,pos);
            int y = positionPixel(1,ymin1,ymax1,pos,0);
            outtextxy(verPos+55,y-6,floatnumtext);
            pos += interval;
         }//i

      }
   }//if(showYAxisNums)

}//yAxisScale


//---------------------------------------------------------------------------
void drawOriginLine()
{
    
  if(logPlotMode == 0 || logPlotMode == 1)
  {
      C1 = ( ymax1/(ymax1-ymin1) )* (bottom-top);
      C1 = C1 + ymargin;  //Add margin offset
      moveto(left,C1);
      lineto(right,C1);
      tagOrigin(image,left,C1);
  }
  
}//drawOriginLine


//---------------------------------------------------------------------------
void drawVerticalMarker(float pos, int color,int width)
{
	
	//int width: 0: standard 1 pixel; 1: 3 pixel width
		 
	// Draws a vertical marker line at the requested postion
	if(logPlotMode == 1 || logPlotMode == 3)
   {
      pos = log10(pos);

   }
	      
	if ((pos >= xmin) && (pos <= xmax))
	{ 
		C1 = ( (pos-xmin) / (xmax-xmin) )* (right-left);
		C1 = C1 + xmargin;  //Add margin offset
		
		setcolor(color);
					
		// Check we have an odd number (for symmetry)
		if ( (width % 2) == 0 )
		{
			// Reduce width by 1 pixel
			width -= 1;	
		}	
		int span = (width - 1) / 2.0;
		
		for(int i = -span; i < span+1; i++)
		{
			moveto(C1+i,top);
			lineto(C1+i,bottom);
		}
					
	}
      
   
	
	
}// drawVerticalMarker	

//---------------------------------------------------------------------------
void setupGrid(int xInterval,int yInterval)
{
   verLines(xInterval);
   horLines(yInterval);

}//setupGrid

//---------------------------------------------------------------------------
void verLines(int xInterval)
{
   setcolor(15);
   for(int i=left1;i<=right1;i+=xInterval)
   {
      moveto(i,top1);
      lineto(i,bottom1);
   }//i
   

}//verLines

//---------------------------------------------------------------------------
void horLines(int yInterval)
{
   //Produce horizontal lines at yInterval on each side of origin:
   setcolor(15);
   for(int i=C1;i>=top1;i-=yInterval)
   {
      moveto(left1,i);
      lineto(right1,i);
   }//i

   for(int i=C1;i<=bottom1;i+=yInterval)
   {
      moveto(left1,i);
      lineto(right1,i);
   }//i


}//horLines


void text(int image,float x,float y,char *caption)
{
   
   
   int ypixel = positionPixel(image,ymin1,ymax1,y,0);
   int xpixel = positionPixel(image,xmin,xmax,x,1);
   setfontcolor(15);
   outtextxy(xpixel,ypixel,caption);
   
   
}//text

//---------------------------------------------------------------------------
void setLinePlotFlag(int flg)
{
  //Only use with single trace
  lineFlag = flg;
}

//---------------------------------------------------------------------------
void changePlotMode(int p) 
{
   plotMode = p;
   setLinePlotFlag(0); //Ensure we star anew for line plots
}

//---------------------------------------------------------------------------
void changeLogPlotMode(int p) 
{
   logPlotMode = p;
}

//---------------------------------------------------------------------------
void xPlotIncr(int xincr)
{
   xincrement = xincr;
}

//---------------------------------------------------------------------------
void circleSize(int cSize)
{
   cirSize = cSize;
}



// This function to be used in animated plots (e.g. oscilloscope)
// where information can be updated in real time.
// Each box contains a single item with caption and data.
// This function provides the legend captions.
// Provides up to four legend boxes side by side.
// To be used with the legend(.) function which provides the data.  
void legendCaptions(struct leg_captions l_caps,int col, int numBoxes, int leg_left, int leg_top)
{
   
   int left = leg_left;
   int top = leg_top;
   
   
   for(int i = 0; i < numBoxes; i++)
   {
		
		setcolor(LIGHTGRAY);
		rectangle(left-5,top-5,left+139,top+15);	
		
		
		int arr[] = {left-5+1,top-5+1, left+139+1,top-5+1, 
					    left+139+1,top+15-1,left-5+1,top+15-1,left-5+1,top-5+1};
   	setcolor(BLACK);
   	fillpoly(5, arr);	
   	
		
		//Write captions:
		setfontcolor(col);
		if ( i == 0) 
		{
			outtextxy(left,top,l_caps.data1_caption);	
		}  
		else if (i == 1)
		{
			outtextxy(left,top,l_caps.data2_caption);		
		}
		else if ( i == 2)
		{
			outtextxy(left,top,l_caps.data3_caption);	
		}	
		else if (i == 3)
		{
			outtextxy(left,top,l_caps.data4_caption);	
		}	
		
		
		left += 144;  
   }
	

}//legendCaptions



// Legend function to be used for non-animated plots
void legendCaptions1(int col,int bck_col,int numPlots,struct legend_info info[], int left, int top,int width,int height)
{
   //Set up captions with associated plot symbols:
   //Last legend to a theoretical (line) -if required
   //MAX of four  caption lines!!
   
   //Write legend border:
   int color = bck_col; //7;
      
   
   setcolor(color);
   rectangle(left-5,top-5,left+width,top+height);
   
   //color= LIGHTGRAY;
   //setcolor(color);
   floodfill(left+10,top+10,color);  
    
      
   //Set font color for caption text:
   setfontcolor(col);   
   
   int ypos = 5;
   for(int i=0;i<numPlots;i++)
   {
		//First Legend:
		//outtextxy(left,top+ypos,l_caps.vmax_caption);
		outtextxy(left,top+ypos,info[i].text);
		//setcolor(1);
		setcolor(info[i].color);
		if(info[i].type == CIRCLE_BORDER)
		{
			circleSize(5);
			circle(left+(width-20),top+ypos+5,cirSize);
		}
		if(info[i].type == CIRCLE_FILLED)
		{
			circleSize(5);
			circle(left+(width-20),top+ypos+5,cirSize);
			floodfill(left+(width-20)+1,top+ypos+5+1,info[i].color);
		}
		
		if(info[i].type == SQUARE_BORDER)
		{
			circleSize(8);
			rectangle(left+(width-20),top+ypos,left+(width-20)+cirSize,top+ypos+cirSize);
		}
		if(info[i].type == SQUARE_FILLED)
		{
			circleSize(8);
			rectangle(left+(width-20),top+ypos,left+(width-20)+cirSize,top+ypos+cirSize);
			floodfill(left+(width-20)+1,top+ypos+1,info[i].color);
		}   
		if(info[i].type == TRIANGLE_BORDER)
		{
			circleSize(5);
			triangle(left+(width-20),top+ypos+5,cirSize,TRIANGLE_BORDER);
		}
		if(info[i].type == TRIANGLE_FILLED)
		{
			circleSize(5);
			triangle(left+(width-20),top+ypos+5,cirSize,TRIANGLE_BORDER);
			floodfill(left+(width-20)+1,top+ypos+5+1,info[i].color);
		}
		
		
		if(info[i].type == LINE)
		{
			int font_color = getfontcolor();
			setfontcolor(info[i].color);
			outtextxy(left+(width-20)-10,top+ypos,"----");
			//Revert back to previous font color;
			setfontcolor(font_color);
		}
		 ypos += 20;	
			
   }//i	
   
      
    //Put border around the legend with another color:
   setcolor(BLACK);
   rectangle(left-5,top-5,left+width,top+height);
   
      
      

}//legendCaptions1


//---------------------------------------------------------------------------
// To be used with animated plots (in association with legendCaptions(.))
void legend(struct leg_data l_data,bool refresh, int numBoxes, int color, int leg_left, int leg_top)
{
   
   //NB: If working for non animating graph, refresh is set to zero
   
   char cap1[30], cap2[30], cap3[30], cap4[30];
	         
	int left = leg_left + 4;
   int top = leg_top + 2;
    
    
   if(refresh)  //Repaint to background previous value  
   {
       setfontcolor(m_cMonoBackCol);
       
   }  
   else
   {       
          setfontcolor(color); //10-light green; 5-Magenta
   }      
      
   int shift = 0;
   int data_width = 132;
   int top_high = top + 8;
   int top_low = top - 4;
   int left_start = left+45;
   
   setcolor (BLACK);
   float data;
   for(int i=0; i < numBoxes;i++)
   {
   
		if (i == 0)
		{
			
   	   int arr[] = {shift+left_start,top_low, shift+left+data_width,top_low, 
					       shift+left+data_width,top_high,  shift+left_start,top_high,shift+left_start,top_low};
   		fillpoly(5, arr);	
   	   
   	   if (isnan(l_data.data1))
   	   {
				sprintf(cap1,"%s  %s","XXXX",l_data.data1_unit);
			} 
   	   else
   	   {  	   
   	   
				if ( (l_data.data1 <= 0.0))
				{ 
					data = l_data.data1;
					sprintf(cap1,"%5.2f  %s", data,l_data.data1_unit);
				}
				else if ( (l_data.data1 < 1.0) && (l_data.data1 > 1.0e-3))
				{ 
					
					data = l_data.data1 * 1000;
					sprintf(cap1,"%5.2f  %c%s", data,'m',l_data.data1_unit);
				}
				else if ( (l_data.data1 >= 1.0) && (l_data.data1 < 1.0e3))
				{ 
					data = l_data.data1;
					sprintf(cap1,"%5.2f  %s",data,l_data.data1_unit);
				}
				else if ( (l_data.data1 >= 1.0e3) && (l_data.data1 < 1.0e6))
				{ 
					data = l_data.data1 / 1000;
					sprintf(cap1,"%5.2f  %c%s",data,'k',l_data.data1_unit);
				}
				else if ( l_data.data1 >= 1.0e6)
				{ 
					data = l_data.data1 / 1000000;
					sprintf(cap1,"%5.2f  %c%s",data,'M',l_data.data1_unit);
				}
			}
						  	      	   	
			outtextxy(left+shift+50,top-2,cap1);  
		}
		else if (i == 1)
		{
			
			int arr[] = {shift+left_start,top_low, shift+left+data_width,top_low, 
					       shift+left+data_width,top_high,  shift+left_start,top_high,shift+left_start,top_low};
   		fillpoly(5, arr);	
   	   
   	   if (isnan(l_data.data2))
   	   {
				sprintf(cap2,"%s  %s","XXXX",l_data.data2_unit);
			}   	
			else
			{     	      	      	   	   
				if ( (l_data.data2 <= 0.0))
				{ 
					data = l_data.data2;
					sprintf(cap2,"%5.2f  %s",data,l_data.data2_unit);
				}
				else if ( (l_data.data2 < 1.0) && (l_data.data2 > 1.0e-3))
				{ 
					
					data = l_data.data2 * 1000;
					sprintf(cap2,"%5.2f  %c%s", data,'m',l_data.data2_unit);
				}
				else if ( (l_data.data2 >= 1.0) && (l_data.data2 < 1.0e3))
				{ 
					data = l_data.data2;
					sprintf(cap2,"%5.2f  %s",data,l_data.data2_unit);
				}
				else if ( (l_data.data2 >= 1.0e3) && (l_data.data2 < 1.0e6))
				{ 
					data = l_data.data2 / 1000;
					sprintf(cap2,"%5.2f  %c%s",data,'k',l_data.data2_unit);
				}
				else if ( l_data.data2 >= 1.0e6)
				{ 
					data = l_data.data2 / 1000000;
					sprintf(cap2,"%5.2f  %c%s",data,'M',l_data.data2_unit);
				}			
			}
			//Output data2
			outtextxy(left+shift+50,top-2,cap2);
			
			
		}
		else if (i == 2)
		{
						
			int arr[] = {shift+left_start,top_low, shift+left+data_width,top_low, 
					       shift+left+data_width,top_high,  shift+left_start,top_high,shift+left_start,top_low};
   		fillpoly(5, arr);	
   	   
			if (isnan(l_data.data3))
   	   {
				sprintf(cap3,"%s  %s","XXXX",l_data.data3_unit);
			}
			else
			{			
						
				if ( (l_data.data3 <= 0.0))
				{ 
					data = l_data.data3;
					sprintf(cap3,"%5.2f  %s", data,l_data.data3_unit);
				}
				else if ( (l_data.data3 < 1.0) && (l_data.data3 > 1.0e-3))
				{ 
					
					data = l_data.data3 * 1000;
					sprintf(cap3,"%5.2f  %c%s", data,'m',l_data.data3_unit);
				}
				else if ( (l_data.data3 >= 1.0) && (l_data.data3 < 1.0e3))
				{ 
					data = l_data.data3;
					sprintf(cap3,"%5.2f  %s",data,l_data.data3_unit);
				}
				else if ( (l_data.data3 >= 1.0e3) && (l_data.data3 < 1.0e6))
				{ 
					data = l_data.data3 / 1000;
					sprintf(cap3,"%5.2f  %c%s",data,'k',l_data.data3_unit);
				}
				else if ( l_data.data3 >= 1.0e6)
				{ 
					data = l_data.data3 / 1000000;
					sprintf(cap3,"%5.2f  %c%s",data,'M',l_data.data3_unit);
				}
		   }
		   								
			//Ouput data3:
			outtextxy(left+shift+50,top-2,cap3);
			
		}
		else if (i == 3)
		{
			
			
   	   int arr[] = {shift+left_start,top_low, shift+left+data_width,top_low, 
					       shift+left+data_width,top_high,  shift+left_start,top_high,shift+left_start,top_low};
   		fillpoly(5, arr);	
   	   
   	   
   	   if (isnan(l_data.data4))
   	   {
				sprintf(cap4,"%s  %s","XXXX",l_data.data4_unit);
			}
			else
			{	
			
				if ( (l_data.data4 <= 0.0))
				{ 
					data = l_data.data4;
					sprintf(cap4,"%5.2f  %s",data,l_data.data4_unit);
				}
				else if ( (l_data.data4 < 1.0) && (l_data.data4 > 1.0e-3))
				{ 
					data = l_data.data4 * 1000;
					sprintf(cap4,"%5.2f  %c%s", data,'m',l_data.data4_unit);
				}
				else if ( (l_data.data4 >= 1.0) && (l_data.data4 < 1.0e3))
				{ 
					data = l_data.data4;
					sprintf(cap4,"%5.2f  %s",data,l_data.data4_unit);
				}
				else if ( (l_data.data4 >= 1.0e3) && (l_data.data4 < 1.0e6))
				{ 
					data = l_data.data4 / 1000;
					sprintf(cap4,"%5.2f  %c%s",data,'k',l_data.data4_unit);
				}
				else if ( l_data.data4 >= 1.0e6)
				{ 
					data = l_data.data4 / 1000000;
					sprintf(cap4,"%5.2f  %c%s",data,'M',l_data.data4_unit);
				}
			}			
   	   
			//Output data4:
			outtextxy(left+shift+50,top-2,cap4);
				
		}	
		
		
		shift += 144;     
   }
   
   
   

}//legend



//---------------------------------------------------------------------------
void axisDefaults(int xdivs,int ydivs, char *xspec,
                              char *yspec)
{
   xnoDivs = xdivs;   //Redefine no. scale divs. along x-axis
   ynoDivs = ydivs;   //Redefine no. scale divs. along y-axis
   strcpy(xfield,xspec);  //Redefine field specification: x scaling
   strcpy(yfield,yspec);  //Redefine field specification: y scaling

}//axisDefaults

//---------------------------------------------------------------------------
void setxMinLinear(int min)
{
   xminLinear=min;
}

//---------------------------------------------------------------------------
int getxMinLinear()
{
   return xminLinear;
}

//---------------------------------------------------------------------------
void setxMaxLinear(int max)
{
   xmaxLinear=max;
}

//---------------------------------------------------------------------------
int getxMaxLinear()
{
   return xmaxLinear;
}

//---------------------------------------------------------------------------
void setxMin(int min)
{
   xmin=min;
}

//---------------------------------------------------------------------------
void setxMax(int max)
{
   xmax=max;
}

//---------------------------------------------------------------------------
void setyMin(int min)
{
   ymin1=min;
}

//---------------------------------------------------------------------------
void setyMax(int max)
{
   ymax1=max;
}

//---------------------------------------------------------------------------
void setGridFlg(bool flg)
{
   m_bGrid=flg;
}

//---------------------------------------------------------------------------
bool getGridFlg()
{
   return m_bGrid;
}

//---------------------------------------------------------------------------
void setxgrad(bool flg)
{
   xgrad=flg;
}

//---------------------------------------------------------------------------
bool getxgrad()
{
   return xgrad;
}

//---------------------------------------------------------------------------
void initialisePizPos()
{
   counter1=left1;
   counter2=left2;
}

//---------------------------------------------------------------------------
float getXValFromPixPos(int npixels)
{

   int X;
   int XVal = 0;;

   if(image == 1)
   {
      X = right1 - left1;


      if(xmin==0)
      {
         XVal = ((npixels-xmargin)*xmax )/X;
         return XVal;
      }
      else if(xmax==0)
      {
        XVal = xmin - ((npixels-xmargin)*xmin )/X;
        return XVal;
      }
      else
      {
         float D = fabs(xmin) + fabs(xmax);
         float R = fabs(xmin)/D ;
         XVal = ((npixels-xmargin)*D)/X;
         if(XVal <= R*D)
         {
            XVal = xmin + XVal;
         }
         else
         {
            XVal = XVal - R*D;
         }
         return XVal;
      }
      

   }//if(image == 1)

   return XVal;
}//getXValueFromPixPos
#endif
#ifdef GTK
//---------------------------------------------------------------------------
      
  cairo_surface_t *surface = NULL;
  cairo_t *context = NULL;
  GdkRGBA black, white, dark_gray, ori_line_color;
  int surface_width = 0;
  int surface_height = 0;
  
  
         
   //Width/Height of GUI form
    int X_FORM_SIZE, Y_FORM_SIZE;  
			 
	bool showGrid, xgrad, ygrad;
	int plotMode, gridInt;
	int xincrement, cirSize, logPlotMode, lineFlag,
		xmargin, ymargin;
	float ymin1, ymin2, ymax1, ymax2, xmin, xmax, sigStore1,
		  sigStore2;
	float xminLinear,xmaxLinear;
	float ymaxVal1,ymaxVal2;
	int left, top, right, bottom,C1, counter1, counter2;
	int left1,top1,right1,bottom1,left2,top2,right2,bottom2,C2;
	int xnoDivs, ynoDivs;
	char xfield[4], yfield[4];
	char xLabel[100],yLabel[100], Title[100], WinTitle[100];
	int image;
	int buffCount1, buffCount2;
	//TRect MonoGraphRectangle;
	bool showXAxisNums,showYAxisNums,m_bGrid;
	bool Bitmap;
	char m_sPrinterName[100];
	int m_iFormHeight, m_iFormWidth;
	GdkRGBA m_cMonoBackCol;
			
	bool show_ori_line;   
	bool showXGradLines, showYGradLines;  
	
	// Width of a line plot in pixels
	// Should be defined as an odd number (for plot symmetry)
	// Even numbers shall be reduced by 1 in plot routine
	int line_width;  
        
  
	// Structures will go here
    struct leg_captions l_caps;
    struct leg_data l_data;
  
  
    //Audio player members:
	char m_szAudioIniFile[200],m_sFile[200];
	int m_iNumChannels, m_iSampleRate, m_iBitsPerSample;
	int m_iCurrSampleNum;
	//TColor m_cWaveSigColor, m_cDataPlotColor, m_cAudioMarkerColor,
	//       m_cAudioClipMarkerColor;
	int m_iSigUpper, m_iSigLower;
	bool m_bSigTrace,m_bSigRangeDefined;

	
	//frequency spectrum:
	int m_iFreqIndex;
	bool m_bHarmonic;  //Flag for plotting frequency harmonics

	//Colours for graphics:
	//TColor m_cBackground,m_cGridlines,m_cSignal,m_cSpectrum, m_cHarmonics,
	//m_cSFmarker, m_cOrigin;
	bool m_bAnimated,m_bResize;
	char m_sFileExt[4];
	bool m_bIsWaveFile;

	//Data for bitmap image:
	//intmat m_iImageData;
	int m_iBitsPerPixel;

  
  
  
  void Graphic_Init(int argc, char * argv[], int xFormSize,int yFormSize, int pMode,int logMode,
               char *xlab, char *ylab, char *win_title, char *gphtitle, int gph_refresh_rate,
               float yMin, float yMax, float xMin, float xMax,
               int xInt, int yInt, bool xGrad, bool yGrad, int XAxisPrec, int YAxisPrec,
               bool grid, int gInt,
               bool xshow,bool yshow,bool ori, GdkRGBA ori_color,bool bitmap,
               GdkRGBA monobcolor,char *audfile, char *audInifile,
               int chans,int samrate, int bps)
 
 {
	 
	
	//Initialise colors:
	white.red = 1.0;
	white.green = 1.0;
	white.blue = 1.0;
	white.alpha = 0.0;
  	 
	black.red = 0.0;
	black.green = 0.0;
	black.blue = 0.0;
	white.alpha = 0.0;
	
	dark_gray.red = 0.663;
	dark_gray.green = 0.663;
	dark_gray.blue = 0.663;
	dark_gray.alpha = 0.0;  
   
   
	X_FORM_SIZE = xFormSize;
	Y_FORM_SIZE = yFormSize;
   
     
    m_bAnimated = false;
    m_bResize = false;
  
	//Set scales ranges 
	ymin1 = yMin;
	ymax1 = yMax;
	xmin = xMin;
	xmax = xMax;
	
	
   //Save linear definitions (in case transformed to log values):
   xminLinear = xmin;
   xmaxLinear = xmax;

   //Get flag for plotting harmonics on frequency spectrum
   m_bHarmonic = false;

   //Set audio player members:
   strcpy(m_szAudioIniFile,audInifile);
   strcpy(m_sFile,audfile);
   m_iNumChannels = chans;
   m_iSampleRate = samrate;
   m_iBitsPerSample = bps;

   
   //Check for bitmap option:
   Bitmap = bitmap;

   //Define plot mode:
   plotMode = pMode;  //0:line, 1:pixel, 2:circle, 3:rectangle, 4:triangle
   logPlotMode = logMode;  //0: default - no log plotting
                          //1: log plot: x axis
                          //2: log plot: y axis
                          //3: log plot: x and y axis

              
   image = 1;


   xincrement = 1; //Default value for pixel plot advances
   cirSize = 5;    //Initialise circle size for circle plots
   
   
   setLinePlotFlag(0);

   
   //Set  axis numbering defaults:
   showXAxisNums = xshow;
   showYAxisNums = yshow;
   

   //Set window form title
   strcpy(WinTitle,win_title);
   //Set graph title:
   strcpy(Title,gphtitle);
   
   //Set axis labels:
   strcpy(xLabel,xlab);
   strcpy(yLabel,ylab);

   //Set flags for graduations:
   xgrad = xGrad;
   ygrad = yGrad;
   xnoDivs = xInt;
   ynoDivs = yInt;

   showGrid = grid;
   //showXGradLines = xshow;
   //showYGradLines = yshow;
	showXGradLines = xGrad;
   showYGradLines = yGrad;

   if(logPlotMode == 1)
   {
      xmin = log10(xmin);
      if(xmax!= 0)
         xmax = log10(xmax);
         
   }
   if(logPlotMode == 2)
   {
      ymin1 = log10(ymin1);
      if(ymax1!= 0)
         ymax1 = log10(ymax1);
   }
   if(logPlotMode == 3)
   {
      ymin1 = log10(ymin1);
      if(ymax1!= 0)
         ymax1 = log10(ymax1);
      xmin = log10(xmin);
      if(xmax!= 0)
         xmax = log10(xmax);
   }
   
   
  counter1 = 0.0;
  
  // Define decimal precision for number labelling on x and axes:
  char prec = XAxisPrec + '0';
  xfield[0] = '5';
  xfield[1] = '.';
  xfield[2] = prec;
  
  prec = YAxisPrec + '0';
  yfield[0] = '5';
  yfield[1] = '.';
  yfield[2] = prec;

  // Set default for the line width plot (in pixels)
  line_width = 1;    
  lineFlag = 0; //Initiliase for start of line plots
  
   
   //-----------------------------------------------------------------------------------------------
   // Initialise GTK elements:   
    //----------------------------------------------------------------------------------------------
  	
	
		
	
  
  //Set up display graphic area:  -this is the outer rectangle for entire graph (plot + labels)
  setupGraphSize();
	
  //Initialise background:
  m_cMonoBackCol = monobcolor;
  //drawGraphBackground();
  
         
     	
  
}

void setGtkContext (cairo_t *cr)
{
	context = cr;	
	
}


//--------------------------------------------------------------------------
void drawGraphBackground ()
{
	
	setMonoGraphBackground (m_cMonoBackCol);
	    	
	// Draw the X-Y labels and Title
	setupWindow(image,xmargin,ymargin,X_FORM_SIZE-xmargin,
               Y_FORM_SIZE-ymargin,xLabel,yLabel);

        
	
	
	
	
	// Draw the X-Y scaling
	//Draw axis scales:
	if(!Bitmap) axisScales();
    
	//Draw origin line:
	if (show_ori_line)
	{
		drawOriginLine();
	}
	
	
	
	// Draw the gridlines
	//Draw graduations:
	if(showGrid)
	{
      if (gdk_rgba_equal ( &m_cMonoBackCol, &black))
      {
         graduations(dark_gray);
      }
      if (gdk_rgba_equal ( &m_cMonoBackCol, &white))
      {
         graduations(black);
      }  
      } 
  	
		
			
}	

//---------------------------------------------------------------------------
void setupGraphSize()
{
     
      //Set up outer rectangle window:
      
      xmargin = 30;
      ymargin = 35;

      left = xmargin;
      left1 = left;
      top = ymargin;
      top1 = top;
      right = X_FORM_SIZE-xmargin;
      right1 = right;
      bottom = Y_FORM_SIZE-ymargin;
      bottom1 = bottom;
      


}//setupGraphSize

//---------------------------------------------------------------------------
void setMonoGraphBackground(GdkRGBA color)
{
	
   //cairo_t *context = cairo_create (surface);
   //context = cairo_create (surface);
	
	// Draw main window area
	cairo_set_source_rgb (context, 1, 1, 1);
	cairo_rectangle (context, 0, 0, X_FORM_SIZE, Y_FORM_SIZE);
    cairo_fill (context);

	// Draw the graphing area
	cairo_set_source_rgb (context, color.red, color.green, color.blue);
	cairo_rectangle (context, xmargin, ymargin, X_FORM_SIZE - 2*xmargin, Y_FORM_SIZE - 2*ymargin);
	cairo_fill (context);
	 

}//setMonoGraphBackGround

//---------------------------------------------------------------------------
void setupWindow(int image,int left,int top,int right,
                             int bottom, char *xlabel, char *ylabel)
{
   
   //Print title and axis labels:
   title(Title);
   xAxisText(image,xlabel);
   yAxisText(image,ylabel);
	

}//setupWindow




//---------------------------------------------------------------------------
void title(char *text)
{
   
   
   cairo_text_extents_t extents;   //To help with text centering
   
   //pango_layout_get_pixel_size();
   
   //cairo_t *cr = cairo_create (surface);
   //context = cairo_create (surface);
   
   cairo_set_source_rgb(context, 0.0, 0.0, 0.0);
   cairo_select_font_face(context, "Arial",	CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
   cairo_set_font_size(context, 15);
     
   cairo_text_extents (context, text, &extents);
   
  //cairo_move_to(cr, (X_FORM_SIZE / 2) - 2*xmargin, ymargin-10);
  cairo_move_to(context, (X_FORM_SIZE / 2) - (extents.width / 2), ymargin - 10);
  cairo_show_text(context, text);
  
	
}//title

//---------------------------------------------------------------------------
void xAxisText(int image,char *text)
{
	
   cairo_text_extents_t extents;   //To help with text centering
   
   //cairo_t *cr = cairo_create (surface);
   //context = cairo_create (surface);
   cairo_set_source_rgb(context, 0.0, 0.0, 0.0);
   cairo_select_font_face(context, "Arial",	CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
   cairo_set_font_size(context, 14);
     
   cairo_text_extents (context, text, &extents);
   
   //cairo_move_to(cr, (X_FORM_SIZE / 2) - 2*xmargin, bottom1+25);
   cairo_move_to(context, (X_FORM_SIZE / 2) - (extents.width / 2), bottom1+25);
   cairo_show_text(context, text);
	


}//xAxisText

//---------------------------------------------------------------------------
void yAxisText(int image,char *text)
{
            
    cairo_text_extents_t extents;   //To help with text centering 
      
    //cairo_t *cr = cairo_create (surface);
    //context = cairo_create (surface);
	
	cairo_set_source_rgb(context, 0.0, 0.0, 0.0);
	cairo_select_font_face(context, "Arial",	CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size(context, 12);
    
    cairo_text_extents (context, text, &extents); 
	int len = bottom1-top1;
    int txtHeight = extents.height;
    int verlen = txtHeight*strlen(text);
    int centralPos = (len-verlen)/2;
    int pos = centralPos + ymargin;
      
   
    //Print y axis label
     char str[2];
     str[1] = '\0';
     for(int i=0;i<strlen(text);i++)
     {
         str[0] = text[i];
         cairo_move_to(context, 5, pos);
		 cairo_show_text(context, str);
	     pos = pos+15;
	     
      }//i
   
	 
      
      
      
}//yAxisText



//---------------------------------------------------------------------------
void tagOrigin(int image,int left,int posOrig)
{

      
	//cairo_t *cr = cairo_create (surface);
	//context = cairo_create (surface);
	cairo_set_source_rgb(context , ori_line_color.red, ori_line_color.green, ori_line_color.blue);
       
	cairo_move_to(context , left-5, C1 );
	cairo_line_to(context , left, C1 );  
	cairo_stroke(context );   

}//tagOrigin

//---------------------------------------------------------------------------
void graduations(GdkRGBA color)
{
   
   int Image = 1;   //Set for mono graph plot:
   
      
   if(showXGradLines)
   {
      //Obtain vertical graduations:
      if(logPlotMode == 1 || logPlotMode == 3)
      {
         //log graduations:
         for(int i=xmin;i<=xmax-1; i+=1)
         {
			 int x = positionPixel(Image,xmin,xmax,i,1);
             pixelVerLine(color,x,4);
             for(int n=2;n<10;n++)
             {
               float pos = log10( n*pow(10,i) );
               x = positionPixel(Image,xmin,xmax,pos,1);
               pixelVerLine(color,x,4);
             }//n
         }//i

      }
      else
      {  //linear graduations
         if(xgrad)
         {
            float interval = (xmax-xmin)/xnoDivs;
            float pos = xmin;
            for(int i=0;i<=xnoDivs;i++)
            {
               int x = positionPixel(Image,xmin,xmax,pos,1);
               pixelVerLine(color,x,4);
               pos += interval;
            }//i
         }//if

      }
   }//if(showXGradLines)
   
   if(showYGradLines)
   {
       //Obtain horizontal graduations:
      if(logPlotMode == 2 || logPlotMode == 3)
      {
         for(int i=ymin1;i<=ymax1-1;i+=1)
         {
            int y = positionPixel(Image,ymin1,ymax1,i,0);
            pixelHorLine(color,y,3);
            for(int n=2;n<10;n++)
            {
               //float pos = log10( n*pow10(i) );
               float pos = log10( n*pow(10,i) );
               y = positionPixel(Image,ymin1,ymax1,pos,0);
               pixelHorLine(color,y,3);
            }//n
         }//i
      }
      else
      {
         if(ygrad)
         {
            float interval = (ymax1-ymin1)/ynoDivs;
            float pos = ymin1;
            for(int i=0;i<=ynoDivs;i++)
            {
               int y = positionPixel(Image,ymin1,ymax1,pos,0);
               pixelHorLine(color,y,3);
               pos += interval;
            }//i
         }//if
      }
   }//if(showYGradLines
	
	
}//graduations



//---------------------------------------------------------------------------
void plotLine(float ySig, float xSig,int color,bool start, bool end)
{
//NB: first indicates first data of the series: need to perform cairo_moveto
	//    last indicates last data of series: the ncan perform cairo_stroke
	//    (this prevents doing the operation for each data point)   
   
	     
	if(logPlotMode == 1) xSig = log10(xSig);
	if(logPlotMode == 2) ySig = log10(ySig);
	if(logPlotMode == 3)
	{
      xSig = log10(xSig);
      ySig = log10(ySig);
	}
	int ypixel = positionPixel(1,ymin1,ymax1,ySig,0);
	int xpixel = positionPixel(1,xmin,xmax,xSig,1);
   
   //setcolor(color);
   cairo_set_source_rgb(context, getGdkColor(color).red, getGdkColor(color).green, getGdkColor(color).blue);
      
   if(plotMode == PIXEL)
   {
      cairo_move_to (context, xpixel,ypixel);
	  cairo_line_to (context, xpixel,ypixel+1);
	  cairo_stroke (context);
   }
   if(plotMode == LINE)
   {
      if(lineFlag>0)
      {
         // Check we have an odd number (for symmetry)
         if ( (line_width % 2) == 0 )
         {
				// Reduce width by 1 pixel
				line_width -= 1;	
			}	
         int span = (line_width - 1) / 2.0;
         
         for(int i = -span; i < span+1; i++)
			{
				//moveto(sigStore1+i,sigStore2+i);
				//lineto(xpixel+i,ypixel+i);
				if (start == true)
				{
					cairo_move_to (context, sigStore1+i,sigStore2+i);
				}	
				cairo_line_to (context, xpixel+i,ypixel+i);
				if (end == true)
				{
					cairo_stroke (context);
				}	
			}
				
      }
      sigStore1 = xpixel;
      sigStore2 = ypixel;
      lineFlag = 1;
   }
   if(ySig>ymaxVal1) ymaxVal1 = ySig;
   


} // plotLine

//---------------------------------------------------------------------------
void plotSig(float ySig, float xSig,int color)
{
	
	//NB: first indicates first data of the series: need to perform cairo_moveto
	//    last indicates last data of series: the ncan perform cairo_stroke
	//    (this prevents doing the operation for each data point)   
   
	//cairo_t *cr = cairo_create (surface);
	//context = cairo_create (surface);
	     
     
	if(logPlotMode == 1) xSig = log10(xSig);
	if(logPlotMode == 2) ySig = log10(ySig);
	if(logPlotMode == 3)
	{
      xSig = log10(xSig);
      ySig = log10(ySig);
	}
	int ypixel = positionPixel(1,ymin1,ymax1,ySig,0);
	int xpixel = positionPixel(1,xmin,xmax,xSig,1);
   
   //setcolor(color);
   cairo_set_source_rgb(context, getGdkColor(color).red, getGdkColor(color).green, getGdkColor(color).blue);
   if(plotMode == TRIANGLE_BORDER)
   {
		//triangle(xpixel,ypixel,cirSize,TRIANGLE_BORDER);
		triangle(xpixel, ypixel, cirSize, TRIANGLE_BORDER, color);	
	}	
	if(plotMode == TRIANGLE_FILLED)
   {
		//triangle(xpixel,ypixel,cirSize,TRIANGLE_FILLED);	
		triangle(xpixel, ypixel, cirSize, TRIANGLE_FILLED, color);
	}	
   if(plotMode == CIRCLE_BORDER)
   {
		//circle(xpixel,ypixel,cirSize);
		cairo_arc (context,xpixel,ypixel,cirSize,0,2*3.142);
		cairo_stroke (context);
		
	}	
   if(plotMode == CIRCLE_FILLED)
   {
      //circle(xpixel,ypixel,cirSize);
      //floodfill(xpixel+1,ypixel+1,color);
      cairo_arc (context,xpixel,ypixel,cirSize,0,2*3.142);
	  cairo_fill (context);
		
                            
   }
   if(plotMode == SQUARE_BORDER)
   {
      //rectangle(xpixel-cirSize,ypixel-cirSize,xpixel+cirSize,ypixel+cirSize);
      cairo_rectangle (context,xpixel-cirSize,ypixel-cirSize, xpixel+cirSize,ypixel+cirSize);		
	  cairo_stroke (context); 

                              
   }
   if(plotMode == SQUARE_FILLED)
   {
      //rectangle(xpixel-cirSize,ypixel-cirSize,xpixel+cirSize,ypixel+cirSize);
      //floodfill(xpixel-cirSize+1,ypixel-cirSize+1,color);
      cairo_rectangle (context,xpixel-cirSize,ypixel-cirSize, xpixel+cirSize,ypixel+cirSize);		
	  cairo_fill (context); 
                        
   }
    
   
   if(plotMode == PIXEL)
   {
      cairo_move_to (context, xpixel,ypixel);
	  cairo_line_to (context, xpixel,ypixel+1);
	  cairo_stroke (context);
   }
   if(plotMode == LINE)
   {
      if(lineFlag>0)
      {
         // Check we have an odd number (for symmetry)
         if ( (line_width % 2) == 0 )
         {
				// Reduce width by 1 pixel
				line_width -= 1;	
			}	
         int span = (line_width - 1) / 2.0;
         
         for(int i = -span; i < span+1; i++)
			{
				//moveto(sigStore1+i,sigStore2+i);
				//lineto(xpixel+i,ypixel+i);
				cairo_move_to (context, sigStore1+i,sigStore2+i);
				cairo_line_to (context, xpixel+i,ypixel+i);
				cairo_stroke (context);
					
			}
				
      }
      sigStore1 = xpixel;
      sigStore2 = ypixel;
      lineFlag = 1;
   }
   if(ySig>ymaxVal1) ymaxVal1 = ySig;
   
    
   
}//plotSig  (for single screen plotting)




//---------------------------------------------------------------------------
void lineWidth(int width)
{
	line_width = width;
}

//---------------------------------------------------------------------------
void triangle(int x, int y, int size, int type, int color)
{
				   
	//cairo_t *cr = cairo_create (surface);
	//context = cairo_create (surface);
	cairo_set_source_rgb(context, getGdkColor(color).red, getGdkColor(color).green, getGdkColor(color).blue);
	
	// Get vertices of the triangle
	int a = (int) (2.0*size)/1.732;
	
	//cairo_move_to(cr, (X_FORM_SIZE / 2) - 2*xmargin, bottom1+25);
	cairo_move_to(context, x-a,y+size);
	cairo_line_to(context, x+a,y+size);
	
	cairo_move_to(context, x+a,y+size);
	cairo_line_to(context, x,y);
	
	
	cairo_line_to(context,x-a,y+size);
	
	if(type == TRIANGLE_BORDER)
	{
		cairo_stroke (context);
	}
	if(type == TRIANGLE_FILLED)
	{
	  cairo_fill (context);	
	}
			
}	

//---------------------------------------------------------------------------
int positionPixel(int image,float ymin,float ymax,float sig,
                              int pixelMode)
{
   int X,y;
   float R = ymax/(ymax-ymin);
   if(ymax==0) R = ymin/(ymin-ymax);
   if(pixelMode == 0)
   {                    //positioning along y axis
      if(image == 1) X = bottom1 - top1;
      else if(image == 2)
        X = bottom2 - top2;
      if(ymax==0) y = (fabs(sig)/(-ymin))*R*X;
      else
         y = (fabs(sig)/ymax)*R*X;
      if(sig>=0.0)
      {
         if(ymax != 0) y = (R*X)-y;
      }
      else
      {
         if(ymax != 0)
            y = y+(R*X);
      }
      y += ymargin;
   }//if
   if(pixelMode == 1)
   {                //positioning along x axis
      R = ymax/(ymax-ymin);
      //if(ymax==0) R = ymin/(ymin-ymax);
      R = 1-R;
      if(image == 1) X = right1 - left1;
      else if(image == 2)
         X = right2 - left2;
      float orig = R*X;
      if(sig>=0.0)
      {
         if(ymax==0) y = (fabs(sig)/(-ymin))*(X-orig);
         else
            y = orig + (fabs(sig)/ymax)*(X-orig);
      }
      if(sig<0.0)
      {
         y = orig - (fabs(sig)/fabs(ymin))*orig;
      }
      y+= xmargin;
   }

   return y;

}//positionPixel


void plotVerLine(float ySig,float xSig)
{

	
   //This routine plots a vertical line (along +yaxis) of given height at x position specified

   //if(logPlotMode == 1) xSig = log10(xSig);
   if(logPlotMode == 2) ySig = log10(ySig);
   if(logPlotMode == 3)
   {
      xSig = log10(xSig);
      ySig = log10(ySig);
   }

   int yminpixel = positionPixel(1,ymin1,ymax1,0.0,0);
   int ymaxpixel = positionPixel(1,ymin1,ymax1,ySig,0);
   int xpixel = positionPixel(1,xmin,xmax,xSig,1);

   //cairo_t *cr = cairo_create (surface);
   //context = cairo_create (surface);
   cairo_set_source_rgb(context, white.red, white.green, white.blue);
    
   cairo_move_to(context, xpixel,yminpixel);
   cairo_line_to(context, xpixel,ymaxpixel);
   cairo_stroke (context);
   

}//plotVerLine



//---------------------------------------------------------------------------
void pixelVerLine(GdkRGBA color, int x, int pixelIncr)
{
    
   
   //cairo_t *cr = cairo_create (surface);
   //context = cairo_create (surface);
   cairo_set_source_rgb(context, color.red, color.green, color.blue);
      
   for(int j=top;j<=bottom;j+=pixelIncr)
   {
	   cairo_move_to(context, x,j);
	   cairo_line_to(context, x,j+1);
   }
   cairo_stroke (context);
      

}//pixelVerLine


//---------------------------------------------------------------------------
void pixelHorLine(GdkRGBA color, int y, int pixelIncr)
{
      
   //cairo_t *cr = cairo_create (surface);
   //context = cairo_create (surface);
   cairo_set_source_rgb(context, color.red, color.green, color.blue);
      
   
   for(int j=left;j<=right;j+=pixelIncr)
   {
	   cairo_move_to(context, j,y);
	   cairo_line_to(context, j+1,y);
   }
   cairo_stroke (context);
   

}//pixelVerLine


//---------------------------------------------------------------------------
void axisScales()
{ 
   
   xAxisScale(); 
   yAxisScale(); //Only for mono-trace
    
}

//---------------------------------------------------------------------------
void xAxisScale()
{
   
   if(showXAxisNums)
   {
      char numtext[3], powtext[3];
      //cairo_t *cr = cairo_create (surface);
      //context = cairo_create (surface);
	  cairo_set_source_rgb(context, black.red, black.green, black.blue);
	  
	  cairo_select_font_face(context, "Arial",	CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	  cairo_set_font_size(context, 12);
      
      sprintf(numtext,"%d",10);
      //int horPos = (bottom-top)+14;
	  int horPos = (bottom-top)+22;
	  
      //Obtain x axis scale:
      if(logPlotMode == 1 || logPlotMode == 3)
      {
         for(int i=xmin;i<=xmax;i+=1)
         {
            int x = positionPixel(1,xmin,xmax,i,1);
            sprintf(powtext,"%d",i);
            x = x-30;
            //outtextxy(x+15,horPos+25,numtext);
            cairo_move_to(context, x+15, horPos+25);
		    cairo_show_text(context, numtext);
            
            //outtextxy(x+28,horPos+20,powtext);
            cairo_move_to(context, x+28, horPos+20);
		    cairo_show_text(context, powtext);
         }//i

      }
      else
      {
		 char floatnumtext[6], field[6];
         char *s1 = "%", *s2 = "f";
         strcpy(field,s1);
         strcat(field,xfield);
         strcat(field,s2);

         float interval = (xmax-xmin)/xnoDivs;
         float pos = xmin;
         for(int i=0;i<=xnoDivs;i++)
         {
            int x = positionPixel(1,xmin,xmax,pos,1);
            sprintf(floatnumtext,field,pos);
            x = x-30;
				cairo_move_to(context, x+15, horPos+25);
				cairo_show_text(context, floatnumtext);
            	pos += interval;

         }//i

      }
   }//if(ShowXAxisNums)
    
        

}//xAxisScale


//---------------------------------------------------------------------------
void yAxisScale()
{

   
   if(showYAxisNums)
   {
      char numtext[3], powtext[3];

      sprintf(numtext,"%d",10);
      int verPos = left-80;
      
      //cairo_t *cr = cairo_create (surface);
      //context = cairo_create (surface);
	  cairo_set_source_rgb(context, black.red, black.green, black.blue);
	  
	  cairo_select_font_face(context, "Arial",	CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	  cairo_set_font_size(context, 12);
      
      
      //Obtain y axis scale:
      if(logPlotMode == 2 || logPlotMode == 3)
      {
         for(int i = ymin1;i<=ymax1;i+=1)
         {
            int y = positionPixel(1,ymin1,ymax1,i,0);
            sprintf(powtext,"%d",i);
            cairo_move_to(context, verPos+60, y-5 );
		    cairo_show_text(context, numtext);
                        
            cairo_move_to(context, verPos+75, y-10 );
		    cairo_show_text(context, powtext);
             
            
         }//i
      }
      else
      {
         char floatnumtext[6], field[6];
         char *s1 = "%", *s2 = "f";
         strcpy(field,s1);
         strcat(field,yfield);
         strcat(field,s2);

         float interval = (ymax1-ymin1)/ynoDivs;
         float pos = ymin1;
         for(int i=0;i<=ynoDivs;i++)
         {
            sprintf(floatnumtext,field,pos);
            int y = positionPixel(1,ymin1,ymax1,pos,0);
            //outtextxy(verPos+55,y-6,floatnumtext);
            //cairo_move_to(cr, verPos+55, y-6 );
            cairo_move_to(context, verPos+55, y+5 );
		    cairo_show_text(context, floatnumtext);
            pos += interval;
         }//i

      }
   }//if(showYAxisNums)
    

}//yAxisScale


//---------------------------------------------------------------------------
void drawOriginLine()
{
   
  if(logPlotMode == 0 || logPlotMode == 1)
  {
      //cairo_t *cr = cairo_create (surface);
      //context = cairo_create (surface);
	  cairo_set_source_rgb(context, ori_line_color.red, ori_line_color.green, ori_line_color.blue);
	       
      C1 = ( ymax1/(ymax1-ymin1) )* (bottom-top);
      C1 = C1 + ymargin;  //Add margin offset
      cairo_move_to(context, left, C1 );
	  cairo_line_to(context, right, C1 );  
	  cairo_stroke(context);            
      
      tagOrigin(image,left,C1);
  }
  
  
}//drawOriginLine


//---------------------------------------------------------------------------
void drawVerticalMarker(float pos, GdkRGBA color,int width)
{
	
	
	//cairo_t *cr = cairo_create (surface);
	//context = cairo_create (surface);
	cairo_set_source_rgb(context, color.red, color.green, color.blue);
	  
	
	
	//int width: 0: standard 1 pixel; 1: 3 pixel width
		 
	// Draws a vertical marker line at the requested postion
	if(logPlotMode == 1 || logPlotMode == 3)
   {
      pos = log10(pos);

   }
	      
	if ((pos >= xmin) && (pos <= xmax))
	{ 
		C1 = ( (pos-xmin) / (xmax-xmin) )* (right-left);
		C1 = C1 + xmargin;  //Add margin offset
		
					
		// Check we have an odd number (for symmetry)
		if ( (width % 2) == 0 )
		{
			// Reduce width by 1 pixel
			width -= 1;	
		}	
		int span = (width - 1) / 2.0;
		
		for(int i = -span; i < span+1; i++)
		{
			//moveto(C1+i,top);
			//lineto(C1+i,bottom);
			cairo_move_to(context, C1+i, top );
			cairo_line_to(context, C1+i, bottom );  
	 
		}
		
					
	}
      
   	cairo_stroke(context);
	             
      
	
	
}// drawVerticalMarker	

//---------------------------------------------------------------------------
void setupGrid(int xInterval,int yInterval)
{
   
   verLines(xInterval);
   horLines(yInterval);
    

}//setupGrid

//---------------------------------------------------------------------------
void verLines(int xInterval)
{
   
	//cairo_t *cr = cairo_create (surface);
	//context = cairo_create (surface);
	cairo_set_source_rgb(context, white.red, white.green, white.blue);
	
   for(int i=left1;i<=right1;i+=xInterval)
   {
		cairo_move_to(context, i, top1 );
		cairo_line_to(context,i, bottom1 );
   }//i
    
   cairo_stroke (context);

}//verLines

//---------------------------------------------------------------------------
void horLines(int yInterval)
{
   
	//cairo_t *cr = cairo_create (surface);
	//context = cairo_create (surface);
	cairo_set_source_rgb(context, white.red, white.green, white.blue);
	
   //Produce horizontal lines at yInterval on each side of origin:
   for(int i=C1;i>=top1;i-=yInterval)
   {
		cairo_move_to(context, left1, i );
		cairo_line_to(context,right1, i );
   
   }//i

   for(int i=C1;i<=bottom1;i+=yInterval)
   {
		cairo_move_to(context, left1, i );
		cairo_line_to(context,right1, i );
   }//i
	
	cairo_stroke (context);	
	
	

}//horLines


void text(int image,float x,float y,char *caption)
{
   
   
	int ypixel = positionPixel(image,ymin1,ymax1,y,0);
	int xpixel = positionPixel(image,xmin,xmax,x,1);
       
	//cairo_t *cr = cairo_create (surface);
	//context = cairo_create (surface);
	cairo_set_source_rgb(context, white.red, white.green, white.blue);
	cairo_move_to(context, xpixel, ypixel );
	cairo_show_text(context,caption );
   
   
   
}//text

//---------------------------------------------------------------------------
void setLinePlotFlag(int flg)
{
  //Only use with single trace
  lineFlag = flg;
}

//---------------------------------------------------------------------------
void changePlotMode(int p) 
{
   
   plotMode = p;
   setLinePlotFlag(0); //Ensure we star anew for line plots
    
}

//---------------------------------------------------------------------------
void changeLogPlotMode(int p) 
{
   logPlotMode = p;
}

//---------------------------------------------------------------------------
void xPlotIncr(int xincr)
{
   xincrement = xincr;
}

//---------------------------------------------------------------------------
void circleSize(int cSize)
{
   cirSize = cSize;
}



// This function to be used in animated plots (e.g. oscilloscope)
// where information can be updated in real time.
// Each box contains a single item with caption and data.
// This function provides the legend captions.
// Provides up to four legend boxes side by side.
// To be used with the legend(.) function which provides the data.  
void legendCaptions(struct leg_captions l_caps,GdkRGBA col, int numBoxes, int leg_left, int leg_top)
{
   
	 int color;
	 int x_adjust = 5;
	 int y_adjust = 5;
	  
	//cairo_t *cr = cairo_create (surface);
	//context = cairo_create (surface);
	cairo_select_font_face(context, "Arial",	CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size(context, 13);
      
   
	int left = leg_left;
	int top = leg_top;
		  
	int box_width = round ((X_FORM_SIZE - 2 * xmargin) / numBoxes);
		
	
    
	for(int i = 0; i < numBoxes; i++)
	{
		
		//setcolor(LIGHTGRAY);
		color = DARKGRAY; //Set color for box borders
		cairo_set_source_rgb(context, getGdkColor(color).red, getGdkColor(color).green, getGdkColor(color).blue); 
		cairo_rectangle (context, left, leg_top-20, box_width, 20);		
		cairo_stroke (context);			
		
		color = BLACK; //Background color for legend
		cairo_set_source_rgb(context, getGdkColor(color).red, getGdkColor(color).green, getGdkColor(color).blue);
		cairo_rectangle (context, left, leg_top-20, box_width, 20);		
   	   	cairo_fill (context);
		
		//Write captions:
		//setfontcolor(col);
		cairo_set_source_rgb(context, col.red, col.green, col.blue);
		if ( i == 0) 
		{
			//outtextxy(left,top,l_caps.data1_caption);	
			cairo_move_to(context, left+x_adjust,top-y_adjust);
			cairo_show_text(context, l_caps.data1_caption);
 		}  
		else if (i == 1)
		{
			//outtextxy(left,top,l_caps.data2_caption);
			cairo_move_to(context, left+x_adjust,top-y_adjust);
			cairo_show_text(context, l_caps.data2_caption);		
		}
		else if ( i == 2)
		{
			//outtextxy(left,top,l_caps.data3_caption);
			cairo_move_to(context, left+x_adjust,top-y_adjust);
			cairo_show_text(context, l_caps.data3_caption);	
		}	
		else if (i == 3)
		{
			//outtextxy(left,top,l_caps.data4_caption);
			cairo_move_to(context, left+x_adjust,top-y_adjust);
			cairo_show_text(context, l_caps.data4_caption);	
		}	
		
		
		//left += 144;
		left += box_width;  
   }
	

}//legendCaptions



// Legend function to be used for non-animated plots
void legendCaptions1(GdkRGBA col,int bck_col,int numPlots,struct legend_info info[], int left, int top,int width,int height)
{
   
	//Set up captions with associated plot symbols:
	//Last legend to a theoretical (line) -if required
	//MAX of four  caption lines!!
         
	//cairo_t *cr = cairo_create (surface);
	//context = cairo_create (surface);
	cairo_select_font_face(context, "Arial",	CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size(context, 13);
       
   	int color = bck_col; 
	 
	cairo_set_source_rgb(context, getGdkColor(color).red, getGdkColor(color).green, getGdkColor(color).blue); 
	cairo_rectangle (context,left-5,top-5,left+width,top+height);		
	cairo_stroke (context);			

	 
   	cairo_set_source_rgb(context, getGdkColor(color).red, getGdkColor(color).green, getGdkColor(color).blue); 
	cairo_rectangle (context,left-5,top-5,left+width,top+height);		
	cairo_fill (context); 

      
   //Set font color for caption text:
   cairo_set_source_rgb(context, col.red, col.green, col.blue);
   
   int ypos = 10;
   for(int i=0;i<numPlots;i++)
   {
	   	//First Legend:
		cairo_set_source_rgb(context, getGdkColor(info[i].color).red, getGdkColor(info[i].color).green, getGdkColor(info[i].color).blue);
		cairo_move_to(context,left,top+ypos);
		cairo_show_text(context, info[i].text);
		cairo_stroke (context);
		
		cairo_set_source_rgb(context, getGdkColor(info[i].color).red, getGdkColor(info[i].color).green, getGdkColor(info[i].color).blue);
		if(info[i].type == CIRCLE_BORDER)
		{
			circleSize(5);
			cairo_arc (context,left+(width-15),top+ypos-5,cirSize,0,2*3.142);
			cairo_stroke (context);
			
		}
		if(info[i].type == CIRCLE_FILLED)
		{
			circleSize(5);
			cairo_arc (context,left+(width-15),top+ypos-5,cirSize,0,2*3.142);
			cairo_fill (context);
		}
		
		if(info[i].type == SQUARE_BORDER)
		{
			circleSize(8);
			cairo_rectangle (context,left+(width-20),top+ypos-8, cirSize,cirSize);		
			cairo_stroke (context); 

			
		}
		if(info[i].type == SQUARE_FILLED)
		{
			circleSize(8);
			cairo_rectangle (context,left+(width-20),top+ypos-8,cirSize,cirSize);		
			cairo_fill (context); 

		}   
		if(info[i].type == TRIANGLE_BORDER)
		{
			circleSize(9);
			triangle(left+(width-15),top+ypos-8,cirSize,TRIANGLE_BORDER,info[i].color);
		}
		if(info[i].type == TRIANGLE_FILLED)
		{
			circleSize(9);
			triangle(left+(width-15),top+ypos-8,cirSize,TRIANGLE_FILLED,info[i].color);
		}
		
		
		if(info[i].type == LINE)
		{
			printf ("doing LINE!\n");
			cairo_move_to(context, left+(width-20),top+ypos-3);
			cairo_line_to(context,left+(width-20)+20,top+ypos-3);
			cairo_stroke (context);
		
		}
		 
		 ypos += 20;	
			
   }//i	
   
      
    //Put border around the legend with another color:
	cairo_set_source_rgb(context, col.red, col.green, col.blue);
	cairo_rectangle (context,left-5,top-5,left+width,top+height);		
	cairo_stroke (context);			
	   
      

}//legendCaptions1


//---------------------------------------------------------------------------
// To be used with animated plots (in association with legendCaptions(.))
void legend(struct leg_data l_data,bool refresh, int numBoxes, int color, int leg_left, int leg_top)
{
   
   
	//NB: If working for non animating graph, refresh is set to zero
   
	char cap1[30], cap2[30], cap3[30], cap4[30];
	         
	//int left = leg_left + 4;
	int left = leg_left;
	int top = leg_top + 2;
   
   int box_width = round ((X_FORM_SIZE - 2 * xmargin) / numBoxes);
   
	//cairo_t *cr = cairo_create (surface);
	//context = cairo_create (surface);
	
	cairo_select_font_face(context, "Arial",	CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size(context, 13);
     
      
    
   if(refresh)  //Repaint to background previous value  
   {
       //setfontcolor(m_cMonoBackCol);
       cairo_set_source_rgb(context, m_cMonoBackCol.red, m_cMonoBackCol.green, m_cMonoBackCol.blue); 
	   
   }  
   else
   {       
          //setfontcolor(color); //10-light green; 5-Magenta
          cairo_set_source_rgb(context, getGdkColor(color).red, getGdkColor(color).green, getGdkColor(color).blue);
   }      
      
   
   int shift = 0;
   //int top_high = top + 8;
   int top_low = top - 22; //4;
   int left_start = left+45;
   //int left_start = left;
   //Adjust box region for painting;
   int box_width_mask = 45;
   
   //setcolor (BLACK);
   int b_color = BLACK;  // Define background color
   
   float data;
   for(int i=0; i < numBoxes;i++)
   {
   
		if (i == 0)
		{
			
			// Set background color:
			cairo_set_source_rgb(context, getGdkColor(b_color).red, getGdkColor(b_color).green, getGdkColor(b_color).blue);
   			cairo_rectangle (context,shift+left_start,top_low, box_width_mask,20);
			cairo_fill (context); 

			
			if (isnan(l_data.data1))
			{
				sprintf(cap1,"%s  %s","XXXX",l_data.data1_unit);
			} 
			 else
			 {  	   
			   
				if ( (l_data.data1 <= 0.0))
				{ 
					data = l_data.data1;
					sprintf(cap1,"%5.2f  %s", data,l_data.data1_unit);
				}
				else if ( (l_data.data1 < 1.0) && (l_data.data1 > 1.0e-3))
				{ 
					
					data = l_data.data1 * 1000;
					sprintf(cap1,"%5.2f  %c%s", data,'m',l_data.data1_unit);
				}
				else if ( (l_data.data1 >= 1.0) && (l_data.data1 < 1.0e3))
				{ 
					data = l_data.data1;
					sprintf(cap1,"%5.2f  %s",data,l_data.data1_unit);
				}
				else if ( (l_data.data1 >= 1.0e3) && (l_data.data1 < 1.0e6))
				{ 
					data = l_data.data1 / 1000;
					sprintf(cap1,"%5.2f  %c%s",data,'k',l_data.data1_unit);
				}
				else if ( l_data.data1 >= 1.0e6)
				{ 
					data = l_data.data1 / 1000000;
					sprintf(cap1,"%5.2f  %c%s",data,'M',l_data.data1_unit);
				}
			}
												
			//Set text color
			cairo_set_source_rgb(context, getGdkColor(color).red, getGdkColor(color).green, getGdkColor(color).blue);
   			cairo_move_to(context,left+shift+50,top-8);
			cairo_show_text(context, cap1);
			cairo_stroke (context);
		
			   
		}
		else if (i == 1)
		{
			
			// Set background color:
			cairo_set_source_rgb(context, getGdkColor(b_color).red, getGdkColor(b_color).green, getGdkColor(b_color).blue);
   			cairo_rectangle (context,shift+left_start,top_low, box_width_mask,20);
			cairo_fill (context); 

   	   
			if (isnan(l_data.data2))
			{
				sprintf(cap2,"%s  %s","XXXX",l_data.data2_unit);
			}   	
			else
			{     	      	      	   	   
				if ( (l_data.data2 <= 0.0))
				{ 
					data = l_data.data2;
					sprintf(cap2,"%5.2f  %s",data,l_data.data2_unit);
				}
				else if ( (l_data.data2 < 1.0) && (l_data.data2 > 1.0e-3))
				{ 
					
					data = l_data.data2 * 1000;
					sprintf(cap2,"%5.2f  %c%s", data,'m',l_data.data2_unit);
				}
				else if ( (l_data.data2 >= 1.0) && (l_data.data2 < 1.0e3))
				{ 
					data = l_data.data2;
					sprintf(cap2,"%5.2f  %s",data,l_data.data2_unit);
				}
				else if ( (l_data.data2 >= 1.0e3) && (l_data.data2 < 1.0e6))
				{ 
					data = l_data.data2 / 1000;
					sprintf(cap2,"%5.2f  %c%s",data,'k',l_data.data2_unit);
				}
				else if ( l_data.data2 >= 1.0e6)
				{ 
					data = l_data.data2 / 1000000;
					sprintf(cap2,"%5.2f  %c%s",data,'M',l_data.data2_unit);
				}			
			}
			//Set text color
			cairo_set_source_rgb(context, getGdkColor(color).red, getGdkColor(color).green, getGdkColor(color).blue);
   			cairo_move_to(context,left+shift+50,top-8);
			cairo_show_text(context, cap2);
			cairo_stroke (context);
		
			
		}
		else if (i == 2)
		{
						
			// Set background color:
			cairo_set_source_rgb(context, getGdkColor(b_color).red, getGdkColor(b_color).green, getGdkColor(b_color).blue);
   			cairo_rectangle (context,shift+left_start,top_low, box_width_mask,20);
			cairo_fill (context); 

			if (isnan(l_data.data3))
			{
				sprintf(cap3,"%s  %s","XXXX",l_data.data3_unit);
			}
			else
			{			
						
				if ( (l_data.data3 <= 0.0))
				{ 
					data = l_data.data3;
					sprintf(cap3,"%5.2f  %s", data,l_data.data3_unit);
				}
				else if ( (l_data.data3 < 1.0) && (l_data.data3 > 1.0e-3))
				{ 
					
					data = l_data.data3 * 1000;
					sprintf(cap3,"%5.2f  %c%s", data,'m',l_data.data3_unit);
				}
				else if ( (l_data.data3 >= 1.0) && (l_data.data3 < 1.0e3))
				{ 
					data = l_data.data3;
					sprintf(cap3,"%5.2f  %s",data,l_data.data3_unit);
				}
				else if ( (l_data.data3 >= 1.0e3) && (l_data.data3 < 1.0e6))
				{ 
					data = l_data.data3 / 1000;
					sprintf(cap3,"%5.2f  %c%s",data,'k',l_data.data3_unit);
				}
				else if ( l_data.data3 >= 1.0e6)
				{ 
					data = l_data.data3 / 1000000;
					sprintf(cap3,"%5.2f  %c%s",data,'M',l_data.data3_unit);
				}
		   }
		   								
			//Set text color
			cairo_set_source_rgb(context, getGdkColor(color).red, getGdkColor(color).green, getGdkColor(color).blue);
   			cairo_move_to(context,left+shift+50,top-8);
			cairo_show_text(context, cap3);
			cairo_stroke (context);
		
			
		}
		else if (i == 3)
		{
			
			
			// Set background color:
			cairo_set_source_rgb(context, getGdkColor(b_color).red, getGdkColor(b_color).green, getGdkColor(b_color).blue);
   			cairo_rectangle (context,shift+left_start,top_low, box_width_mask,20);
			cairo_fill (context); 

   	   
			if (isnan(l_data.data4))
			{
				sprintf(cap4,"%s  %s","XXXX",l_data.data4_unit);
			}
			else
			{	
			
				if ( (l_data.data4 <= 0.0))
				{ 
					data = l_data.data4;
					sprintf(cap4,"%5.2f  %s",data,l_data.data4_unit);
				}
				else if ( (l_data.data4 < 1.0) && (l_data.data4 > 1.0e-3))
				{ 
					data = l_data.data4 * 1000;
					sprintf(cap4,"%5.2f  %c%s", data,'m',l_data.data4_unit);
				}
				else if ( (l_data.data4 >= 1.0) && (l_data.data4 < 1.0e3))
				{ 
					data = l_data.data4;
					sprintf(cap4,"%5.2f  %s",data,l_data.data4_unit);
				}
				else if ( (l_data.data4 >= 1.0e3) && (l_data.data4 < 1.0e6))
				{ 
					data = l_data.data4 / 1000;
					sprintf(cap4,"%5.2f  %c%s",data,'k',l_data.data4_unit);
				}
				else if ( l_data.data4 >= 1.0e6)
				{ 
					data = l_data.data4 / 1000000;
					sprintf(cap4,"%5.2f  %c%s",data,'M',l_data.data4_unit);
				}
			}			
   	   
			//Set text color
			cairo_set_source_rgb(context, getGdkColor(color).red, getGdkColor(color).green, getGdkColor(color).blue);
   			cairo_move_to(context,left+shift+50,top-8);
			cairo_show_text(context, cap4);
			cairo_stroke (context);
		
				
		}	
		
				
		shift += box_width;     
   }
     
   

}//legend



//---------------------------------------------------------------------------
void axisDefaults(int xdivs,int ydivs, char *xspec,
                              char *yspec)
{
   xnoDivs = xdivs;   //Redefine no. scale divs. along x-axis
   ynoDivs = ydivs;   //Redefine no. scale divs. along y-axis
   strcpy(xfield,xspec);  //Redefine field specification: x scaling
   strcpy(yfield,yspec);  //Redefine field specification: y scaling
		
}//axisDefaults

//---------------------------------------------------------------------------
void setxMinLinear(int min)
{
   xminLinear=min;
}

//---------------------------------------------------------------------------
int getxMinLinear()
{
   return xminLinear;
}

//---------------------------------------------------------------------------
void setxMaxLinear(int max)
{
   xmaxLinear=max;
}

//---------------------------------------------------------------------------
int getxMaxLinear()
{
   return xmaxLinear;
}

//---------------------------------------------------------------------------
void setxMin(int min)
{
   xmin=min;
}

//---------------------------------------------------------------------------
void setxMax(int max)
{
   xmax=max;
}

//---------------------------------------------------------------------------
void setyMin(int min)
{
   ymin1=min;
}

//---------------------------------------------------------------------------
void setyMax(int max)
{
   ymax1=max;
}

//---------------------------------------------------------------------------
void setGridFlg(bool flg)
{
   m_bGrid=flg;
}

//---------------------------------------------------------------------------
bool getGridFlg()
{
   return m_bGrid;
}

//---------------------------------------------------------------------------
void setxgrad(bool flg)
{
   xgrad=flg;
}

//---------------------------------------------------------------------------
bool getxgrad()
{
   return xgrad;
}

//---------------------------------------------------------------------------
void initialisePizPos()
{
   counter1=left1;
   counter2=left2;
}

//---------------------------------------------------------------------------
float getXValFromPixPos(int npixels)
{

   int X;
   int XVal = 0;;

   if(image == 1)
   {
      X = right1 - left1;


      if(xmin==0)
      {
         XVal = ((npixels-xmargin)*xmax )/X;
         return XVal;
      }
      else if(xmax==0)
      {
        XVal = xmin - ((npixels-xmargin)*xmin )/X;
        return XVal;
      }
      else
      {
         float D = fabs(xmin) + fabs(xmax);
         float R = fabs(xmin)/D ;
         XVal = ((npixels-xmargin)*D)/X;
         if(XVal <= R*D)
         {
            XVal = xmin + XVal;
         }
         else
         {
            XVal = XVal - R*D;
         }
         return XVal;
      }
      

   }//if(image == 1)

   return XVal;
}//getXValueFromPixPos



GdkRGBA getGdkColor (int color)
{
	GdkRGBA g_color;
	
	switch (color)
	{
		case BLACK:
		g_color.red = 0.0;
		g_color.green = 0.0;
		g_color.blue = 0.0;
		g_color.alpha = 0.0;
		break;
		
		case BLUE:
		g_color.red = 0.0;
		g_color.green = 0.0;
		g_color.blue = 1.0;
		g_color.alpha = 0.0;
		break;
		
		case GREEN:
		g_color.red = 0.0;
		g_color.green = 1.0;
		g_color.blue = 0.0;
		g_color.alpha = 0.0;
		break;
		
		case CYAN: //TBD
		g_color.red = 0.0;
		g_color.green = 0.0;
		g_color.blue = 0.0;
		g_color.alpha = 0.0;
		break;
		
		case RED:
		g_color.red = 1.0;
		g_color.green = 0.0;
		g_color.blue = 0.0;
		g_color.alpha = 0.0;
		break;
		
		case MAGENTA://TBD
		g_color.red = 0.0;
		g_color.green = 0.0;
		g_color.blue = 0.0;
		g_color.alpha = 0.0;
		break;
		
		case BROWN://TBD
		g_color.red = 0.0;
		g_color.green = 0.0;
		g_color.blue = 0.0;
		g_color.alpha = 0.0;
		break;
		
		case LIGHTGRAY://TBD
		g_color.red = 0.0;
		g_color.green = 0.0;
		g_color.blue = 0.0;
		g_color.alpha = 0.0;
		break;
		
		case DARKGRAY://TBD
		g_color.red = 0.663;
		g_color.green = 0.663;
		g_color.blue = 0.663;
		g_color.alpha = 0.0;
		
		break;
		
		case LIGHTBLUE://TBD
		g_color.red = 0.0;
		g_color.green = 0.0;
		g_color.blue = 0.0;
		g_color.alpha = 0.0;
		break;
		
		case LIGHTGREEN://TBD
		g_color.red = 0.0;
		g_color.green = 0.0;
		g_color.blue = 0.0;
		g_color.alpha = 0.0;
		break;
		
		case LIGHTCYAN://TBD
		g_color.red = 0.0;
		g_color.green = 0.0;
		g_color.blue = 0.0;
		g_color.alpha = 0.0;
		break;
		
		case LIGHTRED://TBD
		g_color.red = 0.0;
		g_color.green = 0.0;
		g_color.blue = 0.0;
		g_color.alpha = 0.0;
		break;
		
		case LIGHTMAGENTA://TBD
		g_color.red = 0.0;
		g_color.green = 0.0;
		g_color.blue = 0.0;
		g_color.alpha = 0.0;
		break;
		
		case YELLOW://TBD
		g_color.red = 1.0;
		g_color.green = 1.0;
		g_color.blue = 0.0;
		g_color.alpha = 0.0;
		break;
		
		case WHITE:
		g_color.red = 1.0;
		g_color.green = 1.0;
		g_color.blue = 1.0;
		g_color.alpha = 0.0;
		break;
	}	
	
	return g_color;
		


}//getGdkColor

#endif
