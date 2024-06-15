#ifdef WIN32
    Tcl_CreateCommand (interp, "ProsAvviaProtezione", ProsAvviaProtezioneCmd, 
       (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateCommand (interp, "ProsVerificaProtezione", ProsVerificaProtezioneCmd, 
       (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateCommand (interp, "ProsJollyStringa", ProsJollyStringaCmd, 
       (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateCommand (interp, "ProsChiudiProtezione", ProsChiudiProtezioneCmd, 
       (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
#endif

