// Example program
//#include "stdafx.h"

#include <iostream>
#include <string>


typedef float float3[3];

static float dot(const float3 vector1, const float3 vector2);
static float* cross_product(const float3 vector1, const float3 vector2);
static float vector_magnitude(const float3 vector);

void print_vector(const float* v)
{
    std::cout << v[0] << ", " << v[1] << ", " << v[2] << "\n";

}

static float* get_vertex_weight(
    const float3& vertex1, const float3& vertex2, const float3& vertex3, const float3& point)
{
    // barycentric coordinates to extract the area of the inverse try
    float area_v1, area_v2, area_v3;

    const float* vertex_array[3]{ vertex1, vertex2, vertex3 };

    float3 vertex_areas;

    for (unsigned int i = 0; i < 3; ++i)
    {
        float3 v0 = {
            vertex_array[(i + 1) % 3][0] - vertex_array[i][0],
            vertex_array[(i + 1) % 3][1] - vertex_array[i][1],
            vertex_array[(i + 1) % 3][2] - vertex_array[i][2],
        };

        float3 v1 = {
            vertex_array[(i + 2) % 3][0] - vertex_array[i][0],
            vertex_array[(i + 2) % 3][1] - vertex_array[i][1],
            vertex_array[(i + 2) % 3][2] - vertex_array[i][2],
        };

        float3 v2 = {
            point[0] - vertex_array[i][0],
            point[1] - vertex_array[i][1],
            point[2] - vertex_array[i][2]
        };

        float dot00 = dot(v0, v0);
        float dot01 = dot(v0, v1);
        float dot02 = dot(v0, v2);
        float dot11 = dot(v1, v1);
        float dot12 = dot(v1, v2);

        // calc barycentric
        float inv_denom = 1 / (dot00 * dot11 - dot01 * dot01);
        float u = 1 - ((dot11 * dot02 - dot01 * dot12) * inv_denom);
        float v = 1 - ((dot00 * dot12 - dot01 * dot02) * inv_denom);

        std::cout << u << ", " << v << std::endl;
        vertex_areas[i] = vector_magnitude(cross_product(
            float3{ v0[0] * u, v0[1] * u, v0[2] * u },
            float3{ v1[0] * v, v1[1] * v, v1[2] * v }));
    }

    float total_area = vertex_areas[0] + vertex_areas[1] + vertex_areas[2];

    static float3 vertex_height;
    for (unsigned int i = 0; i < 3; ++i)
    {
        // normalize areas and get inverse
        vertex_height[i] = vertex_areas[i] / total_area;
    }

    print_vector(vertex_height);
    std::cout << vertex_height[0] + vertex_height[1] + vertex_height[2] << std::endl;

    return vertex_height;
}

static float dot(const float3 vector1, const float3 vector2)
{
    return vector1[0] * vector2[0] + vector1[1] * vector2[1] + vector1[2] * vector2[2];
}

static float* cross_product(const float3 vector1, const float3 vector2)
{
    static float3 cross;
    for (unsigned int i = 0; i < 3; ++i)
    {
        cross[i] = vector1[(i + 1) % 3] * vector2[(i + 2) % 3] - vector1[(i + 2) % 3] * vector2[(i + 1) % 3];
    }

    return cross;
}

static float vector_magnitude(const float3 vector)
{
    return sqrt(vector[0] * vector[0] + vector[1] * vector[1] + vector[2] * vector[2]);
}

void print_string(std::string holi)
{
    std::cout << holi << std::endl;
}

int main()
{
    print_string(holi="holi");
}
