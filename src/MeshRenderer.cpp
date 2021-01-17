#include "MeshRenderer.hpp"

MeshRenderer::MeshRenderer(unsigned int shaderID, unsigned int depthShaderID, Mesh& mesh)
    : VertexArrayID(0), vertexbuffer(0), uvbuffer(0), normalbuffer(0), elementbuffer(0)
{
    tridimodel = mesh;
    model = glm::mat4(1.0f);

    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);
    
    programID = shaderID;
    depthProgramID = depthShaderID;

    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, tridimodel.indexed_vertices.size() * sizeof(glm::vec3), &tridimodel.indexed_vertices[0], GL_STATIC_DRAW);

    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, tridimodel.indexed_uvs.size() * sizeof(glm::vec2), &tridimodel.indexed_uvs[0], GL_STATIC_DRAW);

    glGenBuffers(1, &normalbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
    glBufferData(GL_ARRAY_BUFFER, tridimodel.indexed_normals.size() * sizeof(glm::vec3), &tridimodel.indexed_normals[0], GL_STATIC_DRAW);

    // Generate a buffer for the indices as well
    glGenBuffers(1, &elementbuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, tridimodel.indices.size() * sizeof(unsigned short), &tridimodel.indices[0] , GL_STATIC_DRAW);

    // Get a handle for our "LightPosition" uniform
    glUseProgram(programID);

    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glVertexAttribPointer(
            0,                  // attribute
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            nullptr            // array buffer offset
    );

    // 2nd attribute buffer : UVs
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glVertexAttribPointer(
            1,                                // attribute
            2,                                // size
            GL_FLOAT,                         // type
            GL_FALSE,                         // normalized?
            0,                                // stride
            nullptr                          // array buffer offset
    );

    // 3rd attribute buffer : normals
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
    glVertexAttribPointer(
            2,                                // attribute
            3,                                // size
            GL_FLOAT,                         // type
            GL_FALSE,                         // normalized?
            0,                                // stride
            nullptr                          // array buffer offset
    );

    // Index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

}

MeshRenderer::~MeshRenderer() = default;


