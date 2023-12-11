
#include <synapse/Synapse>
#include <synapse/SynapseMain.hpp>

#include "quad.h"
#include "field_fbo.h"
#include "arrows_2D.h"
#include "fluid_solver.h"

using namespace Syn;


// TODO: refactor into FluidRenderer later
#define VELOCITY_FIELD      0
#define DIVERGENCE_FIELD    1
#define PRESSURE_FIELD      2
#define CURL_FIELD          3
#define SPEED_FIELD         4


//
class layer : public Layer
{
public:
    layer() : Layer("layer") {}
    virtual ~layer() {}

    virtual void onAttach() override;
    virtual void onUpdate(float _dt) override;
    virtual void onImGuiRender() override;
    void onKeyDownEvent(Event *_e);
    void onMouseButtonEvent(Event *_e);
    void onResize(Event *_e);

private:
    Ref<Framebuffer> m_renderBuffer = nullptr;
    Ref<Font> m_font = nullptr;
    glm::ivec2 m_vp = { 0, 0 };

    Ref<Shader> m_divergenceShader = nullptr;
    Ref<FieldFBO> m_velocity = nullptr;
    Ref<FieldFBO> m_divergence = nullptr;

    Ref<Shader> m_scalarFieldShader = nullptr;
    Ref<Shader> m_vectorFieldShader = nullptr;

    Ref<FluidSolver> m_solver = nullptr;

    Ref<Arrows2D> m_quiver = nullptr;
    
    // determine which field is rendering
    uint32_t m_currentField = 0;
    void nextField() { change_current_field_( 1); }
    void prevField() { change_current_field_(-1); }
    void change_current_field_(int _offset);
    const char *field_ID(uint32_t _field);
    Shader *m_activeFieldShader = nullptr;
    FieldFBO *m_activeField = nullptr;
    void __debug_active_field();

    // flags
    bool m_renderFluid = false;
    bool m_showQuiver = false;
    bool m_wireframeMode = false;
    bool m_toggleCulling = false;

};

//
class syn_app_instance : public Application
{
public:
    syn_app_instance() { this->pushLayer(new layer); }
};
Application* CreateSynapseApplication() { return new syn_app_instance(); }

//---------------------------------------------------------------------------------------
void layer::onResize(Event *_e)
{
    ViewportResizeEvent *e = dynamic_cast<ViewportResizeEvent*>(_e);
    m_vp = e->getViewport();

    glm::ivec2 dim = Renderer::get().getViewport() / 256;
    m_solver = std::make_shared<FluidSolver>(dim);
    m_solver->__debug_init_velocity();
    m_solver->computePressure();

    m_quiver = std::make_shared<Arrows2D>(m_solver->velocity(), 1);

    change_current_field_(0);
    m_renderFluid = true;

}

//---------------------------------------------------------------------------------------
void layer::onAttach()
{
    // register event callbacks
    EventHandler::register_callback(EventType::INPUT_KEY, SYN_EVENT_MEMBER_FNC(layer::onKeyDownEvent));
    EventHandler::register_callback(EventType::INPUT_MOUSE_BUTTON, SYN_EVENT_MEMBER_FNC(layer::onMouseButtonEvent));
    EventHandler::register_callback(EventType::VIEWPORT_RESIZE, SYN_EVENT_MEMBER_FNC(layer::onResize));
    EventHandler::push_event(new WindowToggleFullscreenEvent());

    //
    m_renderBuffer = API::newFramebuffer(ColorFormat::RGBA16F, glm::ivec2(0), 1, true, true, "render_buffer");

    // visualization shaders
    m_scalarFieldShader = ShaderLibrary::load("../assets/shaders/visualization/scalarField.glsl");
    m_vectorFieldShader = ShaderLibrary::load("../assets/shaders/visualization/vectorField.glsl");

    // m_divergenceShader = ShaderLibrary::load("../assets/shaders/divergence.glsl");
    m_divergenceShader = ShaderLibrary::load(FileName("../assets/shaders/global/stencil.vert"),
                                             FileName("../assets/shaders/solver/divergence.frag"));

    // load font
    m_font = MakeRef<Font>("../assets/ttf/JetBrainsMono-Medium.ttf", 14.0f);
    m_font->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));


    // general settings
	Renderer::get().setClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	Renderer::get().disableImGuiUpdateReport();
    // Application::get().setMaxFPS(30.0f);
}

