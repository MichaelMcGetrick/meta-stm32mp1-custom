//---------------------------------------------------------------------------
#ifndef GRAPHIC_HPP
#define GRAPHIC_HPP


// DEFINE THE REQUIRED GRAPHICS LIBRARY
//-----------------------------------------------------------------------------
//#define LIBGRAPH	//Graphics library based on Turbo C graphics implementation
#define GTK			// GTK 3+
//-----------------------------------------------------------------------------

#include <stdbool.h>
#ifdef LIBGRAPH
#include <graphics.h>  //Turbo C based graphics
#endif
#ifdef GTK
#include <gtk/gtk.h>
#endif

//#define BASE_CLASS_IMPLEMENTATION_ONLY true
/*-----------------------------------------------------------------------------
   AUTHOR:            M.J. McGetrick
                      (c) Copyright 2020 M.J. McGetrick
   DATE:
   DESCRIPTION:       Graphic_lx
                      Suite of functions for simple graph plotting using
                      the Linux Turbo C graphics library.
                      
                       
------------------------------------------------------------------------------*/
#ifdef GTK
#define BLACK			0
#define BLUE			1
#define GREEN			2
#define CYAN			3
#define RED				4
#define MAGENTA			5
#define BROWN			6
#define LIGHTGRAY		7
#define DARKGRAY		8
#define LIGHTBLUE		9
#define LIGHTGREEN		10
#define LIGHTCYAN		11
#define LIGHTRED		12
#define LIGHTMAGENTA	13
#define YELLOW			14
#define WHITE			15
#endif


#ifdef LIBGRAPH
//ATTRIBUTES:
         
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
        char xLabel[100],yLabel[100], Title[100];
        int image;
        int buffCount1, buffCount2;
        //TRect MonoGraphRectangle;
        bool showXAxisNums,showYAxisNums,m_bGrid;
        bool Bitmap;
        char m_sPrinterName[100];
        int m_iFormHeight, m_iFormWidth;
        int m_cMonoBackCol;
                
        bool show_ori_line;   
        bool showXGradLines, showYGradLines;  
        
        // Width of a line plot in pixels
        // Should be defined as an odd number (for plot symmetry)
        // Even numbers shall be reduced by 1 in plot routine
        int line_width;  
        
        
        
        //PLOT MODE ENUMERATIONS
        enum PlotModes{ LINE,
			               PIXEL,
			               CIRCLE_BORDER,
			               CIRCLE_FILLED,
			               SQUARE_BORDER,
			               SQUARE_FILLED,
			               TRIANGLE_BORDER,
			               TRIANGLE_FILLED };
	  
		                 
        
        //LEGEND ATTRIBUTES
        struct leg_captions     //This can be expanded as and when required
        {
                char* data1_caption;
                char* data2_caption;
                char* data3_caption;
                char* data4_caption;
                
        };           

        
        struct legend_info     //This can be expanded as and when required
        {
                int type;   //Data plot symbol
                int color;  //Color for plot
                char* text; //Caption for this data
                
        };
        
        	  

        struct leg_data     //This can be expanded as and when required
        {
                float data1;    
                float data2;    
                float data3;  
                float data4; 
                char *data1_unit;
                char *data2_unit;
                char *data3_unit;
                char *data4_unit;   
                 
        };           
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

