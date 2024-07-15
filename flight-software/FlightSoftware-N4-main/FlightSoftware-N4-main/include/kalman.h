
#pragma once

/* define struct to hold filtered data*/
struct Filtered_Data{
    float x_acceleration;
};


/* this function returns Kalman-filtered data */
struct Filtered_Data filterData(float);

