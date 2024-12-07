/* 
PROGRAM NAME: gtk_cairo
DESCRIPTION: Test program to display sampled data on the TFT/MIPI DSI Display of the STM32MP157F-DK2 
AUTHOR: Michael McGetrick
*/

#include <gtk/gtk.h>
#include <cairo.h>
#include <string.h>
#include <math.h>
#include "graph.h"
#include "mcp3008.h"


//----------------------------------------------------------------
// DEFINES
//----------------------------------------------------------------
#define NUM_POINTS 		(1000u)
#define PERIOD 			(200u)

#define SAMPLING_TYPE	1 //0: Simulation; 1: Real sampling from MCP
#define WINDOW_TITLE	"MJMTEK"
#define		DIAGNOSTICS

//----------------------------------------------------------------
// GLOBAL VARIABLES
//----------------------------------------------------------------
static pthread_t drawing_thread, poll_thread;
static pthread_mutex_t mutex;

GdkRGBA bck_color;                 
//GdkRGBA ori_line_color;
GdkRGBA ver_marker_color;
GtkWidget *DrawingArea;


FILE *fp;
char *filename = "spi_data_index0.txt";
bool saveToFile = false;

uint64_t sample_delay;
float data[DATA_LEN];

struct timespec req,rem;

int EXIT_PROGRAM = 	0;

bool draw_flg = false;

//----------------------------------------------------------------
// Function declarations
//----------------------------------------------------------------
static void * draw_thread (void *);
static void * data_poll_thread (void *);
static float sine_to_point (int x, int width, int height);
static void simulation ( void );
static void init_graph( int argc, char * argv[] );
static void drawLegend ();
static void sys_delay(void);
static void ctrl_c_handler(int sig);
static void plotData (void);
static void plotData1 (void);


/* Local function prototypes. */
static gboolean invalidate_cb (void *);
static gboolean drawing_area_configure_cb (GtkWidget *, GdkEventConfigure *);
static void drawing_area_draw_cb (GtkWidget *, cairo_t *, void *);

static gboolean invalidate_cb (void *ptr)
{
	//printf ("In invalidate funtion!\n");
	usleep (1E6 / 1000);
	  if (GTK_IS_WIDGET (ptr))
	  {
	  //printf ("Calling Redraw!\n");
	    gtk_widget_queue_draw (GTK_WIDGET (ptr));
	    return TRUE;
	   }

  	return FALSE;
}



static gboolean drawing_area_configure_cb (GtkWidget *widget, GdkEventConfigure *event)
{
  if (event -> type == GDK_CONFIGURE)
  {
    pthread_mutex_lock (&mutex);

    if (surface != (cairo_surface_t *)NULL)
    {
      cairo_surface_destroy (surface);
    }

    GtkAllocation allocation;
    gtk_widget_get_allocation (widget, &allocation);
    surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, allocation.width, allocation.height);
    surface_width = allocation.width;
    surface_height = allocation.height;	

     //Initialise graph by forcing a draw - TBD: FIX IT
     
     usleep (1E6 / 1000);
     invalidate_cb ((void*)DrawingArea);

    pthread_mutex_unlock (&mutex);
  }

  return TRUE;
}



static void drawing_area_draw_cb (GtkWidget *widget, cairo_t *context, void *ptr)
{
  // Copy the contents of the surface to the current context. 
  
  pthread_mutex_lock (&mutex);
  static int cnt = 0;
  printf ("Redraw called: cnt: %d\n",cnt);
  
  if (surface != (cairo_surface_t *)NULL)
  {
    cairo_set_source_surface (context, surface, 0, 0);
    cairo_paint (context);
        
  }
  
  cnt ++;
  pthread_mutex_unlock (&mutex);
  	
}





//----------------------------------------------------------------
// FUNCTION DEFINITIONS
//----------------------------------------------------------------

static void * draw_thread (void *ptr)
{
		
	if (SAMPLING_TYPE == 0 )
	{
		//Simulation
		simulation (); 
		
	}
	else
	{
		// Real data sampling
		plotData1 ();
			
	}			

	return NULL;  
}


