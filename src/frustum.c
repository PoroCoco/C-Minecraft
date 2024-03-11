#include <frustum.h>

typedef struct sphere {
    vec3 center;
    float radius;
} sphere;

float get_signed_distance_to_plane(plane *p, vec3 point){
    return glm_vec3_dot(p->normal, point) - p->distance;
}

bool is_on_or_forward_plane(plane *p, sphere *s){
    return get_signed_distance_to_plane(p, s->center) > - s->radius;
}

bool chunk_is_in_frustum(frustum * f, chunk *c){
    sphere s = {
        .center = {(float)(c->x) * CHUNK_X_SIZE, (float)0, (float)(c->z)*CHUNK_Z_SIZE},
        // .radius = CHUNK_X_SIZE > CHUNK_Z_SIZE ? CHUNK_X_SIZE/2.f : CHUNK_Z_SIZE/2.f
        .radius = 1000000.f // This should be the radius of the chunk but I don't know why this kind of works ... 
    };
    

    return (is_on_or_forward_plane(&f->leftFace, &s) &&
        is_on_or_forward_plane(&f->rightFace, &s) &&
        is_on_or_forward_plane(&f->farFace, &s) &&
        is_on_or_forward_plane(&f->nearFace, &s) &&
        is_on_or_forward_plane(&f->topFace, &s) &&
        is_on_or_forward_plane(&f->bottomFace, &s));
}

frustum frustum_init(camera *cam, float aspect, float fovY, float zNear, float zFar)
{
    frustum frustum;
    const float halfVSide = zFar * tanf(fovY * .5f);
    const float halfHSide = halfVSide * aspect;
    vec3 frontMultFar;
    glm_vec3_scale(cam->cameraFront, zFar, frontMultFar);
    vec3 camera_right;
    glm_vec3_cross(cam->cameraUp, cam->cameraFront, camera_right);
    glm_vec3_norm(camera_right);

    vec3 tmp;
    // nearFace
    glm_vec3_copy(cam->cameraFront, frustum.nearFace.normal);
    glm_vec3_norm(frustum.nearFace.normal);

    glm_vec3_scale(cam->cameraFront, zNear, tmp);
    glm_vec3_add(tmp, cam->cameraPos, tmp);
    frustum.nearFace.distance = glm_vec3_dot(frustum.nearFace.normal, tmp);

    // farFace
    glm_vec3_copy(cam->cameraFront, frustum.farFace.normal);
    glm_vec3_scale(frustum.farFace.normal, -1.f, frustum.farFace.normal);
    glm_vec3_norm(frustum.farFace.normal);

    glm_vec3_add(cam->cameraPos, frontMultFar, tmp);
    frustum.farFace.distance = glm_vec3_dot(frustum.farFace.normal, tmp);
    
    // rightFace
    glm_vec3_scale(camera_right, halfHSide, tmp);
    glm_vec3_sub(frontMultFar, tmp, tmp);
    glm_vec3_cross(tmp, cam->cameraUp, frustum.rightFace.normal);
    glm_vec3_norm(frustum.rightFace.normal);

    frustum.rightFace.distance = glm_vec3_dot(frustum.rightFace.normal, cam->cameraPos);

    // leftFace
    glm_vec3_scale(camera_right, halfHSide, tmp);
    glm_vec3_add(frontMultFar, tmp, tmp);
    glm_vec3_cross(cam->cameraUp, tmp, frustum.leftFace.normal);
    glm_vec3_norm(frustum.leftFace.normal);

    frustum.leftFace.distance = glm_vec3_dot(frustum.leftFace.normal, cam->cameraPos);

    // topFace
    glm_vec3_scale(cam->cameraUp, halfVSide, tmp);
    glm_vec3_sub(frontMultFar, tmp, tmp);
    glm_vec3_cross(camera_right, tmp, frustum.topFace.normal);
    glm_vec3_norm(frustum.topFace.normal);

    frustum.topFace.distance = glm_vec3_dot(frustum.topFace.normal, cam->cameraPos);

    // bottomFace
    glm_vec3_scale(cam->cameraUp, halfVSide, tmp);
    glm_vec3_add(frontMultFar, tmp, tmp);
    glm_vec3_cross(tmp, camera_right, frustum.bottomFace.normal);
    glm_vec3_norm(frustum.bottomFace.normal);

    frustum.bottomFace.distance = glm_vec3_dot(frustum.bottomFace.normal, cam->cameraPos);

    return frustum;
}

void world_update_frustum(world *w, player *p, float fov, float aspect, float z_near, float z_far){
    frustum frustum = frustum_init(p->cam, aspect, fov, z_near, z_far);

    int frustum_count = 0;
    for (size_t i = 0; i < TOTAL_CHUNKS; i++){// ToDo : fixray for each
        if (w->loaded_chunks->container[i] != _fixray_null){
            chunk *c = w->loaded_chunks->container[i];
            c->in_frustum = chunk_is_in_frustum(&frustum, c);
            if (c->in_frustum){
                frustum_count++;
            }
        }
    }
    printf("Total chunks : %d, in frustum %d\n", TOTAL_CHUNKS, frustum_count);
}
