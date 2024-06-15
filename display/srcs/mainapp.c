#include <gtk/gtk.h>
#include <glib/gprintf.h>
#include <ep-common.h>
#include <picking.h>

#include <gtk-support.h>

#include "main.h"
#include "mainstruct.h"
#include "mainext.h"
#include "mainapp.h"
#include "mainwin.h"


struct _MainApp
{
    GtkApplication parent;
};

G_DEFINE_TYPE(MainApp, main_app, GTK_TYPE_APPLICATION);


static void main_app_init (MainApp *app)
{
    g_print("main_app_init\n");
}

static void main_app_startup (GApplication *app)
{
    g_print("main_app_startup\n");
    G_APPLICATION_CLASS (main_app_parent_class)->startup (app);
}

static void main_app_activate (GApplication *app)
{
    MainWindow *win;

    g_print("main_app_activate\n");
	
	signal(SIGSEGV, SIG_IGN);    win = main_window_new (MAIN_APP (app), settore, maximized, fullscreen);
    gtk_window_present (GTK_WINDOW (win));
}

static void main_app_class_init (MainAppClass *class)
{
    g_print("main_app_class_init\n");
    G_APPLICATION_CLASS (class)->startup = main_app_startup;
    G_APPLICATION_CLASS (class)->activate = main_app_activate;
}

MainApp *main_app_new (void)
{
    g_print("main_app_new\n");
    char szBuffer[256];
    sprintf(szBuffer, "org.%s.%s", APP_CLASS, APP_NAME);

	printf(" cLBLUE    = { %.1f, %.1f, %.1f, %.1f } \n",    cLBLUE.red,        cLBLUE.green,   cLBLUE.blue,   cLBLUE.red );   
	printf(" cBLUE     = { %.1f, %.1f, %.1f, %.1f } \n",    cBLUE.red,         cBLUE.green,    cBLUE.blue,    cBLUE.red );    
	printf(" cBLACK    = { %.1f, %.1f, %.1f, %.1f } \n",    cBLACK.red,        cBLACK.green,   cBLACK.blue,   cBLACK.red );   
	printf(" cRED      = { %.1f, %.1f, %.1f, %.1f } \n",    cRED.red,          cRED.green,     cRED.blue,     cRED.red );     
	printf(" cLRED     = { %.1f, %.1f, %.1f, %.1f } \n",    cLRED.red,         cLRED.green,    cLRED.blue,    cLRED.red );    
	printf(" cLCYAN    = { %.1f, %.1f, %.1f, %.1f } \n",    cLCYAN.red,        cLCYAN.green,   cLCYAN.blue,   cLCYAN.red );   
	printf(" cLICE     = { %.1f, %.1f, %.1f, %.1f } \n",    cLICE.red,         cLICE.green,    cLICE.blue,    cLICE.red );    
	printf(" cCYAN     = { %.1f, %.1f, %.1f, %.1f } \n",    cCYAN.red,         cCYAN.green,    cCYAN.blue,    cCYAN.red );    
	printf(" cWHITE    = { %.1f, %.1f, %.1f, %.1f } \n",    cWHITE.red,        cWHITE.green,   cWHITE.blue,   cWHITE.red );   
	printf(" cYELLOW   = { %.1f, %.1f, %.1f, %.1f } \n",    cYELLOW.red,       cYELLOW.green,  cYELLOW.blue,  cYELLOW.red );  
	printf(" cLYELLOW  = { %.1f, %.1f, %.1f, %.1f } \n",    cLYELLOW.red,      cLYELLOW.green, cLYELLOW.blue, cLYELLOW.red ); 
	printf(" cLGREEN   = { %.1f, %.1f, %.1f, %.1f } \n",    cLGREEN.red,       cLGREEN.green,  cLGREEN.blue,  cLGREEN.red );  
	printf(" cGREEN    = { %.1f, %.1f, %.1f, %.1f } \n",    cGREEN.red,        cGREEN.green,   cGREEN.blue,   cGREEN.red );   
	printf(" cGREYCYAN = { %.1f, %.1f, %.1f, %.1f } \n",    cGREYCYAN.red,     cGREYCYAN.green,cGREYCYAN.blue,cGREYCYAN.red );
	printf(" cLGREY    = { %.1f, %.1f, %.1f, %.1f } \n",    cLGREY.red,        cLGREY.green,   cLGREY.blue,   cLGREY.red );   
	printf(" cGREY     = { %.1f, %.1f, %.1f, %.1f } \n",    cGREY.red,         cGREY.green,    cGREY.blue,    cGREY.red );    
	printf(" cSLGREY   = { %.1f, %.1f, %.1f, %.1f } \n",    cSLGREY.red,       cSLGREY.green,  cSLGREY.blue,  cSLGREY.red );  
	printf(" cSGREY    = { %.1f, %.1f, %.1f, %.1f } \n",    cSGREY.red,        cSGREY.green,   cSGREY.blue,   cSGREY.red );   
	printf(" cORANGE   = { %.1f, %.1f, %.1f, %.1f } \n",    cORANGE.red,       cORANGE.green,  cORANGE.blue,  cORANGE.red );  


    return g_object_new (MAIN_APP_TYPE, "application-id", szBuffer, "flags", ep_get_glib_default_flags(), NULL);
}
