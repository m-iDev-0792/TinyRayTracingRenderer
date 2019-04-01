#ifndef RENDERER_H
#define RENDERER_H

#include <QObject>
#include <cmath>
#include <QPixmap>
#include <QImage>
#include <QDebug>
#include "utility.h"
struct Vec {//定义向量类型
    double x, y, z;

    Vec(double xx = 0, double yy = 0, double zz = 0) {
        x = xx;
        y = yy;
        z = zz;
    }

    Vec operator +(const Vec &b) const {
        return Vec(x + b.x, y + b.y, z + b.z);
    }

    Vec operator -(const Vec &b) const {
        return Vec(x - b.x, y - b.y, z - b.z);
    }

    Vec operator *(double b) const {
        return Vec(x * b, y * b, z * b);
    }

    Vec mult(const Vec &b) const {
        return Vec(x * b.x, y * b.y, z * b.z);
    }                                                                          //向量分量相乘

    Vec &norm() {
        return *this = *this * (1 / sqrt(x * x + y * y + z * z));
    }                                                                             //单位化本向量

    double dot(const Vec &b) const {
        return x * b.x + y * b.y + z * b.z;
    }                                                                          //点乘,

    Vec operator %(Vec &b) {
        return Vec(y * b.z - z * b.y, z * b.x - x * b.z, x * b.y - y * b.x);
    }                                                                                                  //叉乘

};

struct Ray {
    Vec o, d;
    Ray(Vec o_, Vec d_) : o(o_), d(d_) {
    }

};

enum Refl_t {//材质定义
    DIFF, SPEC, REFR
};
struct Sphere {//定义圆数据结构
    double rad;
    Vec p, e, c;          // 位置,发射亮度,颜色
    Refl_t refl;          // 材质类型
    Sphere(double rad_, Vec p_, Vec e_, Vec c_, Refl_t refl_) :
        rad(rad_), p(p_), e(e_), c(c_), refl(refl_) {
    }

        //测试光线与圆是否相交,未相交返回0
    double intersect(Ray &r) {
        Vec op = p - r.o;
        double t, eps = 1e-4, b = op.dot(r.d), det = b * b - op.dot(op) + rad * rad;
        if (det < 0) return 0; else det = sqrt(det);
        return (t = b - det) > eps ? t : ((t = b + det) > eps ? t : 0);
    }

};


class Renderer : public QObject
{
  Q_OBJECT
public:
  explicit Renderer(QObject *parent = nullptr);
  QImage resultImg;
  //场景内的圆,墙面实际 用一个很大的圆模拟,参数依次是:半径,位置,发射光,颜色,材质类型
  Sphere spheres[12] = {
      Sphere(1e5, Vec(1e5 + 1, 40.8, 81.6), Vec(), Vec(.75, .25, .25), DIFF),            //左
      Sphere(1e5, Vec(-1e5 + 99, 40.8, 81.6), Vec(), Vec(.25, .25, .75), DIFF),            //右
      Sphere(1e5, Vec(50, 40.8, 1e5), Vec(), Vec(.75, .75, .75), DIFF),            //后
      Sphere(1e5, Vec(50, 40.8, -1e5 + 170), Vec(), Vec(), DIFF),            //前
      Sphere(1e5, Vec(50, 1e5, 81.6), Vec(), Vec(.75, .75, .75), DIFF),            //底部
      Sphere(1e5, Vec(50, -1e5 + 81.6, 81.6), Vec(), Vec(.75, .75, .75), DIFF),            //顶部
      Sphere(16.5, Vec(27, 16.5, 47), Vec(), Vec(1, 1, 1) * .999, SPEC),            //镜面球
      Sphere(16.5, Vec(50, 50, 47), Vec(), Vec(1, 1, 1) * .999, SPEC),            //镜面球
      Sphere(16.5, Vec(73, 16.5, 78), Vec(), Vec(1, 1, 1) * .999, REFR),            //玻璃球
      Sphere(8, Vec(27, 8, 90), Vec(), Vec(0.5, 0.5, 0.5), DIFF),            //漫反射
      Sphere(5, Vec(35, 5, 100), Vec(), Vec(0.125, 0.84, 0.553), DIFF),            //漫反射
      Sphere(600, Vec(50, 681.6 - .27, 81.6), Vec(12, 12, 12), Vec(), DIFF)             //光源
  };
  inline double clamp(double x) {
      return x < 0 ? 0 : x > 1 ? 1 : x;
  }

  inline int toInt(double x) {
      return int(pow(clamp(x), 1 / 2.2) * 255 + .5);
  }

  inline bool intersect(Ray &r, double &t, int &id) {
      double n = sizeof(spheres) / sizeof(Sphere), d, inf = t = 1e20;
      for (int i = int(n); i--; )
          if ((d = spheres[i].intersect(r)) && d < t) {
              t = d;
              id = i;
          }
      return t < inf;
  }
  Vec radiance(Ray &&r, int depth, unsigned short *Xi);
  Vec radiance(Ray &r, int depth, unsigned short *Xi);
signals:
  void newLineRendered(QImage image);
  void renderState(int progress,int remain);
public slots:
  void renderScene(int sample,int width,int height);
};

#endif // RENDERER_H
