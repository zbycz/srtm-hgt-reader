/* 
 * File:   srtmHgtReader.cpp
 * Author: Pavel Zbytovský <pavel@zby.cz>
 *
 * Created on April 28, 2013, 12:01 AM
 */
//#define SRTMSLIM 1

#include <stdio.h> 
#include <stdlib.h> //exit
#include <stdint.h> //int16_t
#include <math.h>

#include "srtmHgtReader.h" //fmod



//const int secondsPerPx = 1;  //arc seconds per pixel (3 equals cca 90m)
//const int totalPx = 3601;
//const char* folder = "aster";

const int secondsPerPx = 3;  //arc seconds per pixel (3 equals cca 90m)
const int totalPx = 1201;
const char* folder = "srtm";

FILE* srtmFd = NULL;
int srtmLat = 255; //default never valid
int srtmLon = 255;
unsigned char * srtmTile = NULL;

/** Prepares corresponding file if not opened */
void srtmLoadTile(int latDec, int lonDec){
    if(srtmLat != latDec || srtmLon != lonDec) {
        srtmLat = latDec;
        srtmLon = lonDec;
        
        if(srtmFd != NULL){
            fclose(srtmFd);
        }
        
        char filename[20];
        sprintf(filename, "%s/N%dE0%d.hgt", folder, latDec, lonDec);
        printf("Opening %s\n", filename);
        
        srtmFd = fopen(filename, "r");
        
        if(srtmFd == NULL) {
            printf("Error opening %s\n",  filename);
            exit(1);
        }
        
#if !SRTMSLIM
        if(srtmTile == NULL){
            srtmTile = (unsigned char*) malloc(totalPx * totalPx * 2); //allocate only once
        }
        
        //read the whole tile
        fread(srtmTile, 1, (2 * totalPx * totalPx), srtmFd);
#endif
    }
}

void srtmClose(){
    if(srtmFd != NULL){
        fclose(srtmFd);
    }
    
#if !SRTMSLIM
    if(srtmTile != NULL){
        free(srtmTile);
    }
#endif
}

/** Pixel idx from left bottom corner (0-1200) */
void srtmReadPx(int y, int x, int* height){
    int row = (totalPx-1) - y;
    int col = x;
    int pos = (row * totalPx + col) * 2;
    
#if SRTMSLIM
    
    //seek and read 2 bytes short
    unsigned char buff[2];// = {0xFF, 0xFB}; //-5 (bigendian)
    fseek(srtmFd, pos, SEEK_SET);
    fread(&buff, 2, 1, srtmFd);
    
#else
    
    //set correct buff pointer
    unsigned char * buff = & srtmTile[pos];
    
#endif
    
    //solve endianity (using int16_t)
    int16_t hgt = 0 | (buff[0] << 8) | (buff[1] << 0);
    
    if(hgt == -32768) {
        printf("ERROR: Void pixel found on xy(%d,%d) in latlon(%d,%d) tile.\n", x,y, srtmLat, srtmLon);
        exit(1);
    }
    
    *height = (int) hgt;
}       

/** Returns interpolated height from four nearest points */
float srtmGetElevation(float lat, float lon){
    int latDec = (int)lat;
    int lonDec = (int)lon;

    float secondsLat = (lat-latDec) * 60 * 60;
    float secondsLon = (lon-lonDec) * 60 * 60;
    
    srtmLoadTile(latDec, lonDec);

    //X coresponds to x/y values,
    //everything easter/norhter (< S) is rounded to X.
    //
    //  y   ^
    //  3   |       |   S
    //      +-------+-------
    //  0   |   X   |
    //      +-------+-------->
    // (sec)    0        3   x  (lon)
    
    //both values are 0-1199 (1200 reserved for interpolating)
    int y = secondsLat/secondsPerPx;
    int x = secondsLon/secondsPerPx;
    
    //get norther and easter points
    int height[4];
    srtmReadPx(y,   x, &height[2]);
    srtmReadPx(y+1, x, &height[0]);
    srtmReadPx(y,   x+1, &height[3]);
    srtmReadPx(y+1, x+1, &height[1]);

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
    return  height[0] * dy * (1 - dx) +
            height[1] * dy * (dx) +
            height[2] * (1 - dy) * (1 - dx) +
            height[3] * (1 - dy) * dx;
}


/** Returns amount of ascent and descent between points */
TSrtmAscentDescent srtmGetAscentDescent(float lat1, float lon1, float lat2, float lon2, float dist){
    TSrtmAscentDescent ret = {0};
    
    //segment we need to devide in "pixels"
    double latDiff = lat2 - lat1;
    double lonDiff = lon2 - lon1;
    
    //how many pixels there are both in y and x axis
    double latSteps = latDiff * (3600 / 3); // 1/pixelDistance = cca 0.00083
    double lonSteps = lonDiff * (3600 / 3);
    
    //we use the max of both
    int steps = fmax(fabs(latSteps), fabs(lonSteps));

    //just in case both points are inside one pixel (we need interpolation!)
    if(steps == 0) steps = 1;
    
    
    //set the delta of each step
    double latStep = latDiff / steps;
    double lonStep = lonDiff / steps;
    double distStep = dist/steps;
      //printf("steps %d: %f %f %f\n", steps, latStep, lonStep, distStep);
    
    int i;
    double lat = lat1, lon = lon1;
    float height, lastHeight, eleDiff;

    //get first elevation -> we need eleDiff then
    height = srtmGetElevation(lat, lon);
      //printf("first: %f %f hgt:%f\n", lat, lon, height);
    
    for(i=0; i<steps; ++i){
        lat += latStep;
        lon += lonStep;
        lastHeight = height;
        
        height = srtmGetElevation(lat, lon);
        eleDiff = height - lastHeight;
        
        if(eleDiff > 0){
            ret.ascent += eleDiff;
            ret.ascentOn += distStep;
        }
        else{
            ret.descent += -eleDiff;
            ret.descentOn += distStep;
        }
        
        //printf("LL(%d): %f %f hgt: %0.1f, diff %0.1f\n", i, lat, lon, height, eleDiff);
    }
    
    // printf("last: %f %f\n", i, lat, lon); ==   printf("ll2: %f %f\n", i, lat2, lon2);
    
    return ret;
}
