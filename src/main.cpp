
#include <synapse/Synapse>
#include <synapse/SynapseMain.hpp>

#include "quad.h"
#include "field_fbo.h"
#include "arrows_2D.h"


using namespace Syn;

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

    Ref<FieldFBO> test_field = nullptr;
    Ref<Shader> test_shader = nullptr;
    void test_compute_field();
    bool test_done = false;
    Ref<Arrows2D> m_quiver = nullptr;
    
    bool m_doUpdate = false;
    void computeDivergence();
    glm::vec2 m_cellSize;
    float m_aspectRatio;
    float m_dx = 1.0f;

    // flags
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

    // glm::ivec2 dim = { 128, 128 };
    glm::ivec2 dim = Renderer::get().getViewport();
    m_velocity   = VectorField(dim, "velocity");
    m_divergence = ScalarField(dim, "divergence");

    // -- TESTS init -- //
    test_field = std::make_shared<FieldFBO>(ColorFormat::RGBA32F, dim, "test_field");
    test_shader = ShaderLibrary::load(FileName("../assets/shaders/global/stencil.vert"),
                                      FileName("../assets/shaders/test_vel.frag"));
    // -- end TESTS -- //


    m_cellSize = 1.0f / glm::vec2(m_vp.x, m_vp.y);
    m_aspectRatio = m_vp.y / (float)m_vp.x;

    m_doUpdate = true;

}

//---------------------------------------------------------------------------------------
void layer::computeDivergence()
{
    return;
    m_divergence->bind();
    m_divergenceShader->enable();
    m_velocity->bindTexture(0);
    m_divergenceShader->setUniform2fv("u_tx_size", m_cellSize);
    m_divergenceShader->setUniform1f("u_half_inv_dx", 0.5f / m_dx);

    Quad::render();

}

//---------------------------------------------------------------------------------------
void layer::test_compute_field()
{
    if (test_done)
        return;

    // bind framebuffer for computation
    test_field->bind();

    // enable correct shader
    test_shader->enable();
    test_shader->setUniform2fv("u_tx_size", m_cellSize);
    
    // compute (i.e. render)
    Quad::render();
    
    // create quiver for visualization
    m_quiver = std::make_shared<Arrows2D>(test_field, 128);

    test_done = true;
    
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

    if (m_doUpdate)
    {
        Quad::bind();
        computeDivergence();
        test_compute_field();
    }

    // -- END SIMULATION -- //

    m_renderBuffer->bind();
    if (m_wireframeMode)
        renderer.enableWireFrame();    
    renderer.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    // -- BEGINNING OF SCENE -- //

    if (test_done)
    {
        // if tests are completed, visualize the result in the render buffer
        m_vectorFieldShader->enable();
        test_field->bindTexture(0);
        // the value of the sampler2D is set to the corresponding texture slot (above)
        m_vectorFieldShader->setUniform1i("u_text_sampler", 0);

        Quad::render();
        
        if (m_quiver)
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
