//
// Created by jichong on 2023/7/17.
//

#ifndef VECTOR3_H
#define VECTOR3_H
#include <string>

#include <android/log.h>
//#define LOGD(format,...) __android_log_print(ANDROID_LOG_DEBUG,"Derek",format, __VA_ARGS__)


class Vector3 {
public:
    Vector3();
    Vector3(float x,float y,float z);
    void Set(float x,float y,float z);
    std::string toString();

private:
    float m_x,m_y,m_z;
};





#endif //VECTOR3_H
