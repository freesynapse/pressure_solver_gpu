
#pragma once

#include "field_fbo.h"


//
class FluidSolver
{
public:
    FluidSolver() {}
    FluidSolver(const glm::ivec2 _dim);
    ~FluidSolver() = default;

    //
    void step(float _dt);

    //
    void applyForce();
    void applyVorticityConfinement();
    void advect(const Ref<FieldFBO> &_quantity, const Ref<FieldFBO> &_tmp_quantity);
    void diffuseVelocity();
    void subtractPressureGradient();
    
    //
    void computeDivergence();
    void computePressure();
    void computeCurl();
    void computeSpeed();
    

private:

    //
    bool m_clearPressure = false;
    bool m_clearVelocity = false;

    uint32_t m_jacobiIterCount = 40;
    glm::ivec2 m_dim;
    glm::vec2 m_cellSize;
    float m_y_aspectRatio;

    float m_dx = 1.0f;

    Ref<FieldFBO> m_velocity;
    Ref<FieldFBO> m_divergence;
    Ref<FieldFBO> m_curl;
    Ref<FieldFBO> m_pressure;
    Ref<FieldFBO> m_speed;
    Ref<FieldFBO> m_tmp_fbo0, m_tmp_fbo1;

};

