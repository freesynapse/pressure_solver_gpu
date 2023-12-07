
#include "arrows_2D.h"

#include <synapse/API>


//
Arrows2D::Arrows2D(const Ref<FieldFBO> &_vector_field, uint32_t _sampling_rate)
{
    m_dim = _vector_field->getSize();
    m_samplingRate = _sampling_rate;
    m_vertexCount = _vector_field->fieldSize();
    m_data = new glm::vec4[m_vertexCount];
    _vector_field->readFieldData((void*)&m_data[0]);

    updateData();

}

//---------------------------------------------------------------------------------------
void Arrows2D::updateData()
{
    m_range = { std::numeric_limits<float>::max(), std::numeric_limits<float>::min() };
    float v_magnitudes[m_vertexCount];

    for (int y = 0; y < m_dim.y; y++)
    {
        for (int x = 0; x < m_dim.x; x++)
        {
            int idx = y * m_dim.x + x;
            glm::vec2 v_xy = { m_data[idx].x, m_data[idx].y };
            float m = glm::length(v_xy);
            m_range[0] = min(m, m_range[0]);
            m_range[1] = max(m, m_range[1]);
            
            v_magnitudes[idx] = m;
            printf("x %d, y %d : idx %d : v_magnitudes[idx] = %f\n", x, y, idx, v_magnitudes[idx]);

        }
    }

    float inv_max_mag = 1.0f / m_range[1];
    arrow_2D_vertex_t V[m_vertexCount];
    glm::vec2 ndc_pos = { -1.0f, -1.0f };
    float dx = 2.0f / (float)(m_dim.x);
    float dy = 2.0f / (float)(m_dim.y);
    glm::vec2 mid = { dx * 0.5f, dy * 0.5f };
    const float size = Renderer::get().getViewportF().y / (float)m_dim.y;

    for (int y = 0; y < m_dim.y; y++)
    {
        ndc_pos.x = -1.0f;
        for (int x = 0; x < m_dim.x; x++)
        {
            int idx = y * m_dim.x + x;
            glm::vec2 v = m_data[idx];
            float v_mag = v_magnitudes[idx];
            V[idx] =
            {
                .position = ndc_pos + mid,
                .size = size,
                .orientation = atan2f(v.y, v.x),
                .linewidth = 0.1f,
                .magnitude = v_mag * inv_max_mag, // normalize magnitude [0..1]
            };
            printf("x %d, y %d, idx %d, dx %f, dy %f : \n", x, y, idx, dx, dy);
            V[idx].__debug_print();
            printf("\n");
            ndc_pos.x += dx;

        }
        ndc_pos.y += dy;

    }

    //
    Ref<VertexBuffer> vbo = API::newVertexBuffer(GL_STATIC_DRAW);
    vbo->setBufferLayout({
        { VERTEX_ATTRIB_LOCATION_POSITION, ShaderDataType::Float2, "a_position" },
        { 1, ShaderDataType::Float, "a_size" },
        { 2, ShaderDataType::Float, "a_orientation" },
        { 3, ShaderDataType::Float, "a_linewidth" },
        { 4, ShaderDataType::Float, "a_magnitude" },
    });

    vbo->setData(V, sizeof(V));

    //
    m_vao = API::newVertexArray(vbo);

    // create shader
    m_shader = ShaderLibrary::load("../assets/shaders/visualization/quiver2D.glsl");

    SYN_TRACE("Arrows2D object initialized from VectorField.");

}

//---------------------------------------------------------------------------------------
void Arrows2D::render()
{
    if (m_vao == nullptr)
        return;

    static auto &renderer = Renderer::get();

    m_shader->enable();
    m_shader->setUniform1f("u_antialias", 0.02f);
    renderer.drawArrays(m_vao, m_vertexCount, 0, false, GL_POINTS);

}



