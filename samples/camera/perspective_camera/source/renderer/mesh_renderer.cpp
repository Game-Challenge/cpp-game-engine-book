//
// Created by captainchen on 2021/5/11.
//

#include "mesh_renderer.h"
#include <glad/gl.h>
#include <rttr/registration>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform2.hpp>
#include <glm/gtx/euler_angles.hpp>
#include "material.h"
#include "mesh_filter.h"
#include "texture2d.h"
#include "technique.h"
#include "pass.h"
#include "shader.h"
#include "component/game_object.h"
#include "component/transform.h"

using namespace rttr;
RTTR_REGISTRATION
{
registration::class_<MeshRenderer>("MeshRenderer")
.constructor<>()(rttr::policy::ctor::as_raw_ptr);
}

MeshRenderer::MeshRenderer():vertex_buffer_object(-1),element_buffer_object(-1) {

}

MeshRenderer::~MeshRenderer() {

}

void MeshRenderer::SetMaterial(Material* material) {
    material_=material;
}


void MeshRenderer::Render() {
    //主动获取 Transform 组件，计算mvp。
    auto component_transform=game_object()->GetComponent("Transform");
    auto transform=dynamic_cast<Transform*>(component_transform);
    if(!transform){
        return;
    }
    glm::mat4 trans = glm::translate(transform->position());
    auto rotation=transform->rotation();
    glm::mat4 eulerAngleYXZ = glm::eulerAngleYXZ(glm::radians(rotation.y), glm::radians(rotation.x), glm::radians(rotation.z));
    glm::mat4 scale = glm::scale(transform->scale()); //缩放;
    glm::mat4 model = trans*scale*eulerAngleYXZ;
    glm::mat4 mvp=projection_*view_*model;

    //主动获取 MeshFilter 组件
    auto component_meshfilter=game_object()->GetComponent("MeshFilter");
    auto mesh_filter=dynamic_cast<MeshFilter*>(component_meshfilter);
    if(!mesh_filter){
        return;
    }
    if (vertex_buffer_object==-1){
        //在GPU上创建缓冲区对象
        glGenBuffers(1,&vertex_buffer_object);
        //将缓冲区对象指定为顶点缓冲区对象
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
        //上传顶点数据到缓冲区对象
        glBufferData(GL_ARRAY_BUFFER, mesh_filter->mesh()->vertex_num_ * sizeof(MeshFilter::Vertex), mesh_filter->mesh()->vertex_data_, GL_STATIC_DRAW);
    }
    if (vertex_buffer_object==-1){
        return;
    }
    if(element_buffer_object==-1){
        //在GPU上创建缓冲区对象
        glGenBuffers(1,&element_buffer_object);
        //将缓冲区对象指定为顶点索引缓冲区对象
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer_object);
        //上传顶点索引数据到缓冲区对象
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh_filter->mesh()->vertex_index_num_ * sizeof(unsigned short), mesh_filter->mesh()->vertex_index_data_, GL_STATIC_DRAW);
    }
    if(element_buffer_object==-1){
        return;
    }

    //遍历Pass节点，获取`Shader`的`gl_program_id`，指定为目标Shader程序。
    std::vector<Pass*>& pass_vec=material_->technique_active()->pass_vec();
    for (int i = 0; i < pass_vec.size(); ++i) {
        Pass* pass=pass_vec[i];
        GLuint gl_program_id=pass->shader()->gl_program_id();
        glUseProgram(gl_program_id);
        glEnable(GL_DEPTH_TEST);

        GLint mvp_location = glGetUniformLocation(gl_program_id, "u_mvp");
        GLint vpos_location = glGetAttribLocation(gl_program_id, "a_pos");
        GLint vcol_location = glGetAttribLocation(gl_program_id, "a_color");
        GLint a_uv_location = glGetAttribLocation(gl_program_id, "a_uv");

        glEnableVertexAttribArray(vpos_location);
        glEnableVertexAttribArray(vcol_location);
        glEnableVertexAttribArray(a_uv_location);

        //指定当前使用的VBO
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
        //将Shader变量(a_pos)和顶点坐标VBO句柄进行关联，最后的0表示数据偏移量。
        glVertexAttribPointer(vpos_location, 3, GL_FLOAT, false, sizeof(MeshFilter::Vertex), 0);
        //启用顶点Shader属性(a_color)，指定与顶点颜色数据进行关联
        glVertexAttribPointer(vcol_location, 4, GL_FLOAT, false, sizeof(MeshFilter::Vertex), (void*)(sizeof(float)*3));
        //将Shader变量(a_uv)和顶点UV坐标VBO句柄进行关联，最后的0表示数据偏移量。
        glVertexAttribPointer(a_uv_location, 2, GL_FLOAT, false, sizeof(MeshFilter::Vertex), (void*)(sizeof(float)*(3+4)));

        //上传mvp矩阵
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, &mvp[0][0]);

        //从Pass节点拿到保存的Texture
        std::vector<std::pair<std::string,Texture2D*>> textures=pass->textures();
        for (int texture_index = 0; texture_index < textures.size(); ++texture_index) {
            GLint u_texture_location= glGetUniformLocation(gl_program_id, textures[texture_index].first.c_str());
            //激活纹理单元0
            glActiveTexture(GL_TEXTURE0+texture_index);
            //将加载的图片纹理句柄，绑定到纹理单元0的Texture2D上。
            glBindTexture(GL_TEXTURE_2D,textures[texture_index].second->gl_texture_id());
            //设置Shader程序从纹理单元0读取颜色数据
            glUniform1i(u_texture_location,GL_TEXTURE0);
        }

        //指定当前使用的顶点索引缓冲区对象
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer_object);
        glDrawElements(GL_TRIANGLES, mesh_filter->mesh()->vertex_index_num_, GL_UNSIGNED_SHORT, 0);//使用顶点索引进行绘制，最后的0表示数据偏移量。
        glUseProgram(-1);
    }
}
