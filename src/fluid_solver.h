
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
    void advect(Ref<FieldFBO> _quantity, Ref<FieldFBO> _tmp_quantity, float _dt);
    void diffuseVelocity();
    void subtractPressureGradient();
    
    //
    void computeDivergence();
    void computePressure();
    void computeCurl();
    void computeSpeed();

    // DEBUG functions
    void __debug_init_velocity();


    // accessors
    const Ref<FieldFBO> &velocity()     { return m_velocity;    }
    const Ref<FieldFBO> &divergence()   { return m_divergence;  }
    const Ref<FieldFBO> &curl()         { return m_curl;        }
    const Ref<FieldFBO> &pressure()     { return m_pressure;    }
    const Ref<FieldFBO> &speed()        { return m_speed;       }
    bool initialized() { return m_initialized; }

private:

    //
    bool m_clearPressure = false;
    bool m_clearVelocity = false;

    uint32_t m_jacobiIterCount = 40;
    glm::ivec2 m_dim;
    glm::vec2 m_cellSize;
    glm::vec2 m_vp;
    float m_y_aspectRatio;

    float m_dx = 1.0f;

    // fields
    Ref<FieldFBO> m_velocity;
    Ref<FieldFBO> m_divergence;
    Ref<FieldFBO> m_curl;
    Ref<FieldFBO> m_pressure;
    Ref<FieldFBO> m_speed;
    Ref<FieldFBO> m_tmp_fbo0, m_tmp_fbo1;

    // shaders
    Ref<Shader> m_advectShader;
    Ref<Shader> m_divergenceShader;
    Ref<Shader> m_pressureShader;

    // flags
    bool m_initialized = false;

};

