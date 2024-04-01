#pragma once

#define TIMING_START(timing_name) float timing_name = (float)glfwGetTime();
#define TIMING_END(timing_name) fprintf(stderr, "%s : %f s\n", #timing_name, (float)glfwGetTime() - timing_name);