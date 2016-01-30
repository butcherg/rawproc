#ifndef __SATURATION_H__
#define __SATURATION_H__

#include <math.h>

#define  Pr  .299
#define  Pg  .587
#define  Pb  .114

//  public-domain function by Darel Rex Finley
 //
 //  The passed-in RGB values can be on any desired scale, such as 0 to
 //  to 1, or 0 to 255.  (But use the same scale for all three!)
 //
 //  The "change" parameter works like this:
 //    0.0 creates a black-and-white image.
 //    0.5 reduces the color saturation by half.
 //    1.0 causes no change.
 //    2.0 doubles the color saturation.
 //  Note:  A "change" value greater than 1.0 may project your RGB values
 //  beyond their normal range, in which case you probably should truncate
 //  them to the desired range before trying to use them in an image.

 void changeSaturation(double *R, double *G, double *B, double change) 
{
	double  P=sqrt(
	(*R)*(*R)*Pr+
	(*G)*(*G)*Pg+
	(*B)*(*B)*Pb ) ;

	*R=P+((*R)-P)*change;
	*G=P+((*G)-P)*change;
	*B=P+((*B)-P)*change; 
/*
	if (*R>255.0) *R=255.0;
	if (*G>255.0) *G=255.0;
	if (*B>255.0) *B=255.0;
	if (*R<0.0) *R=0.0;
	if (*G<0.0) *G=0.0;
	if (*B<0.0) *B=0.0;
*/
} 

#endif