static void simulation ( void )
{
  int redraw_number = 0;

 static int cnt = 0;	
  while ( 1 )
  {
     usleep (1E6 / GPH_REFRESH_RATE); // Sleep for required interval.
     if (surface == (cairo_surface_t *)NULL)
     {
      continue;
     }

    pthread_mutex_lock (&mutex);
    
    cairo_t *context = cairo_create (surface);
    setGtkContext (context); //Set the graphics library context
    

    // Draw the background. 

	
	//Plot color:
	 cairo_set_source_rgb (context, 0.5, 0.5, 0); //temp
	int plot_color = YELLOW;
	int width = 0; //Dummy variable
	int amplitude = 2;
			
	refreshGraphBackground();
	drawLegend (true);
	
	// Plot the current trace
	for (int i = 0; i < DATA_LEN; i++)
	{	
		plotSig(sine_to_point (i+redraw_number, width, amplitude),i,plot_color);
		
	}
	setLinePlotFlag(0); //Ensure we do not use last position of current plot to draw (lineplot)
	redraw_number += 5;
    //cairo_stroke (context);
    cairo_destroy (context);
  printf ("Simulation draw count: %d\n", cnt);
  cnt ++;	
		
    pthread_mutex_unlock (&mutex);
  }
  
	
}// simulation	


static void plotData ( void )
{

  static unsigned int cnt = 0;
  //while ( 1 )
  //{
     //usleep (1E6 / GPH_REFRESH_RATE); // Sleep for required interval.
     //printf ("In plotData: cnt: %d\n", cnt);
         
        
     
	//printf ("Starting drawing\n");
    pthread_mutex_lock (&mutex);

    cairo_t *context = cairo_create (surface);
    setGtkContext (context); //Set the graphics library context


    // Draw the background.

	//Plot color:
	 cairo_set_source_rgb (context, 0.5, 0.5, 0); //temp
	int plot_color = YELLOW;

	refreshGraphBackground();
	drawLegend (true);

	// Plot the current trace
	for (int i = 0; i < DISPLAY_LEN; i++)
	{
		plotSig(data[i],i,plot_color);
		
	}
	setLinePlotFlag(0); //Ensure we do not use last position of current plot to draw (lineplot)
	//cairo_stroke (context);
    cairo_destroy (context);
    cnt ++;
    usleep (1E6 / 1000);
    
    pthread_mutex_unlock (&mutex);
    draw_flg = false;
 //}


}// plotDsata


static void plotData1 ( void )
{

  static unsigned int cnt = 0;
  while ( 1 )
  {
     usleep (1E6 / GPH_REFRESH_RATE); // Sleep for required interval.
     //printf ("In plotData1: cnt: %d\n", cnt);
     if (surface == (cairo_surface_t *)NULL)
     {
      continue;
     }
  
     
     
    // Wait until current polling iteration is complete
     int wait_cnt = 0;
     while (draw_flg == false)
     {
	//Wait for data polling to complete
	wait_cnt ++;
	usleep (1E6 / 1000);
	//printf ("Waitcnt: %d\n",wait_cnt);
     }
    
        
     
	//printf ("Starting drawing\n");
    pthread_mutex_lock (&mutex);

    cairo_t *context = cairo_create (surface);
    setGtkContext (context); //Set the graphics library context


    // Draw the background.

	//Plot color:
	 cairo_set_source_rgb (context, 0.5, 0.5, 0); //temp
	int plot_color = YELLOW;

	refreshGraphBackground();
	drawLegend (true);

	// Plot the current trace
	for (int i = 0; i < DISPLAY_LEN; i++)
	{
		plotSig(data[i],i,plot_color);
		
	}
	setLinePlotFlag(0); //Ensure we do not use last position of current plot to draw (lineplot)
	//cairo_stroke (context);
    cairo_destroy (context);
    cnt ++;
    //printf ("end of drawing\n");
    usleep (1E6 / 1000);
#ifdef DIAGNOSTICS    
    if (cnt == 10)
    {
    	// Save image 
    	cairo_surface_write_to_png (surface,"signal_image.png");
    	while (1); //Infinite wait for diagnostics
    }
#endif    
    
    pthread_mutex_unlock (&mutex);
    draw_flg = false;
 }


}// plotDsata1




