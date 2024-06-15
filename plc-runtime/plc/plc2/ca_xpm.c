/*
 *	****************************************************************************
 *
 *	File CA_XPM.C
 *
 *	Libreria di gestione file grafici di tipo XPM per XNC.
 *
 *	Mercoledi' 06 dicembre 1995 - CNi s.r.l.
 *
 *	****************************************************************************
 */


#ifdef Linux
#include <stdlib.h>
#include <stdio.h>
#else
#include "ca_all.h"
#include <xpm.h>
#endif


#ifdef Linux
#include <X11/StringDefs.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/xpm.h>
#else
#include <libxerr.h>
#include <tipierr.h>

#include "CniAlf.h"
#endif


#ifdef EXCEED
int		xcdDoPixmap=1;
#endif


/*
 *	***************************************************************************
 *
 *	Funzione xncReadXpmData
 *
 *	Legge una struttura dati di tipo XPM.
 *
 *	Widget	hWidget		-	widget dove mettere il pixmap
 *	char	**xpmData	-	struttura dati
 *	int		*pnWidth	-	puntatore al deposito della larghezza
 *	int		*pnHeight	-	puntatore al deposito dell'altezza
 *
 *	Ritorna:
 *		il pixmap creato o NULL se errore
 *
 *	***************************************************************************
 */
Pixmap xncReadXpmData(
	Widget	hWidget,
	char	**xpmData,
	int		*pnWidth,
	int		*pnHeight
)
{
	Window					hRootWindow;
	Pixmap					hPixmap;
	Display					*hDisplay=XtDisplay(hWidget);
	XpmAttributes			xpm_attr;
	static XpmColorSymbol	none_color={ NULL, "None", (Pixel)0 };


	hRootWindow=DefaultRootWindow(hDisplay);

#ifdef EXCEED
	memset(&xpm_attr, 0, sizeof(XpmAttributes));
	xpm_attr.valuemask = XpmColorSymbols|XpmSize;
	xpm_attr.colorsymbols = &none_color;
	xpm_attr.numsymbols = 1;
#else
	xpm_attr.valuemask = XpmReturnPixels|XpmColorSymbols|XpmSize|XpmCloseness;
	xpm_attr.colorsymbols = &none_color;
	xpm_attr.numsymbols = 1;
	xpm_attr.closeness = 0xffff;
#endif

	XtVaGetValues(
		hWidget,
		XtNbackground, (XtArgVal)&none_color.pixel,
		NULL
	);
	
	if(
#ifdef EXCEED
		xcdDoPixmap &&
#endif
		XpmCreatePixmapFromData(
			hDisplay,
			hRootWindow,
			xpmData, 
			&hPixmap,
			NULL,
			&xpm_attr
		)==XpmSuccess
	)
	{
		if( pnWidth!=(int *)NULL )
			*pnWidth=xpm_attr.width;
		if( pnHeight!=(int *)NULL )
			*pnHeight=xpm_attr.height;

		return(hPixmap);
	}

#ifdef EXCEED
	/*
	 *	Pixmap trouble
	 */

	/*
	 *	Creo un Pixmap 20x20
	 */
	if( xcdDoPixmap==0 )
	{
		hPixmap=XCreatePixmap(
			hDisplay,
			hRootWindow,
			20,
			20,
			DefaultDepthOfScreen(XtScreen(hWidget))
		);
		if( hPixmap!=(Pixmap)NULL )
		{
			GC	gc;
 
			gc=XCreateGC(hDisplay, hRootWindow, (unsigned long)0, NULL);
			XFillRectangle(hDisplay, hPixmap, gc, 0, 0, 20, 20);

			XFreeGC(hDisplay, gc);
		}

		pnWidth ? *pnWidth = 20 : False;
		pnHeight ? *pnHeight = 20 : False;

		return(hPixmap);
	}
#endif

	return((Pixmap)NULL);
}	/*	Fine xncReadXpmData */


/*
 *	***************************************************************************
 *
 *	Funzione xncReadXpmFile
 *
 *	Legge un file di tipo XPM.
 *
 *	Widget	hWidget		-	widget dove mettere il pixmap
 *	char	*pszFileName-	file da leggere
 *	int		*pnWidth	-	puntatore al deposito della larghezza
 *	int		*pnHeight	-	puntatore al deposito dell'altezza
 *
 *	Ritorna:
 *		il pixmap creato o NULL se errore
 *
 *	***************************************************************************
 */
Pixmap xncReadXpmFile(
	Widget	hWidget,
	char	*pszFileName,
	int		*pnWidth,
	int		*pnHeight
)
{
	Window					hRootWindow;
	Pixmap					hPixmap;
	Display					*hDisplay=XtDisplay(hWidget);
	XpmAttributes			xpm_attr;
	static XpmColorSymbol	none_color={ NULL, "None", (Pixel)0 };


	hRootWindow=DefaultRootWindow(hDisplay);

#ifdef EXCEED
	memset(&xpm_attr, 0, sizeof(XpmAttributes));
	xpm_attr.valuemask = XpmColorSymbols|XpmSize;
	xpm_attr.colorsymbols = &none_color;
	xpm_attr.numsymbols = 1;
#else
	xpm_attr.valuemask = XpmReturnPixels|XpmColorSymbols|XpmSize|XpmCloseness;
	xpm_attr.colorsymbols = &none_color;
	xpm_attr.numsymbols = 1;
	xpm_attr.closeness = 0xffff;
#endif

	XtVaGetValues(
		hWidget,
		XtNbackground, (XtArgVal)&none_color.pixel,
		NULL
	);
	
	if(
#ifdef EXCEED
		xcdDoPixmap &&
#endif
		XpmReadFileToPixmap(
			hDisplay,
			hRootWindow,
			pszFileName,
			&hPixmap,
			NULL,
			&xpm_attr
		)==XpmSuccess
	)
	{
		if( pnWidth!=(int *)NULL )
			*pnWidth=xpm_attr.width;
		if( pnHeight!=(int *)NULL )
			*pnHeight=xpm_attr.height;

		return(hPixmap);
	}

#ifdef EXCEED
	/*
	 *	Pixmap trouble
	 */

	/*
	 *	Creo un Pixmap 20x20
	 */
	if( xcdDoPixmap==0 )
	{
		hPixmap=XCreatePixmap(
			hDisplay,
			hRootWindow,
			20,
			20,
			DefaultDepthOfScreen(XtScreen(hWidget))
		);
		if( hPixmap!=(Pixmap)NULL )
		{
			GC	gc;

			gc=XCreateGC(hDisplay, hRootWindow, (unsigned long)0, NULL);
			XFillRectangle(hDisplay, hPixmap, gc, 0, 0, 20, 20);

			XFreeGC(hDisplay, gc);
		}

		pnWidth ? *pnWidth = 20 : False;
		pnHeight ? *pnHeight = 20 : False;

		return(hPixmap);
	}
#endif

	return((Pixmap)NULL);
}	/*	Fine xncReadXpmFile */


/*	Fine CA_XPM.C */
