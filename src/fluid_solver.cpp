
#include "fluid_solver.h"

#include <synapse/SynapseCore/Renderer/Shader/ShaderLibrary.hpp>

#include "quad.h"


//
FluidSolver::FluidSolver(const glm::ivec2 _dim)
{
    m_dim = _dim;
    m_cellSize = 1.0f / glm::vec2(m_dim.x, m_dim.y);
    m_vp = Renderer::get().getViewportF();
    m_y_aspectRatio = m_dim.y / (float)m_dim.x;

    // initalize fields
    m_velocity      = VectorField(m_dim, "velocity");
    m_divergence    = ScalarField(m_dim, "divergence");
    m_curl          = VectorField(m_dim, "curl");
    m_pressure      = ScalarField(m_dim, "pressure");
    m_speed         = ScalarField(m_dim, "speed");
    m_tmp_fbo0      = VectorField(m_dim, "tmp_field_0");
    m_tmp_fbo1      = VectorField(m_dim, "tmp_field_1");

    // compile shaders
    m_advectShader = \
        ShaderLibrary::load("advect_shader", 
                            FileName("../assets/shaders/global/screen.vert"), 
                            FileName("../assets/shaders/solver/advect.frag"));

    m_divergenceShader = \
        ShaderLibrary::load("divergence_shader",
                            FileName("../assets/shaders/global/stencil.vert"), 
                            FileName("../assets/shaders/solver/divergence.frag"));

    m_pressureShader = \
        ShaderLibrary::load("pressure_shader",
                            FileName("../assets/shaders/global/stencil.vert"), 
                            FileName("../assets/shaders/solver/pressure.frag"));

    m_initialized = true;
    SYN_TRACE("FluidSolver initialized [ ", m_dim.x, " x ", m_dim.y, " ]");

}

//---------------------------------------------------------------------------------------
void FluidSolver::step(float _dt)
{
    if (m_clearVelocity)
    {
        m_velocity->clear(glm::vec4(0.0f), GL_COLOR_BUFFER_BIT);
        m_pressure->clear(glm::vec4(0.0f), GL_COLOR_BUFFER_BIT);
        m_clearVelocity = false;
    }

    Quad::bind();

    applyForce();
    applyVorticityConfinement();
    advect(m_velocity, m_tmp_fbo0, _dt);
    diffuseVelocity();
    
    computeSpeed();

}

//---------------------------------------------------------------------------------------
void FluidSolver::applyForce()
{

}

//---------------------------------------------------------------------------------------
void FluidSolver::applyVorticityConfinement()
{

}

//---------------------------------------------------------------------------------------
void FluidSolver::advect(Ref<FieldFBO> _quantity, 
                         Ref<FieldFBO> _tmp_quantity,
                         float _dt)
{
    // moves a quantity (density (ink) or velocity itself) along the velocity field
    // uses Runge-Kutta 4 to trace back along the last timestep

    m_advectShader->enable();
    _tmp_quantity->bind();
    
    m_advectShader->setUniform1f("u_dt", _dt);
    glm::vec2 world2tx = 1.0f / (2.0f * glm::vec2(1.0f, m_y_aspectRatio));
    m_advectShader->setUniform2fv("u_world2tx", world2tx);

    m_velocity->bindTexture(0, 0, GL_LINEAR);
    _quantity->bindTexture(0, 0, GL_LINEAR);

    Quad::render();

    std::swap(_quantity, _tmp_quantity);

}

//---------------------------------------------------------------------------------------
void FluidSolver::diffuseVelocity()
{

}

//---------------------------------------------------------------------------------------
void FluidSolver::subtractPressureGradient()
{
    
}

//---------------------------------------------------------------------------------------
void FluidSolver::computeDivergence()
{
    Quad::bind();

    m_divergence->bind();
    m_divergenceShader->enable();

    m_divergenceShader->setUniform1i("u_velocity", 0);
    m_divergenceShader->setUniform2fv("u_tx_size", m_cellSize);
    m_divergenceShader->setUniform1f("u_half_inv_dx", 0.5f / m_dx);

    m_velocity->bindTexture(0);

    Quad::render();

}

//---------------------------------------------------------------------------------------
void FluidSolver::computePressure()
{
    computeDivergence();

    if (m_clearPressure)
        m_pressure->clear(glm::vec4(0.0f), GL_COLOR_BUFFER_BIT);
    
    m_pressureShader->enable();

    m_pressureShader->setUniform2fv("u_tx_size", m_cellSize);
    m_pressureShader->setUniform1f("u_dx2", std::pow(m_dx, 2));

    m_pressureShader->setUniform1i("u_pressure", 0);
    m_pressureShader->setUniform1i("u_divergence", 1);

    m_divergence->bindTexture(1);

    for (uint32_t i = 0; i < m_jacobiIterCount; i++)
    {
        m_tmp_fbo0->bind();
        m_pressure->bindTexture(0);
        Quad::render();
        std::swap(m_tmp_fbo0, m_pressure);
    }

}

//---------------------------------------------------------------------------------------
void FluidSolver::computeCurl()
{

}

//---------------------------------------------------------------------------------------
void FluidSolver::computeSpeed()
{

}

//---------------------------------------------------------------------------------------
void FluidSolver::__debug_init_velocity()
{
    static Ref<Shader> init_vel_shader = ShaderLibrary::load("init_vel_shader",
                                                             FileName("../assets/shaders/global/stencil.vert"),
                                                             FileName("../assets/shaders/debug_init_velocity.frag"));

    m_velocity->bind();
    Quad::bind();

    init_vel_shader->enable();
    init_vel_shader->setUniform2fv("u_tx_size", m_cellSize);
    
    Quad::render();

}




