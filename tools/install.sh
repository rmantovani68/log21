mkdir -p /opt/easy-picking/exe
mkdir -p /opt/easy-picking/data
mkdir -p /opt/easy-picking/export
mkdir -p /opt/easy-picking/trace
mkdir -p /opt/easy-picking/stampe
mkdir -p /opt/easy-picking/images
mkdir -p /usr/share/easy-picking/images


cp ../data/*.cfg /opt/easy-picking/data
cp ../data/*.*xml /opt/easy-picking/data
cp ../images/* /opt/easy-picking/images

cp ../exe/auto_tasks     /opt/easy-picking/exe
cp ../exe/balance        /opt/easy-picking/exe
cp ../exe/bancali        /opt/easy-picking/exe
cp ../exe/barcode        /opt/easy-picking/exe
cp ../exe/bilancia       /opt/easy-picking/exe
cp ../exe/carichi        /opt/easy-picking/exe
cp ../exe/check_bancali  /opt/easy-picking/exe
cp ../exe/display        /opt/easy-picking/exe
cp ../exe/edit           /opt/easy-picking/exe
cp ../exe/ev2002         /opt/easy-picking/exe
cp ../exe/ios            /opt/easy-picking/exe
cp ../exe/main           /opt/easy-picking/exe
cp ../exe/monitor        /opt/easy-picking/exe
cp ../exe/peso           /opt/easy-picking/exe
cp ../exe/printlabel     /opt/easy-picking/exe
cp ../exe/printman       /opt/easy-picking/exe
cp ../exe/readshm        /opt/easy-picking/exe
cp ../exe/receive        /opt/easy-picking/exe
cp ../exe/settori        /opt/easy-picking/exe
cp ../exe/setup          /opt/easy-picking/exe
cp ../exe/test           /opt/easy-picking/exe
cp ../exe/volum          /opt/easy-picking/exe

cp ../exe/gschemas.compiled /opt/easy-picking/exe
cp ../exe/ael.cfg /opt/easy-picking/exe

cp desktop/*.desktop /usr/share/applications
cp desktop/*.directory /usr/share/desktop-directories

cp ../images/easy-picking-main.svg     /usr/share/easy-picking/images
cp ../images/easy-picking-peso.svg     /usr/share/easy-picking/images
cp ../images/easy-picking-monitor.svg  /usr/share/easy-picking/images
cp ../images/easy-picking-balance.svg  /usr/share/easy-picking/images
cp ../images/easy-picking-setup.svg    /usr/share/easy-picking/images
cp ../images/easy-picking-test.svg     /usr/share/easy-picking/images
cp ../images/easy-picking-carichi.svg  /usr/share/easy-picking/images
cp ../images/easy-picking-bancali.svg  /usr/share/easy-picking/images
cp ../images/easy-picking-check-bancali.svg  /usr/share/easy-picking/images
chmod a+rw /opt/easy-picking/images/*
