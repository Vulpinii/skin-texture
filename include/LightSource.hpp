#ifndef LIGHTSOURCE_HPP
#define LIGHTSOURCE_HPP
#include "glm.hpp"
#include "Camera.hpp"
class LightSource
{
public:
    LightSource() = default;

    LightSource(glm::vec3 in_position)
        : position(in_position)
    {
        m_camera = Camera(in_position);
        glm::mat4 lightProjection, lightView;
        glm::mat4 lightSpaceMatrix;
        m_camera.setProjection(glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.01f, 100.0f));

        glGenFramebuffers(1, &m_camera.depthMapFBO);
    }

    void configureDepthMapTo(glm::vec3 target){
        m_camera.Target = target;
        m_camera.updateCameraVectorsFromFront();
        m_camera.configure_depthMap();
    }

    GLuint getDepthMapFBO(){return m_camera.depthMapFBO;}
    GLuint getDephtMapTexture(){return m_camera.depthMapTexture;}
    glm::mat4 getLightSpaceMatrix() {return m_camera.projection * glm::lookAt(m_camera.Position, m_camera.Target, m_camera.Up);}
    glm::vec3 position;
    glm::vec3 color;

private:
    Camera m_camera;
};

#endif //LIGHTSOURCE_HPP
