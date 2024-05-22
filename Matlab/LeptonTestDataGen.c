// Online C compiler to run C program online
#include <stdio.h>

int main() {
    //for(int segment = 1; segment < 3; segment++){
    for(int segment = 3; segment < 5; segment++){
        for(int packet = 0; packet < 59; packet++){
            printf("\n");
            printf("0x%x,", segment << 4);
            printf("0x%x,", packet);
            printf("\n");
            int pixel_clr = 0;
            for(int pixel = 0; pixel < 60; pixel++){
                printf("0x%x, 0x%x,", 0x00, pixel_clr);
                pixel_clr++;
            }
        }
    }
    return 0;
}