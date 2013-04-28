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

#endif	/* SRTMHGTREADER_H */

