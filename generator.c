
#include <stdio.h> 
#include <stdlib.h>
#include <math.h>
#include "srtmHgtReader.h"


FILE* out;
void output(int x, int y){
        float lon = 14.0 + (14.0+x/60.0)/60.0;
        float lat = 50.0 + (52.0+y/60.0)/60.0;
        int height;
        
        srtmReadPx((y+52*60)/3, (x+14*60)/3, &height);
        
        char string[30];
        int num = sprintf(string, "%f %f %d\n", lat, lon, height); 
        //printf(string);
        fwrite(string, 1, num, out);
        
}



int main() {
    srtmLoadTile(50,14);
    
    out = fopen("/home/user/routino/latex/grafy/3D-srtm3.dat", "w");
    
    int x,y;
    for(y=21; y<=33; y+=3){
        for(x=9; x<=63; x+=3){
                output(x,y);
         } 
        fwrite("\n", 1, 1, out);
    }
    fclose(out);
    return 0;
    
    
//     out = fopen("/home/user/routino/latex/grafy/3D-srtm1-s plochami.dat", "w");
//    
//    int x,y;
//    for(y=21; y<=33; y+=1){
//        for(x=9; x<=63; x+=1){
//                output(x,y);
//         } 
//        fwrite("\n", 1, 1, out);
//    }
//    fclose(out);
//    return 0;
//       
    
    
//    out = fopen("/home/user/routino/latex/grafy/3D-srtm5.dat", "w");
//    
//    int x,y;
//    for(y=24; y<=27; y+=3){
//    for(x=9; x<=69; x+=3){
//        output(x,y);
//        output(x+3,y);
//        output(x+3,y+3);
//        output(x,y+3);
//        output(x,y);
//        fwrite("\n", 1, 1, out);
//    } 
//    }
//    
//    y = 30;
//    for(x=36; x<=60; x+=3){
//        output(x,y);
//        output(x+3,y);
//        output(x+3,y+3);
//        output(x,y+3);
//        output(x,y);
//        fwrite("\n", 1, 1, out);
//    }
//    fclose(out);
//    
    
}