void MeshRenderer::draw(unsigned int ShaderID, Camera & camera, LightSource & light)
{
    //createDepthMapFromLight(light);
    //createDepthMapFromEye(camera);

    // Use our shader
    glUseProgram(ShaderID);
    // Model matrix : an identity matrix (model will be at the origin)
    glm::mat4 ModelMatrix      = glm::mat4(1.0f);
    float decalageX = -(tridimodel.bounding_box.xpos.y - ((tridimodel.bounding_box.xpos.y + abs(tridimodel.bounding_box.xpos.x)) / 2.0f) ); 
    float decalageY = -(tridimodel.bounding_box.ypos.y - ((tridimodel.bounding_box.ypos.y + abs(tridimodel.bounding_box.ypos.x)) / 2.0f) ); 
    float decalageZ = -(tridimodel.bounding_box.zpos.y - ((tridimodel.bounding_box.zpos.y + abs(tridimodel.bounding_box.zpos.x)) / 2.0f) ); 
    ModelMatrix = glm::scale(ModelMatrix, glm::vec3(1.0/(tridimodel.bounding_box.ypos.y+decalageY)));
    ModelMatrix = glm::translate(ModelMatrix, glm::vec3(decalageX + 0.5, decalageY, decalageZ -0.5) );

    // Send our transformation to the currently bound shader,
    glUniformMatrix4fv(glGetUniformLocation(ShaderID, "projection"), 1, GL_FALSE, &camera.projection[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(ShaderID, "modelGlobal"), 1, GL_FALSE, &ModelMatrix[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(ShaderID, "view"), 1, GL_FALSE, &camera.GetViewMatrix()[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(ShaderID, "model"), 1, GL_FALSE, &model[0][0]);

    //glm::vec3 lightPos = glm::vec3(0.0f+cos((double)lightPlacement)*4.0f,4.0f,0.0f+sin((double)lightPlacement)*4.0f);
    glUniform3f(glGetUniformLocation(ShaderID, "lightPos"), light.position.x, light.position.y, light.position.z);
    glUniform3f(glGetUniformLocation(ShaderID, "viewPos"), camera.Position.x, camera.Position.y, camera.Position.z);
    glUniform3f(glGetUniformLocation(ShaderID, "lightColor"), light.color.x, light.color.y, light.color.z);
    glUniform3f(glGetUniformLocation(ShaderID, "objectColor"), color.x, color.y, color.z);

    glBindVertexArray(VertexArrayID);
    // Draw the triangles !
    glDrawElements(
                GL_TRIANGLES,      // mode
                tridimodel.indices.size(),    // count
                GL_UNSIGNED_SHORT,   // type
                nullptr           // element array buffer offset
                );

    /*glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);*/

    //glUseProgram(depthProgramID);
    //glUniformMatrix4fv(glGetUniformLocation(depthProgramID, "model"), 1, GL_FALSE, &model[0][0]);
}

void MeshRenderer::updateBuffers()
{
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, tridimodel.indexed_vertices.size() * sizeof(glm::vec3), &tridimodel.indexed_vertices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, tridimodel.indexed_uvs.size() * sizeof(glm::vec2), &tridimodel.indexed_uvs[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
    glBufferData(GL_ARRAY_BUFFER, tridimodel.indexed_normals.size() * sizeof(glm::vec3), &tridimodel.indexed_normals[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, tridimodel.indices.size() * sizeof(unsigned short), &tridimodel.indices[0] , GL_STATIC_DRAW);
}

void MeshRenderer::cleanUp()
{
    // Cleanup VBO and shader
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &uvbuffer);
    glDeleteBuffers(1, &normalbuffer);
    glDeleteBuffers(1, &elementbuffer);

    //***********************************************//
    glDeleteProgram(programID);
    glDeleteProgram(depthProgramID);
    glDeleteVertexArrays(1, &VertexArrayID);
}
/*
void MeshRenderer::createDepthMapFromLight(LightSource &light) const {
    // Use our shader
    glUseProgram(depthProgramID);

    // Model matrix : an identity matrix (model will be at the origin)
    glm::mat4 ModelMatrix      = glm::mat4(1.0f);
    float decalageX = -(tridimodel.bounding_box.xpos.y - ((tridimodel.bounding_box.xpos.y + abs(tridimodel.bounding_box.xpos.x)) / 2.0f) );
    float decalageY = -(tridimodel.bounding_box.ypos.y - ((tridimodel.bounding_box.ypos.y + abs(tridimodel.bounding_box.ypos.x)) / 2.0f) );
    float decalageZ = -(tridimodel.bounding_box.zpos.y - ((tridimodel.bounding_box.zpos.y + abs(tridimodel.bounding_box.zpos.x)) / 2.0f) );
    ModelMatrix = glm::scale(ModelMatrix, glm::vec3(1.0/(tridimodel.bounding_box.ypos.y+decalageY)));
    ModelMatrix = glm::translate(ModelMatrix, glm::vec3(decalageX + 0.5, decalageY, decalageZ -0.5) );

    // Send our transformation to the currently bound shader,
    glUniformMatrix4fv(glGetUniformLocation(depthProgramID, "modelGlobal"), 1, GL_FALSE, &ModelMatrix[0][0]);
    // Send our transformation to the currently bound shader,
    glUniformMatrix4fv(glGetUniformLocation(depthProgramID, "lightSpaceMatrix"), 1, GL_FALSE, &light.getLightSpaceMatrix()[0][0]);


    glViewport(0, 0, 1024, 1024);
    glBindFramebuffer(GL_FRAMEBUFFER, light.getDepthMapFBO());
    glClear(GL_DEPTH_BUFFER_BIT);

    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glVertexAttribPointer(
            0,                  // attribute
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            nullptr            // array buffer offset
    );
    // Index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

    // Draw the triangles !
    glDrawElements(
            GL_TRIANGLES,      // mode
            tridimodel.indices.size(),    // count
            GL_UNSIGNED_SHORT,   // type
            nullptr           // element array buffer offset
    );

    glDisableVertexAttribArray(0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // reset viewport
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void MeshRenderer::createDepthMapFromEye(Camera &eye) {
    // Use our shader
    glUseProgram(depthProgramID);

    // Model matrix : an identity matrix (model will be at the origin)
    glm::mat4 ModelMatrix      = glm::mat4(1.0f);
    float decalageX = -(tridimodel.bounding_box.xpos.y - ((tridimodel.bounding_box.xpos.y + abs(tridimodel.bounding_box.xpos.x)) / 2.0f) );
    float decalageY = -(tridimodel.bounding_box.ypos.y - ((tridimodel.bounding_box.ypos.y + abs(tridimodel.bounding_box.ypos.x)) / 2.0f) );
    float decalageZ = -(tridimodel.bounding_box.zpos.y - ((tridimodel.bounding_box.zpos.y + abs(tridimodel.bounding_box.zpos.x)) / 2.0f) );
    ModelMatrix = glm::scale(ModelMatrix, glm::vec3(1.0/(tridimodel.bounding_box.ypos.y+decalageY)));
    ModelMatrix = glm::translate(ModelMatrix, glm::vec3(decalageX + 0.5, decalageY, decalageZ -0.5) );

    glm::mat4 projCam = eye.projection;
    eye.setProjection(glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.01f, 100.0f));
    // Send our transformation to the currently bound shader,
    glUniformMatrix4fv(glGetUniformLocation(depthProgramID, "modelGlobal"), 1, GL_FALSE, &ModelMatrix[0][0]);
    // Send our transformation to the currently bound shader,
    glUniformMatrix4fv(glGetUniformLocation(depthProgramID, "lightSpaceMatrix"), 1, GL_FALSE, &(eye.projection * glm::lookAt(eye.Position, eye.Target, eye.Up))[0][0]);


    glViewport(0, 0, 1024, 1024);
    glBindFramebuffer(GL_FRAMEBUFFER, eye.depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glVertexAttribPointer(
            0,                  // attribute
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            nullptr            // array buffer offset
    );
    // Index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

    // Draw the triangles !
    glDrawElements(
            GL_TRIANGLES,      // mode
            tridimodel.indices.size(),    // count
            GL_UNSIGNED_SHORT,   // type
            nullptr           // element array buffer offset
    );

    glDisableVertexAttribArray(0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // reset viewport
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    eye.setProjection(projCam);
}
*/