static void sys_delay()
{
	//Notes:
	// If the interval specified in struct timespec req is not an exact
	// multiple of the underlying clock, then the interval will be rounded
	// up to the next multiple.
	//TBC for more accuracy with fast sampling


	//Delay execution of program for desired time:
	int res = nanosleep(&req,&rem);
	if(res < 0)
	{
		printf("nanosleep() ERROR!\n");
		printf("Error number %d: %s\n",errno,strerror(errno));
		if(errno == EINVAL)
		{
			printf("Nanosecond parameter out of bounds (< 0 or > 999999999)\n");
		}
		if(errno == EINTR)
		{
			printf("Sleep interrupted by a signal handler\n");
			//USe this to schedule the remainin time
			//If we have defined &rem, this will contain the remaining time
			//needed to complete request by calling nanosleep(&rem,NULL);
		}

	}


}//sys_delay




static void * data_poll_thread (void *ptr)
{

	//Open file for data:
	if (saveToFile)
	{
		 if( (fp=fopen(filename,"wt")) == NULL)
		 {
			printf("Error opening file -aborting program!\n");
			exit(0);
		 }
	}


	if ( strcmp(DRIVER_NAME, "spidev0.0") == 0)
	{

		//Define transfer words required for using SPIDEV
		setSpiDevTxWords();
		unsigned int cnt = 0;
		
		while (EXIT_PROGRAM == 0)
		{
			
			while (draw_flg == true)
			{
				//Wait for plot routine to set flag to false
				usleep (1E6 / 1000);
			}

			//Get sample
			printf ("Iteration: %d \n", cnt);
			for(int i=0; i< DATA_LEN;i++)
			{
				//Delay for set time:
				sys_delay();
				data [i] = readSpiDev();

				//printf ("Data val: %f\n",data [i]);

			}

			// Plot the graph:


			if (saveToFile)
			{
				//Save data to file and exit program
				for (int i = 0; i < DATA_LEN; i++)
				{
					fprintf(fp, "%f \n", data[i]);

				}
				fclose(fp);

				//Close the devuice:
				MCP3008_Close();
				exit (0);
			}
			cnt ++;
			//Repaint the drawingwidget
			draw_flg = true;
			//gdk_threads_add_idle((GSourceFunc)invalidate_cb,(void*)DrawingArea);
			//gdk_threads_add_idle((GSourceFunc)gtk_widget_queue_draw,(void*)DrawingArea);


		}

	}
	else if ( strcmp(DRIVER_NAME, "mcp3008") == 0)

	{
		printf ("Running custom driver for mcp3008:\n");
		unsigned int cnt = 0;
		while (EXIT_PROGRAM == 0)
		{
			
			while (draw_flg == true)
			{
				//Wait for plot routine to set flag to false
				usleep (1E6 / 1000);
				//printf ("Waiting for redraw flad reset\n");
			}
			
			//Get sample
			printf ("Iteration: %d \n", cnt);
			for(int i=0; i< DATA_LEN;i++)
			{
				//Delay for set time:
				sys_delay();
				data [i] = readADC();
				//printf ("Cnt: %d Data: %f\n",i,data[i]);

			}
			if (saveToFile)
			{
				int cnt_index = 10;
				if (cnt == cnt_index)
				{
					printf ("Closing the file!\n");
					//Save data to file and exit program
					for (int i = 0; i < DISPLAY_LEN; i++)
					{
						fprintf(fp, "%f \n", data[i]);

					}
					fclose(fp);

					//Close the devuice:
					MCP3008_Close();
					exit (0);
				}
			}
			cnt ++;
			//Repaint the drawingwidget
			draw_flg = true;
			//gdk_threads_add_idle((GSourceFunc)invalidate_cb,(void*)DrawingArea);
			//gdk_threads_add_idle((GSourceFunc)gtk_widget_queue_draw,(void*)DrawingArea);	
			
			
		}


	}



} // sampleData




