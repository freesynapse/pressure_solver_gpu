
#include "field_fbo.h"

FieldFBO::FieldFBO(const ColorFormat& _format, const glm::ivec2& _size, const std::string& _name)
{
    m_format = _format;
    m_pxFmt = getOpenGLPixelFormat(m_format);

    // m_colorAttachmentCount = _n_drawbuffers;
    m_colorAttachmentCount = 1;
    m_colorAttachmentID = new GLuint[m_colorAttachmentCount];
    // m_hasDepthAttachment = _use_depthbuffer;
    m_hasDepthAttachment = false;

    // m_name = (_name.compare("") != 0) ? _name : "field";
    m_name = _name;

    // create the framebuffer
    init(_size);

}

//---------------------------------------------------------------------------------------
void FieldFBO::readFieldData(void *_buffer)
{
    // inhibit clamping of values to [0..1]
    glClampColor(GL_CLAMP_READ_COLOR, GL_FALSE);
    
    // bind before reading
    this->bind();

    // read into buffer
    glReadPixels(0, 0, m_size.x, m_size.y, m_pxFmt.storageFormat, GL_FLOAT, _buffer);

}




