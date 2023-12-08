
#include "fluid_solver.h"
#include "quad.h"


//
FluidSolver::FluidSolver(const glm::ivec2 _dim)
{
    m_dim = _dim;
    m_velocity      = VectorField(m_dim, "velocity");
    m_divergence    = ScalarField(m_dim, "divergence");
    m_curl          = VectorField(m_dim, "curl");
    m_pressure      = ScalarField(m_dim, "pressure");
    m_speed         = ScalarField(m_dim, "speed");
    m_tmp_fbo0      = VectorField(m_dim, "tmp_field_0");
    m_tmp_fbo1      = VectorField(m_dim, "tmp_field_1");

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
    advect(m_velocity, m_tmp_fbo0);
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
void FluidSolver::advect(const Ref<FieldFBO> &_quantity, const Ref<FieldFBO> &_tmp_quantity)
{
    
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

}

//---------------------------------------------------------------------------------------
void FluidSolver::computePressure()
{

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