static float sine_to_point (int x, int width, int height)
{
  return (height) * sin (x * 2 * M_PI / (PERIOD));
}


static void drawLegend (int flg)
{
	if(flg)
	{
		l_caps.data1_caption = LEGEND_CAPTION1;
		l_caps.data2_caption = LEGEND_CAPTION2;
		l_caps.data3_caption = LEGEND_CAPTION3;
		l_caps.data4_caption = LEGEND_CAPTION4;
		
		l_data.data1_unit = "V";
		l_data.data2_unit = "mV";
		l_data.data3_unit = "s";
		l_data.data4_unit = "Hz";
		
		 
		legendCaptions(l_caps,getGdkColor(WHITE),LEGEND_NUM_BOXES,left,bottom);
		
		// For testing:
		l_data.data1 = nan ("NaN"); //1.6;
		l_data.data2 = nan ("NaN");//8.4;
		l_data.data3 = nan ("NaN");//25.0;
		l_data.data4 = nan ("NaN");//9.52;
		
		legend(l_data,false, LEGEND_NUM_BOXES, YELLOW, left, bottom);
	}
		
	
} // drawLegend	



void init_graph(int argc, char * argv[])
{

			
		bck_color = getGdkColor (BCK_COLOR);
		ori_line_color = getGdkColor (ORI_LINE_COLOR);
		
		
	
	//Initialise Graphics
	Graphic_Init(argc, argv, X_WIN_SIZE,Y_WIN_SIZE,PLOT_MODE,LOG_MODE, X_LABEL, Y_LABEL, WINDOW_TITLE,GRAPH_TITLE, GPH_REFRESH_RATE,
					Y_MIN,Y_MAX,X_MIN, X_MAX,
					X_INT,Y_INT,X_GRAD,Y_GRAD,X_LABEL_PREC,Y_LABEL_PREC,
					SHOW_GRID, G_INT,
					X_SHOW,Y_SHOW,ORI_LINE,ori_line_color,BITMAP,
					bck_color, AUDIOFILE, INI_AUDIOFILE,
					NUM_CHANS, AUDIO_SAMPLE_RATE, BITS_PER_SAMPLE);
			
	
					
	//Define vetical marker color here: //TBD - define RGBAs for each Turbo color
	ver_marker_color.red = 	1.0;
	ver_marker_color.green = 0.0;
	ver_marker_color.blue = 0.0;
	ver_marker_color.alpha = 0.0;
	if (DRAW_VER_MARKER)
	{
		drawVerticalMarker (V_MARKER_POS,ver_marker_color,MARKER_WIDTH);	
	}				
		
	// Initialise plot line width:
	lineWidth (PLOT1_LINE_WIDTH);
	//Setup Legend:
	//drawLegend (true);
	
		
	
	struct legend_info info[4];
	info[0].type = SQUARE_BORDER;
	info[0].color = BLACK;
	info[0].text = "Data1";
	info[1].type = SQUARE_FILLED;
	info[1].color = BLACK;
	info[1].text = "Data2";
	info[2].type = CIRCLE_BORDER;
	info[2].color = BLACK;
	info[2].text = "Data3";
	
	info[3].type = TRIANGLE_FILLED;
	info[3].color = BLUE;
	info[3].text = "Data4";
	
	 
	//legendCaptions1(GdkRGBA col,DARKGRAY,4,struct legend_info info[], 80, 80,100,100);
	//legendCaptions1(getGdkColor(WHITE),DARKGRAY,4, info, 50, 50,80,45);
	
		
	
	//-----------------------------------------------------------------------------------------------
   // Initialise GTK elements:   
    //----------------------------------------------------------------------------------------------
    gtk_init(&argc, &argv);
	printf ("Args:  %s\n",argv[0]);
	surface = NULL;
		
	GtkWidget *main_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (main_window), WINDOW_TITLE);
  gtk_window_set_default_size (GTK_WINDOW (main_window), 800, 400);
  GtkWidget *drawing_area = gtk_drawing_area_new ();
  DrawingArea = drawing_area;
  // Connect to the configure event to create the surface. 
   
  g_signal_connect (drawing_area, "configure-event", G_CALLBACK (drawing_area_configure_cb), NULL);
  //g_signal_connect((GtkWidget*)drawing_area, "configure-event", G_CALLBACK(configure_canvas), NULL);

  gtk_container_add (GTK_CONTAINER (main_window), drawing_area);
  gtk_widget_show_all (main_window);

  // Create a new thread to update the stored surface. 
  pthread_mutex_init (&mutex, NULL);
  //pthread_create (&drawing_thread, NULL, thread_draw, NULL);
  //if (SAMPLING_TYPE == 0)
  // {	
	
      int rc = pthread_create (&drawing_thread, NULL, draw_thread, NULL);
	
   // }
  
  
  // Create a  timer to invalidate our window at 60Hz, and display the stored surface. 
  
  //if (SAMPLING_TYPE == 0)
  //{
      g_timeout_add (1000 / GPH_REFRESH_RATE, invalidate_cb, drawing_area);
  //}	
  //gdk_threads_add_idle((GSourceFunc)gtk_widget_queue_draw,(void*)DrawingArea);
  

  // Connect our redraw callback. 
  //g_signal_connect (drawing_area, "draw", G_CALLBACK (redraw_canvas), NULL); //this cuasing the jumpiness
   g_signal_connect (drawing_area, "draw", G_CALLBACK (drawing_area_draw_cb), NULL);
  // Connect the destroy signal. 
  g_signal_connect (main_window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

  
	// Start / show the GUI
	gtk_main();
    
	
	
}//init_graph




