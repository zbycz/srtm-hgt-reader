/* 
 * File:   srtmHgtReader.h
 * Author: Pavel Zbytovský <pavel@zby.cz>
 *
 * Created on April 28, 2013, 6:44 PM
 */

#ifndef SRTMHGTREADER_H
#define	SRTMHGTREADER_H

float srtmGetElevation(float lat, float lon);

void srtmClose();



struct _SrtmAscentDescent {
    float ascent;
    float descent;
};

typedef struct _SrtmAscentDescent TSrtmAscentDescent;


TSrtmAscentDescent srtmGetAscentDescent(float lat1, float lon1, float lat2, float lon2);


#endif	/* SRTMHGTREADER_H */