//---------------------------------------------------------------------------------------
void layer::onUpdate(float _dt)
{
    SYN_PROFILE_FUNCTION();
	
    static auto& renderer = Renderer::get();

    // -- START SIMULATION -- //


    // -- END SIMULATION -- //

    m_renderBuffer->bind();
    if (m_wireframeMode)
        renderer.enableWireFrame();    
    renderer.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    // -- BEGINNING OF FIELD RENDERING -- //

    if (m_renderFluid)
    {
        Quad::bind();
        
        //
        if (m_activeField != nullptr)
        {
            m_activeFieldShader->enable();
            m_activeField->bindTexture(0);
            m_activeFieldShader->setUniform1i("u_text_sampler", 0);    
            Quad::render();
        } 
        
        if (m_quiver && m_showQuiver)
          m_quiver->render();

    }

    // -- END OF SCENE -- //


    if (m_wireframeMode)
        renderer.disableWireFrame();

	
    // Text rendering 
    // TODO: all text rendering should go into an overlay layer.
    static float fontHeight = m_font->getFontHeight() + 1.0f;
    int i = 0;
    m_font->beginRenderBlock();
	m_font->addString(2.0f, fontHeight * ++i, "fps=%.0f  VSYNC=%s", TimeStep::getFPS(), Application::get().getWindow().isVSYNCenabled() ? "ON" : "OFF");
    m_font->addString(2.0f, fontHeight * ++i, "Active field: %s (%d)", field_ID(m_currentField), m_currentField);
    m_font->endRenderBlock();

    //
    m_renderBuffer->bindDefaultFramebuffer();
}
 
//---------------------------------------------------------------------------------------
void layer::onKeyDownEvent(Event *_e)
{
    KeyDownEvent *e = dynamic_cast<KeyDownEvent*>(_e);
    static bool vsync = true;

    if (e->getAction() == SYN_KEY_PRESSED)
    {
        switch (e->getKey())
        {
            case SYN_KEY_Z:         vsync = !vsync; Application::get().getWindow().setVSYNC(vsync); break;
            case SYN_KEY_V:         m_renderBuffer->saveAsPNG(); break;
            case SYN_KEY_ESCAPE:    EventHandler::push_event(new WindowCloseEvent()); break;
            case SYN_KEY_F4:        m_wireframeMode = !m_wireframeMode; break;
            case SYN_KEY_F5:        m_toggleCulling = !m_toggleCulling; Renderer::setCulling(m_toggleCulling); break;

            case SYN_KEY_TAB:       m_showQuiver = !m_showQuiver; break;

            case SYN_KEY_LEFT:      prevField(); break;
            case SYN_KEY_RIGHT:     nextField(); break;
            
            case SYN_KEY_F:         __debug_active_field(); break;

            default: break;

        }
    }
    
}

//---------------------------------------------------------------------------------------
void layer::onMouseButtonEvent(Event *_e)
{
    MouseButtonEvent *e = dynamic_cast<MouseButtonEvent *>(_e);

    switch (e->getButton())
    {
        case SYN_MOUSE_BUTTON_1:    break;
        case SYN_MOUSE_BUTTON_2:    break;
        default: break;
    }
    
}