//FUNCTION PROTOTYPES ---------------------------------------------------------
        
       void Graphic_Init(int xFormSize,int yFormSize, int pMode,int logMode,
               char *xlab, char *ylab, char *gphtitle,
               float yMin, float yMax, float xMin, float xMax,
               int xInt, int yInt, bool xGrad, bool yGrad, int XAxisPrec, int YAxisPrec,
               bool grid, int gInt,
               bool xshow,bool yshow,bool ori, int ori_color,bool bitmap,
               int monobcolor,char *audfile, char *audInifile,
               int chans,int samrate, int bps);
        
        
        void setLinePlotFlag(int flg); //Only use with single trace
        void setupGraphSize(void);
        void setMonoGraphBackground(int color);
        void setupWindow(int image,int left, int top, int right, int bottom,
                         char *xlabel, char *ylabel);
        void title(char *text);         //Only for mono-trace
        void xAxisText(int image,char *text);
        void yAxisText(int image,char *text);
        void drawOriginLine();
        void tagOrigin(int image,int left, int posOrig);
        void graduations(int color); //Only for mono-trace
        void plotSig(float ySig, float xSig,int color);
        void triangle(int x, int y, int size, int type); 
        int positionPixel(int image,float ymin,float ymax,float sig,int pixelMode);
        void pixelVerLine(int x, int pixelIncr);
        void pixelHorLine(int y, int pixelIncr);
        void xAxisScale(); //Only for mono-trace
        void yAxisScale(); //Only for mono-trace
        void axisScales();
        void drawVerticalMarker(float pos, int color, int width);
        void setupGrid(int xInt, int yInt);
        void verLines(int xInterval);
        void horLines(int yInterval);
        void text(int image,float x,float y,char *caption);
        void changePlotMode(int p);
        void changeLogPlotMode(int p);
        void xPlotIncr(int xincr);
        void circleSize(int cSize);
        void lineWidth(int width); 
        void legend(struct leg_data l_data, bool refresh, int numBoxes,int color, int leg_left, int leg_top);
        void legendCaptions(struct leg_captions l_caps,GdkRGBA col, int numBoxes, int leg_left, int leg_top);
        //void legendCaptions1(struct leg_captions l_caps,int col,int numPlots,struct leg_symbols sym[]);
        void legendCaptions1(int col,int bck_color,int numPlots,struct legend_info info[],int left, int top,int width,int height); 
        void axisDefaults(int xdivs,int ydivs,char *xspec, char *yspec);
        void setxMinLinear(int min);
        int getxMinLinear();
        void setxMaxLinear(int max);
        int getxMaxLinear();
        void setxMin(int min);
        void setxMax(int max);
        void setyMin(int min);
        void setyMax(int max);
        void setGridFlg(bool flg);
        bool getGridFlg();
        void setxgrad(bool flg);
        bool getxgrad();
        float getXValFromPixPos(int npixels);
        void plotVerLine(float ySig,float xSig);
        void initialisePizPos();
#endif
#ifdef GTK
//ATTRIBUTES:

		extern cairo_surface_t *surface;
		extern cairo_t *context;
		extern int surface_width;
		extern int surface_height;
		extern GdkRGBA black, white, dark_gray;

         
         //Width/Height of GUI form
         extern int X_FORM_SIZE, Y_FORM_SIZE;  
                 
        extern bool showGrid, xgrad, ygrad;
        extern int plotMode, gridInt;
        extern int xincrement, cirSize, logPlotMode, lineFlag,
            xmargin, ymargin;
        extern float ymin1, ymin2, ymax1, ymax2, xmin, xmax, sigStore1,
              sigStore2;
        extern float xminLinear,xmaxLinear;
        extern float ymaxVal1,ymaxVal2;
        extern int left, top, right, bottom,C1, counter1, counter2;
        extern int left1,top1,right1,bottom1,left2,top2,right2,bottom2,C2;
        extern int xnoDivs, ynoDivs;
        extern char xfield[4], yfield[4];
        extern char xLabel[100],yLabel[100], Title[100], WinTitle[100];
        extern int image;
        extern int buffCount1, buffCount2;
        //TRect MonoGraphRectangle;
        extern bool showXAxisNums,showYAxisNums,m_bGrid;
        extern bool Bitmap;
        extern char m_sPrinterName[100];
        extern int m_iFormHeight, m_iFormWidth;
        extern GdkRGBA m_cMonoBackCol;
                
        extern bool show_ori_line;   
        extern bool showXGradLines, showYGradLines;  
        
        // Width of a line plot in pixels
        // Should be defined as an odd number (for plot symmetry)
        // Even numbers shall be reduced by 1 in plot routine
        extern int line_width;  
        
               
        //PLOT MODE ENUMERATIONS
        enum PlotModes{ LINE,
			               PIXEL,
			               CIRCLE_BORDER,
			               CIRCLE_FILLED,
			               SQUARE_BORDER,
			               SQUARE_FILLED,
			               TRIANGLE_BORDER,
			               TRIANGLE_FILLED };
	  
		                 
        
        //LEGEND ATTRIBUTES
        struct leg_captions     //This can be expanded as and when required
        {
                char* data1_caption;
                char* data2_caption;
                char* data3_caption;
                char* data4_caption;
                
        };           

        
        struct legend_info     //This can be expanded as and when required
        {
                int type;   //Data plot symbol
                int color;  //Color for plot
                char* text; //Caption for this data
                
        };
        
        	  

        struct leg_data     //This can be expanded as and when required
        {
                float data1;    
                float data2;    
                float data3;  
                float data4; 
                char *data1_unit;
                char *data2_unit;
                char *data3_unit;
                char *data4_unit;   
                 
        };           
        extern struct leg_captions l_caps;
        extern struct leg_data l_data;



        //Audio player members:
        extern char m_szAudioIniFile[200],m_sFile[200];
        extern int m_iNumChannels, m_iSampleRate, m_iBitsPerSample;
        extern int m_iCurrSampleNum;
        //TColor m_cWaveSigColor, m_cDataPlotColor, m_cAudioMarkerColor,
        //       m_cAudioClipMarkerColor;
        extern int m_iSigUpper, m_iSigLower;
        extern bool m_bSigTrace,m_bSigRangeDefined;

        
        //frequency spectrum:
        extern int m_iFreqIndex;
        extern bool m_bHarmonic;  //Flag for plotting frequency harmonics

        //Colours for graphics:
        //TColor m_cBackground,m_cGridlines,m_cSignal,m_cSpectrum, m_cHarmonics,
        //m_cSFmarker, m_cOrigin;
        extern bool m_bAnimated,m_bResize;
        extern char m_sFileExt[4];
        extern bool m_bIsWaveFile;

        //Data for bitmap image:
        //intmat m_iImageData;
        extern int m_iBitsPerPixel;

