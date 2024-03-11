#pragma once

#include <stdbool.h>
#include <chunk.h>
#include <world.h>
#include <player.h>
#include <camera.h>
#include <cglm/cglm.h> 


typedef struct plane {
    // unit vector
    vec3 normal;
    // distance from origin to the nearest point in the plane
    float distance;             
} plane;

typedef struct frustum {
    plane topFace;
    plane bottomFace;

    plane rightFace;
    plane leftFace;

    plane farFace;
    plane nearFace;
} frustum;

bool chunk_is_in_frustum(frustum * f, chunk *c);
void world_update_frustum(world *w, player *p, float fov, float aspect, float z_near, float z_far);