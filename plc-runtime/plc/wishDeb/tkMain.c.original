/* main.c */
#include <tcl.h>
#include <tk.h>

EXTERN int              Tktable_Init _ANSI_ARGS_((Tcl_Interp *interp));

void RegistraComandi(Tcl_Interp *interp);

/* 
 * Tcl_AppInit is called from Tcl_main
 * after the Tcl interpreter has been created,
 * and before the script file
 * or interactive command loop is entered.
 */

int Tcl_AppInit (Tcl_Interp *interp) 
  {
  /* 
   * Inizialize packages
   * Tcl_Init sets up the Tcl library facility.
   */
	 
#ifdef Linux
  if (Tcl_Init(interp) == TCL_ERROR) 
    {
    return TCL_ERROR;
    }

  if (Tk_Init(interp) == TCL_ERROR) 
    {
    return TCL_ERROR;
    }
#else
  if (Tcl_InitStandAlone(interp) == TCL_ERROR) 
    {
    return TCL_ERROR;
    }

  if (Tk_InitStandAlone(interp) == TCL_ERROR) 
    {
    return TCL_ERROR;
    }
#endif

  Tcl_StaticPackage(interp, "Tk", Tk_Init, Tk_SafeInit);

  if ( Tixsam_Init(interp) == TCL_ERROR)
    {
    return TCL_ERROR;
    } 

  if (Tktable_Init(interp) == TCL_ERROR) {
      return TCL_ERROR;
  }
  Tcl_StaticPackage(interp, "Tktable", Tktable_Init,
                      (Tcl_PackageInitProc *) NULL);


  /*if ( Tix_Init(interp) == TCL_ERROR)
    {
    return TCL_ERROR;
    } 

  if ( Img_Init(interp) == TCL_ERROR)
    {
    return TCL_ERROR;
    } */

  /* 
   * Register application-specific commands.
   */
   RegistraComandi(interp);

  return TCL_OK;
  }

void main (int argc ,char *argv[]) 
  {
  Tk_Main (argc, argv, Tcl_AppInit);
  exit (0);
  }
