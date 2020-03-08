// Example program
#include "stdafx.h"

#include <iostream>
#include <string>

typedef float float3[3];

static void get_vertex_area(const float3 v1, const float3 v2, const float3& v3)
{
    // barycentric coordinates to extract the area of the inverse try
    //float area_v1, area_v2, area_v3;
    const float* farray[3]{ v1, v2, v3 };

    std::cout << farray[0][0];

    for (auto v : { v1, v2, v3 })
    {
        std::cout << v[0] << std::endl;
        std::cout << v << std::endl;
    }

}

int main()
{
    float3 v1{ 1,1,1 };
    float3 v2{ 2,2,2 };
    float3 v3{ 3,3,3 };

    std::cout << v1 << std::endl;
    std::cout << v2 << std::endl;
    std::cout << v3 << std::endl;

    get_vertex_area(v1, v2, v3);

}
