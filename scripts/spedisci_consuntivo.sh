#!/bin/bash
/usr/local/bin/move_export_file_to_send.sh --filename conscni.txt --exportdir /u/prj/mondadori/export --targetdir /u/prj/mondadori/export/ftp-mondadori-export >/dev/null 2>&1
/usr/local/bin/ftp_send.sh --ftpuser mpf-boxline --ftppassword eonrc44 --ftphost ftp.mondadori.it --remotedir input --backupdir /u/prj/mondadori/export/ftp-mondadori-export-backup  --filename /u/prj/mondadori/export/ftp-mondadori-export/conscni* >/dev/null 2>&1
exit 0
