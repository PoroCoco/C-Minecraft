#define FNL_IMPL

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <FastNoiseLite.h>

#include <generation.h>



double get_noise_tree(int x, int y){
    fnl_state noise = fnlCreateState();
    noise.seed = 4567878;
    noise.noise_type = FNL_NOISE_PERLIN;
    noise.frequency = 0.9f;
    float noise_val = fnlGetNoise2D(&noise, (float)x, (float)y);
    return (noise_val + 1.f) / 2.0f;
}


double get_noise(int x, int y){

    fnl_state noise = fnlCreateState();
    noise.seed = 4567;
    noise.noise_type = FNL_NOISE_PERLIN;
    noise.frequency = 0.004f;
    // noise.frequency = 0.005f;
    float noise_val = fnlGetNoise2D(&noise, (float)x, (float)y);
    return (noise_val + 1.f) / 2.0f;
    // // Gather noise data
    // float noise_val =   1 * ((fnlGetNoise2D(&noise, (float)x, (float)y)+1)/2) 
    //                 +   0.5 * ((fnlGetNoise2D(&noise, (float)x*2, (float)y*2)+1)/2);

    // noise_val /= (1+0.5);
    // noise_val = powf(noise_val, 0.7f);
    // return noise_val;

    // Terrace
    // noise_val = roundf(noise_val * 5) / 5;

    return noise_val;
}