//FUNCTION PROTOTYPES ---------------------------------------------------------
        
       void Graphic_Init(int argc, char * argv[], int xFormSize,int yFormSize, int pMode,int logMode,
               char *xlab, char *ylab, char *win_title, char *gphtitle, int gph_refresh_rate,
               float yMin, float yMax, float xMin, float xMax,
               int xInt, int yInt, bool xGrad, bool yGrad, int XAxisPrec, int YAxisPrec,
               bool grid, int gInt,
               bool xshow,bool yshow,bool ori, GdkRGBA ori_color,bool bitmap,
               GdkRGBA monobcolor,char *audfile, char *audInifile,
               int chans,int samrate, int bps);
        
               
        
        
        void setLinePlotFlag(int flg); //Only use with single trace
        void setGtkContext(cairo_t *cr); 
        void setupGraphSize(void);
        void setMonoGraphBackground(GdkRGBA color);
        void setupWindow(int image,int left, int top, int right, int bottom,
                         char *xlabel, char *ylabel);
        void drawGraphBackground ();                 
        void title(char *text);         //Only for mono-trace
        void xAxisText(int image,char *text);
        void yAxisText(int image,char *text);
        void drawOriginLine();
        void tagOrigin(int image,int left, int posOrig);
        void graduations(GdkRGBA color); //Only for mono-trace
        void plotSig(float ySig, float xSig,int color);
        void plotLine(float ySig, float xSig,int color,bool start, bool end);
        void triangle(int x, int y, int size, int type, int color); 
        int positionPixel(int image,float ymin,float ymax,float sig,int pixelMode);
        void pixelVerLine(GdkRGBA color, int x, int pixelIncr);
        void pixelHorLine(GdkRGBA color, int y, int pixelIncr);
        void xAxisScale(); //Only for mono-trace
        void yAxisScale(); //Only for mono-trace
        void axisScales();
        void drawVerticalMarker(float pos, GdkRGBA color, int width);
        void setupGrid(int xInt, int yInt);
        void verLines(int xInterval);
        void horLines(int yInterval);
        void text(int image,float x,float y,char *caption);
        void changePlotMode(int p);
        void changeLogPlotMode(int p);
        void xPlotIncr(int xincr);
        void circleSize(int cSize);
        void lineWidth(int width); 
        void legend(struct leg_data l_data, bool refresh, int numBoxes,int color, int leg_left, int leg_top);
        void legendCaptions(struct leg_captions l_caps,GdkRGBA col, int numBoxes, int leg_left, int leg_top);
        //void legendCaptions1(struct leg_captions l_caps,int col,int numPlots,struct leg_symbols sym[]);
        void legendCaptions1(GdkRGBA col,int bck_color,int numPlots,struct legend_info info[],int left, int top,int width,int height); 
        void axisDefaults(int xdivs,int ydivs,char *xspec, char *yspec);
        void setxMinLinear(int min);
        int getxMinLinear();
        void setxMaxLinear(int max);
        int getxMaxLinear();
        void setxMin(int min);
        void setxMax(int max);
        void setyMin(int min);
        void setyMax(int max);
        void setGridFlg(bool flg);
        bool getGridFlg();
        void setxgrad(bool flg);
        bool getxgrad();
        float getXValFromPixPos(int npixels);
        void plotVerLine(float ySig,float xSig);
        void initialisePizPos();	
        GdkRGBA getGdkColor (int color);	
        	

#endif        
                                       
        
//---------------------------------------------------------------------------
#endif
