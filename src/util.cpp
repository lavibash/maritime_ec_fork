/*
 * util.cpp
 *
 * Implements various utilities for normalization and other
 * tasks.
 *
 * @author Vincent Wang
 */

#include <matrix/math.hpp>
#include <math.h>
#include <iostream>
#include <mec/util.h>
#include <mec/estimation.h>
#include <mec/control.h>

double normalize(double value, double min, double max)
{
    /* Normalize a value between extremes.
     * Output will be between -1.0 and 1.0.
     */

    // Clamp values greater or smaller than extremes
    if (value < min) value = min;
    if (value > max) value = max;

    double zero = (max + min) / 2;
    double mag = (max - min) / 2;
    value += zero;

    return value;
}

void offsets_to_frame(float input[3], float angles[3], float output[3])
{
    /* 
     * Input offsets at those angles from your frame,
     * Converts those offsets to your frame's offsets.
     * Input = x, y, z offsets
     * Angles = roll, pitch, yaw from your frame
     * Output = your frame's x, y, z offsets
     */

    // Pre-compute trig functions.
    float sphi = std::sin(angles[0]);
    float sthe = std::sin(angles[1]);
    float spsi = std::sin(angles[2]);
    float cphi = std::cos(angles[0]);
    float cthe = std::cos(angles[1]);
    float cpsi = std::cos(angles[2]);

    // Calculate rotation matrix for Euler transformation.
    float r11 =  cthe*cpsi;
    float r12 = -cphi*spsi + sphi*sthe*cpsi;
    float r13 = -sphi*spsi + cphi*sthe*cpsi;
    float r21 =  cthe*spsi;
    float r22 =  cphi*cpsi + sphi*sthe*spsi;
    float r23 =  sphi*cpsi + cphi*sthe*spsi;
    float r31 = -sthe;
    float r32 =  sphi*cthe;
    float r33 =  cphi*cthe;

    // Calculate matrix.
    output[0] = r11*input[0] + r12*input[1] + r13*input[2];
    output[1] = r21*input[0] + r22*input[1] + r23*input[2];
    output[2] = r31*input[0] + r32*input[1] + r33*input[2];
}

void transform_frame(float frame1[3], float frame2[3], float angle[3])
{
    /* Rotate one frame to another. Angles are (roll, pitch, yaw). */
    float xrot_arr[3][3] =
    {
        { 1, 0, 0 },
        { 0, cos(angle[1]), -sin(angle[1]) },
        { 0, sin(angle[1]), cos(angle[1]) }
    };

    float yrot_arr[3][3] =
    {
        { cos(angle[0]), 0, sin(angle[0]) },
        { 0, 1, 0 },
        { -sin(angle[0]), 0, cos(angle[0]) }
    };

    float zrot_arr[3][3] =
    {
        { cos(angle[2]), -sin(angle[2]), 0 },
        { sin(angle[2]), cos(angle[2]), 0 },
        { 0, 0, 1 }
    };

    Matrix<float, 3, 3> xrot(xrot_arr);
    Matrix<float, 3, 3> yrot(yrot_arr);
    Matrix<float, 3, 3> zrot(zrot_arr);

    Vector<float, 3> vec_1;
    Vector<float, 3> vec_2;

    vec_1(0) = frame1[0];
    vec_1(1) = frame1[1];
    vec_1(2) = frame1[2];

    vec_2 = xrot * yrot * zrot * vec_1;
    frame2[0] = vec_2(0);
    frame2[1] = vec_2(1);
    frame2[2] = vec_2(2);
}

void velocity_ned_to_body(struct mec_vehicle_velocity_body *body,
        struct mec_vehicle_velocity *ned, struct mec_vehicle_attitude *att)
{
    float ned_velocities[] = { ned->north_m_s, ned->east_m_s, ned->down_m_s };
    float angle[] = { -att->roll, -att->pitch, -att->yaw };
    float body_velocities[3];

    offsets_to_frame(ned_velocities, angle, body_velocities);
    body->forward_m_s = body_velocities[0];
    body->right_m_s = body_velocities[1];
    body->down_m_s = body_velocities[2];
}

void velocity_body_to_ned(struct mec_vehicle_velocity_body *body,
        struct mec_vehicle_velocity *ned, struct mec_vehicle_attitude *att)
{
    float body_velocities[] = { body->forward_m_s, body->right_m_s, body->down_m_s };
    float angle[] = { att->roll, att->pitch, att->yaw };
    float ned_velocities[3];

    offsets_to_frame(body_velocities, angle, ned_velocities);
    ned->north_m_s = ned_velocities[0];
    ned->east_m_s = ned_velocities[1];
    ned->down_m_s = ned_velocities[2];
}

float angle_difference(float a1, float a2)
{
    // For [-180, 180].
    float b1 = a1-a2;
    if (fabs(b1) > M_PI)
    {
        if (a1 < a2)
            a1 += 2*M_PI;
        else 
            a2 += 2*M_PI;
        b1 = a1-a2;
    }
    return b1;
}
