#define FNL_IMPL

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <FastNoiseLite.h>

#include <generation.h>

float lib_noise(int x, int y, float freq){
    fnl_state noise = fnlCreateState();
    noise.seed = 4545;
    noise.noise_type = FNL_NOISE_PERLIN;
    noise.frequency = 0.008f;
    float noise_val = fnlGetNoise2D(&noise, (float)x * freq, (float)y* freq);
    return (noise_val + 1.f) / 2.0f;
}

float lib_noise_simplex(int x, int y, float freq){
    fnl_state noise = fnlCreateState();
    noise.seed = 4545;
    noise.noise_type = FNL_NOISE_OPENSIMPLEX2;
    noise.frequency = 0.008f;
    float noise_val = fnlGetNoise2D(&noise, (float)x * freq, (float)y* freq);
    return (noise_val + 1.f) / 2.0f;
}

float get_noise_tree(int x, int y){
    fnl_state noise = fnlCreateState();
    noise.seed = 4567878;
    noise.noise_type = FNL_NOISE_PERLIN;
    noise.frequency = 0.9f;
    float noise_val = fnlGetNoise2D(&noise, (float)x, (float)y);
    return (noise_val + 1.f) / 2.0f;
}


float get_noise(int x, int y){
    // noise.frequency = 0.005f;

    float moutains = 0.f;
    // float moutains = lib_noise_simplex(x, y, 0.1);
    // if (moutains < .75){
    //     moutains = 0.f;
    // }else{
    //     moutains = lib_noise_simplex(x, y, 0.1) + 0.1 * lib_noise(x, y, 0.5);
    //     moutains = moutains / (1 +0.1);
    // }
    // moutains = roundf(moutains * 16) / 17;


    float elevation = 1.3 * lib_noise_simplex(x, y, 0.18)
                    + 0.4 * lib_noise(x, y, 0.5)
                    + 0.15 * lib_noise(x, y, 1.0)
                    + 0.025 * lib_noise(x, y, 2);

    elevation = elevation / (1.3 + 0.4 + 0.15 + 0.025);
    elevation = powf(elevation, 2.f);
    if (moutains > elevation){
        return moutains;
    }
    return elevation;



    // float elevation = lib_noise(0.25*x, 0.25*y);
    //                 + 0.5 * lib_noise(2 * x, 2 * y);
    //                 + 0.25 * lib_noise(4 * x, 4 * y);
    //                 + 0.125 * lib_noise(8 * x, 8 * y);

    // // return into the 0-1 range
    // elevation = elevation / (1 + 0.5 + 0.25 + 0.125);

    // // Create valley
    // elevation = powf(elevation, 1.2f);

    // return elevation;


    // // Gather noise data
    // float noise_val =   1 * ((fnlGetNoise2D(&noise, (float)x, (float)y)+1)/2) 
    //                 +   0.5 * ((fnlGetNoise2D(&noise, (float)x*2, (float)y*2)+1)/2);

    // noise_val /= (1+0.5);
    // noise_val = powf(noise_val, 0.7f);
    // return noise_val;

    // Terrace
    // noise_val = roundf(noise_val * 5) / 5;

    // return noise_val;
}