//---------------------------------------------------------------------------------------
void layer::onImGuiRender()
{
    static bool p_open = true;

    static bool opt_fullscreen_persistant = true;
    static ImGuiDockNodeFlags opt_flags = ImGuiDockNodeFlags_None;
    bool opt_fullscreen = opt_fullscreen_persistant;

    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    if (opt_fullscreen)
    {
    	ImGuiViewport* viewport = ImGui::GetMainViewport();
    	ImGui::SetNextWindowPos(viewport->Pos);
    	ImGui::SetNextWindowSize(viewport->Size);
    	ImGui::SetNextWindowViewport(viewport->ID);
    	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    }

    // When using ImGuiDockNodeFlags_PassthruDockspace, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
    if (opt_flags & ImGuiDockNodeFlags_PassthruCentralNode)
	    window_flags |= ImGuiWindowFlags_NoBackground;

    window_flags |= ImGuiWindowFlags_NoTitleBar;

    ImGui::GetCurrentContext()->NavWindowingToggleLayer = false;

    //-----------------------------------------------------------------------------------
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("synapse-core", &p_open, window_flags);
    ImGui::PopStyleVar();

    if (opt_fullscreen)
	    ImGui::PopStyleVar(2);

    // Dockspace
    ImGuiIO& io = ImGui::GetIO();
    ImGuiID dockspace_id = 0;
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        dockspace_id = ImGui::GetID("dockspace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), opt_flags);
    }
	
    //-----------------------------------------------------------------------------------
    // set the 'rest' of the window as the viewport
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("synapse-core::renderer");
    static ImVec2 oldSize = { 0, 0 };
    ImVec2 viewportSize = ImGui::GetContentRegionAvail();

    if (viewportSize.x != oldSize.x || viewportSize.y != oldSize.y)
    {
        // dispatch a viewport resize event -- registered classes will receive this.
        EventHandler::push_event(new ViewportResizeEvent(glm::vec2(viewportSize.x, viewportSize.y)));
        SYN_CORE_TRACE("viewport [ ", viewportSize.x, ", ", viewportSize.y, " ]");
        oldSize = viewportSize;
    }

    // direct ImGui to the framebuffer texture
    ImGui::Image((void*)m_renderBuffer->getColorAttachmentIDn(0), viewportSize, { 0, 1 }, { 1, 0 });

    ImGui::End();
    ImGui::PopStyleVar();


    // end root
    ImGui::End();

}

//---------------------------------------------------------------------------------------
void layer::change_current_field_(int _offset)
{
    m_currentField = (m_currentField + _offset + 5) % 5;
    // set active field for rendering
    switch(m_currentField)
    {
        case VELOCITY_FIELD:    m_activeField = m_solver->velocity().get();     m_activeFieldShader = m_vectorFieldShader.get(); break;
        case DIVERGENCE_FIELD:  m_activeField = m_solver->divergence().get();   m_activeFieldShader = m_scalarFieldShader.get(); break;
        case PRESSURE_FIELD:    m_activeField = m_solver->pressure().get();     m_activeFieldShader = m_scalarFieldShader.get(); break;
        case CURL_FIELD:        m_activeField = m_solver->curl().get();         m_activeFieldShader = m_scalarFieldShader.get(); break;
        case SPEED_FIELD:       m_activeField = m_solver->speed().get();        m_activeFieldShader = m_scalarFieldShader.get(); break;
        
        default: m_activeField = nullptr; m_activeFieldShader = nullptr; break;
    }

}

//---------------------------------------------------------------------------------------
const char *layer::field_ID(uint32_t _field)
{
    switch(_field)
    {
        case VELOCITY_FIELD:    return "VELOCITY_FIELD";    break;
        case DIVERGENCE_FIELD:  return "DIVERGENCE_FIELD";  break;
        case PRESSURE_FIELD:    return "PRESSURE_FIELD";    break;
        case CURL_FIELD:        return "CURL_FIELD";        break;
        case SPEED_FIELD:       return "SPEED_FIELD";       break;
        default:                return "UNKNOWN FIELD";     break;
    }
}

//---------------------------------------------------------------------------------------
void layer::__debug_active_field()
{
    int dim = (m_currentField == VELOCITY_FIELD ? 2 : 1);
    glm::vec4 buffer[m_activeField->fieldSize()];
    m_activeField->readFieldData((void*)&buffer[0]);
    glm::ivec2 sz = m_activeField->getSize();
    printf("---- __debug_active_field() : %s (dim %d) ----\n", field_ID(m_currentField), dim);
    for (int y = 0; y < sz.y; y++)
    {
        for (int x = 0; x < sz.x; x++)
        {
            glm::vec4 v = buffer[y * sz.x + x];
            printf("(%d, %d) = [ ", x, y);
            for (int i = 0; i < dim; i++)
                printf("%.2f ", v[i]);
            printf("], \t");
        }
        printf("\n");
    }
}


