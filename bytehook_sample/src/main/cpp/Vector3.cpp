//
// Created by jichong on 2023/7/17.
//

#include "Vector3.h"
#include <sstream>

Vector3::Vector3(): m_x(0), m_y(0), m_z(0) {

}

Vector3::Vector3(float x, float y, float z)
    :m_x(x)
    ,m_y(y)
    ,m_z(z) {

}

void Vector3::Set(float x, float y, float z) {
    m_x = x;
    m_y = y;
    m_z = z;
}

std::string Vector3::toString() {
    std::stringstream ss;
    ss << m_x << ":" << m_y << ":" << m_z;
    return ss.str();
}



