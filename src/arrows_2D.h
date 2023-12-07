#pragma once

#include <synapse/Renderer>
#include "field_fbo.h"

using namespace Syn;


//
struct arrow_2D_vertex_t
{
    glm::vec2 position;
    float size;
    float orientation;
    float linewidth;
    float magnitude;

    void __debug_print()
    {
        printf("\tposition    = %f, %f\n", position.x, position.y);
        printf("\tsize        = %f\n", size);
        printf("\torientation = %f\n", orientation);
        printf("\tlinewidth   = %f\n", linewidth);
        printf("\tmagnitude   = %f\n", magnitude);
    }
};

//
class Arrows2D
{
public:
    Arrows2D(const Ref<FieldFBO> &_vector_field, uint32_t _sampling_rate=1);
    ~Arrows2D() { if (m_data) delete[] m_data; }

    void updateData();
    void render();

private:
    glm::ivec2 m_dim;
    glm::vec4 *m_data = nullptr;
    Ref<VertexArray> m_vao;
    glm::vec2 m_range;
    uint32_t m_vertexCount = 0;
    uint32_t m_samplingRate;

    Ref<Shader> m_shader;

};
