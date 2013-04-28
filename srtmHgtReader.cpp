/* 
 * File:   srtmHgtReader.cpp
 * Author: Pavel Zbytovský <pavel@zby.cz>
 *
 * Created on April 28, 2013, 12:01 AM
 */

#include <stdio.h> 
#include <stdlib.h> //exit
#include <math.h> //fmod



FILE* srtmFd = NULL;
int srtmLat = 255; //default never valid
int srtmLon = 255;
const int secondsPerPx = 3; //arc seconds per pixel (equals cca 90m)
const int totalPx = 1201;

/** Prepares corresponding file if not opened */
void srtmLoadTile(int latDec, int lonDec){
    if(srtmLat != latDec || srtmLon != lonDec) {
        srtmLat = latDec;
        srtmLon = lonDec;
        
        if(srtmFd != NULL){
            fclose(srtmFd);
        }
        
        char filename[20];
        sprintf(filename, "srtm/N%dE0%d.hgt", latDec, lonDec);
        printf("Opening %s\n", filename);
        
        srtmFd = fopen(filename, "r");
        
        if(srtmFd == NULL) {
            printf("Error opening %s\n",  filename);
            exit(1);
        }
    }
}

/** Pixel idx from left bottom corner (0-1200) */
int srtmReadPx(int y, int x){
    int row = (totalPx-1) - y;
    int col = x;
    int pos = (row * totalPx + col) * 2;
    
    //seek and read 2 bytes short
    unsigned char buff[2];// = {0xFF, 0xFB}; //-5 (bigendian)
    fseek(srtmFd, pos, SEEK_SET);
    fread(&buff, 2, 1, srtmFd);
    
    //solve endianity (using int16_t)
    int16_t hgt = 0 | (buff[0] << 8) | (buff[1] << 0);
    
    if(hgt == -32768) {
        printf("ERROR: Void area found on xy(%d,%d) in latlon(%d,%d) tile.\n", x,y, srtmLat, srtmLon);
        exit(1);
    }
    
    return hgt;
}

/** Returns interpolated height from four nearest points */
float srtmGetElevation(float lat, float lon){
    int latDec = (int)lat;
    int lonDec = (int)lon;
    srtmLoadTile(latDec, lonDec);

    float secondsLat = (lat-latDec) * 60 * 60;
    float secondsLon = (lon-lonDec) * 60 * 60;
    
    //X coresponds to x/y values,
    //everything easter/norhter is rounded to X.
    //
    //  y   ^
    //  3   |       |
    //      +-------+-------
    //  0   |   X   |
    //      +-------+-------->
    // (sec)    0        3   x  (lon)
    
    //both values are 0-1199 (1200 reserved for interpolating)
    int y = secondsLat/secondsPerPx;
    int x = secondsLon/secondsPerPx;
    
    //get norther and easter points
    int height[4];
    height[2] = srtmReadPx(y, x);
    height[0] = srtmReadPx(y+1, x);
    height[3] = srtmReadPx(y, x+1);
    height[1] = srtmReadPx(y+1, x+1);

    //ratio where X lays
    float dy = fmod(secondsLat, secondsPerPx) / secondsPerPx;
    float dx = fmod(secondsLon, secondsPerPx) / secondsPerPx;
    
    // Bilinear interpolation
    // h0------------h1
    // |
    // |--dx-- .
    // |       |
    // |      dy
    // |       |
    // h2------------h3   
    float hgt = 
            height[0] * dy * (1 - dx) +
            height[1] * dy * (dx) +
            height[2] * (1 - dy) * (1 - dx) +
            height[3] * (1 - dy) * dx;

    return hgt;
}
      


void test();


/**
 * Example Usage
 * Download srtm HGT files ie from http://dds.cr.usgs.gov/srtm/version2_1/SRTM3/
 */
int main() {
    
    float h1 = srtmGetElevation(50, 14);
    float h2 = srtmGetElevation(50.10083, 14.30987);
    printf("height %f\n", h1);
    printf("height %f\n", h2);
    
    test();
    
    return 0;
}


