SRTM HGT reader
===============

C implementation for reading interpolated height values from SRTM HGT files.

Download srtm HGT files ie from http://dds.cr.usgs.gov/srtm/version2_1/SRTM3/ and put them in srtm/ folder.

Inspired by PHP SRTM Geotiff parser http://www.osola.org.uk/elevations/


More information on data: http://dds.cr.usgs.gov/srtm/version1/Documentation/SRTM_Topo.txt


ASTER GDEM data
---------------

Its also possible to supply ASTER GDEM elevation model from http://asterweb.jpl.nasa.gov/gdem.asp (you have to register yourself to obtain the files, be careful with the license)

ASTER GDEM uses Geotiff files, data cell for every arc second = 3601 lines and cols.
To convert file in HGT use (apt-get install gdal-bin):
    $ gdal_translate -of SRTMHGT ASTGTM2_N50E014_dem.tif N50E014.HGT

Then uncomment the constants at the beggining of srtmHgtReader.c
