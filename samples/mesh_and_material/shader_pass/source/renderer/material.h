//
// Created by captain on 2021/5/3.
//

#ifndef UNTITLED_MATERIAL_H
#define UNTITLED_MATERIAL_H

#include <string>
#include <vector>
#include <map>


class Shader;
class Texture2D;
class Technique;
class Material {
public:
    Material();
    ~Material();

    void Parse(std::string material_path);//加载Material文件并解析

    Technique* technique_active(){return technique_active_;}

private:
    std::vector<Technique*> technique_vec_;
    Technique* technique_active_;

    std::vector<std::pair<std::string,float*>> uniform_matrix4fv_vec;
    std::vector<std::pair<std::string,int>> uniform_1i_vec;
};


#endif //UNTITLED_MATERIAL_H
