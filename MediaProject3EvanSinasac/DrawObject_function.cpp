#include "cMesh.h"
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp> 
// glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/type_ptr.hpp> // glm::value_ptr

#include "GLCommon.h"       // Gives us glad.h
#include "cVAOManager.h"


void GLFW_window_size_callback(GLFWwindow* window, int width, int height)
{
    // TODO: GLFW_window_size_callback()

    return;
}


void DrawObject(cMesh* pCurrentMesh, glm::mat4 matModel,
    GLint matModel_Location,
    GLint matModelInverseTranspose_Location,
    GLuint program,
    cVAOManager* pVAOManager)
{

    // Alpha Transparency
    glEnable(GL_BLEND);
    // Basic "alpha transparency
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Get the uniform (should be outside of the draw call)
    GLint wholeObjectAlphaTransparency_LocID = glGetUniformLocation(program, "wholeObjectAlphaTransparency");
    //Set this value here
    glUniform1f(wholeObjectAlphaTransparency_LocID, pCurrentMesh->alphaTransparency);

    // *****************************************************
    // Translate or "move" the object somewhere
    glm::mat4 matTranslate = glm::translate(glm::mat4(1.0f),
        pCurrentMesh->positionXYZ);
    //matModel = matModel * matTranslate;
    // *****************************************************


    // *****************************************************
    // Rotation around the Z axis
    glm::mat4 rotateZ = glm::rotate(glm::mat4(1.0f),
        pCurrentMesh->orientationXYZ.z,//(float)glfwGetTime(),
        glm::vec3(0.0f, 0.0f, 1.0f));
    //matModel = matModel * rotateZ;
    // *****************************************************

    // *****************************************************
    // Rotation around the Y axis
    glm::mat4 rotateY = glm::rotate(glm::mat4(1.0f),
        pCurrentMesh->orientationXYZ.y,
        glm::vec3(0.0f, 1.0f, 0.0f));
    //matModel = matModel * rotateY;
    // *****************************************************

    // *****************************************************
    // Rotation around the X axis
    glm::mat4 rotateX = glm::rotate(glm::mat4(1.0f),
        pCurrentMesh->orientationXYZ.x,
        glm::vec3(1.0f, 0.0f, 0.0f));
    //matModel = matModel * rotateX;
    // *****************************************************


    // *****************************************************
    // Scale the model
    glm::mat4 matScale = glm::scale(glm::mat4(1.0f),
        glm::vec3(pCurrentMesh->scale,  // Scale in X
            pCurrentMesh->scale,  // Scale in Y
            pCurrentMesh->scale));// Scale in Z
//matModel = matModel * matScale;
// *****************************************************

// *****************************************************
    matModel = matModel * matTranslate;
    matModel = matModel * rotateZ;
    matModel = matModel * rotateY;
    matModel = matModel * rotateX;
    matModel = matModel * matScale;     // <-- mathematically, this is 1st

    // Now the matModel ("Model" or "World") matrix
    //  represents ALL the transformations we want, in ONE matrix.

    // Moved view and projection ("v" and "p") to outside draw scene loop.
    // (because they are the same for all objects)

    glUniformMatrix4fv(matModel_Location, 1, GL_FALSE, glm::value_ptr(matModel));


    // Inverse transpose of the model matrix
    // (Used to calculate the normal location in vertex space, using only rotation)
    glm::mat4 matInvTransposeModel = glm::inverse(glm::transpose(matModel));
    glUniformMatrix4fv(matModelInverseTranspose_Location, 1, GL_FALSE, glm::value_ptr(matInvTransposeModel));


    // Copy the whole object colour information to the sahder               

    // This is used for wireframe or whole object colour. 
    // If bUseDebugColour is TRUE, then the fragment colour is "objectDebugColour".
    GLint bUseDebugColour_Location = glGetUniformLocation(program, "bUseDebugColour");
    GLint objectDebugColour_Location = glGetUniformLocation(program, "objectDebugColour");

    // If true, then the lighting contribution is NOT used. 
    // This is useful for wireframe object
    GLint bDontLightObject_Location = glGetUniformLocation(program, "bDontLightObject");

    // The "whole object" colour (diffuse and specular)
    GLint wholeObjectDiffuseColour_Location = glGetUniformLocation(program, "wholeObjectDiffuseColour");
    GLint bUseWholeObjectDiffuseColour_Location = glGetUniformLocation(program, "bUseWholeObjectDiffuseColour");
    GLint wholeObjectSpecularColour_Location = glGetUniformLocation(program, "wholeObjectSpecularColour");

    if (pCurrentMesh->bUseWholeObjectDiffuseColour)
    {
        glUniform1f(bUseWholeObjectDiffuseColour_Location, (float)GL_TRUE);
        glUniform4f(wholeObjectDiffuseColour_Location,
            pCurrentMesh->wholeObjectDiffuseRGBA.r,
            pCurrentMesh->wholeObjectDiffuseRGBA.g,
            pCurrentMesh->wholeObjectDiffuseRGBA.b,
            pCurrentMesh->wholeObjectDiffuseRGBA.a);
    }
    else
    {
        glUniform1f(bUseWholeObjectDiffuseColour_Location, (float)GL_FALSE);
    }

    glUniform4f(wholeObjectSpecularColour_Location,
        pCurrentMesh->wholeObjectSpecularRGB.r,
        pCurrentMesh->wholeObjectSpecularRGB.g,
        pCurrentMesh->wholeObjectSpecularRGB.b,
        pCurrentMesh->wholeObjectShininess_SpecPower);


    // See if mesh is wanting the vertex colour override (HACK) to be used?
    if (pCurrentMesh->bUseObjectDebugColour)
    {
        // Override the colour...
        glUniform1f(bUseDebugColour_Location, (float)GL_TRUE);
        glUniform4f(objectDebugColour_Location,
            pCurrentMesh->objectDebugColourRGBA.r,
            pCurrentMesh->objectDebugColourRGBA.g,
            pCurrentMesh->objectDebugColourRGBA.b,
            pCurrentMesh->objectDebugColourRGBA.a);
    }
    else
    {
        // DON'T override the colour
        glUniform1f(bUseDebugColour_Location, (float)GL_FALSE);
    }


    // See if mesh is wanting the vertex colour override (HACK) to be used?
    if (pCurrentMesh->bDontLight)
    {
        // Override the colour...
        glUniform1f(bDontLightObject_Location, (float)GL_TRUE);
    }
    else
    {
        // DON'T override the colour
        glUniform1f(bDontLightObject_Location, (float)GL_FALSE);
    }


    // Wireframe
    if (pCurrentMesh->bIsWireframe)                // GL_POINT, GL_LINE, and GL_FILL)
    {
        // Draw everything with only lines
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }




    // glPointSize(20.0f); sometimes this works... Hey, it's not my fault!



    sModelDrawInfo modelInfo;
    //        if (gVAOManager.FindDrawInfoByModelName("bun_zipper_res2 (justXYZ).ply", modelInfo))
    //        if (gVAOManager.FindDrawInfoByModelName("Assembled_ISS.ply", modelInfo))

    if (pVAOManager->FindDrawInfoByModelName(pCurrentMesh->meshName, modelInfo))
    {
        glBindVertexArray(modelInfo.VAO_ID);

        glDrawElements(GL_TRIANGLES,
            modelInfo.numberOfIndices,
            GL_UNSIGNED_INT,
            (void*)0);

        glBindVertexArray(0);
    }


    return;
}