// ------------------------------------------------------------

int main (int argc, char * argv[]) 
{
	
	if (SAMPLING_TYPE == 1)
	{	
		//Initialise graph:
		for (int i = 0; i < DATA_LEN; i++)
		{
			data[i] = 0.0;
		}


		//Register handler for capturing CTL-C signal:
		signal(SIGINT,ctrl_c_handler);



		//Initialise SPI for MCP3008:
		printf("Initialising SPI for MCP3008 peripheral.....\n");

		
		int res = MCP3008_Init(SAMPLE_RATE);
		if(res < 0)
		{
			printf("Problem initialising device - exiting program!\n");
			exit(0);
		}
		

		//set parameters (if not default)
		//setParams(readMode,chanIndex);
		printf("Sample rate: %f\n",SAMPLE_RATE);


		//sample_delay = getSamplingRate();
		sample_delay = (1.0/(float)SAMPLE_RATE)*1000000;
		
		printf("Delay in micro-secs: %lld\n",sample_delay);


		if(sample_delay < 1000000)
		{
			req.tv_sec = 0;
		   req.tv_nsec = sample_delay*1000;
		}
		else
		{
			req.tv_sec = sample_delay/1000000;
		   req.tv_nsec = 0;

		}


		pthread_create (&poll_thread, NULL, data_poll_thread, NULL);
	}
	
	
	
	// Initialise graph and commence data sampling
	init_graph (argc, argv);
	
	
	exit(0);

		
	return 0;
}


//NB: This handler is working for the CTL-C keyboard signal
//	  This will exit the while loop sple (us):  1157
//   to exit the program gracefully
//	  (We need to close the connection to the serial terminal)
void ctrl_c_handler(int sig)
{
   if(sig == SIGINT) //Check it is the right user ID //NB: Not working at the moment
   {
         printf("\nWe have received the CTRL-C signal - aborting sample looping!\n");

         
         // Close device
         MCP3008_Close();


         //EXIT_PROGRAM = 1; //This will stop program
         exit(0);
   }


}//ctrl_c_handler(int sig)


