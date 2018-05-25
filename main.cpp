#include <cmath>
#include <cstdlib>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <ctime>
using namespace std;
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
    double intersect(const Ray &r) const {
        Vec op = p - r.o;
        double t, eps = 1e-4, b = op.dot(r.d), det = b * b - op.dot(op) + rad * rad;
        if (det < 0) return 0; else det = sqrt(det);
        return (t = b - det) > eps ? t : ((t = b + det) > eps ? t : 0);
    }

};
//场景内的圆,墙面实际 用一个很大的圆模拟,参数依次是:半径,位置,发射光,颜色,材质类型
Sphere spheres[] = {
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

inline bool intersect(const Ray &r, double &t, int &id) {
    double n = sizeof(spheres) / sizeof(Sphere), d, inf = t = 1e20;
    for (int i = int(n); i--; )
        if ((d = spheres[i].intersect(r)) && d < t) {
            t = d;
            id = i;
        }
    return t < inf;
}

Vec radiance(const Ray &r, int depth, unsigned short *Xi) {
    double t;                                   // 相交的距离
    int id = 0;                                   // 相交圆的下标
    if (!intersect(r, t, id)) return Vec();     //背景色黑色
    const Sphere &obj = spheres[id];            //
    Vec x = r.o + r.d * t, n = (x - obj.p).norm(), nl = n.dot(r.d) < 0 ? n : n * -1, f = obj.c;
    double p = f.x > f.y && f.x > f.z ? f.x : f.y > f.z ? f.y : f.z;
    if (++depth > 5) if (erand48(Xi) < p) f = f * (1 / p); else return obj.e;
    if (obj.refl == DIFF) {
        double r1 = 2 * M_PI * erand48(Xi), r2 = erand48(Xi), r2s = sqrt(r2);
        Vec w = nl, u = ((fabs(w.x) > .1 ? Vec(0, 1) : Vec(1)) % w).norm(), v = w % u;
        Vec d = (u * cos(r1) * r2s + v * sin(r1) * r2s + w * sqrt(1 - r2)).norm();
        return obj.e + f.mult(radiance(Ray(x, d), depth, Xi));
    } else if (obj.refl == SPEC)
        return obj.e + f.mult(radiance(Ray(x, r.d - n * 2 * n.dot(r.d)), depth, Xi));
    Ray reflRay(x, r.d - n * 2 * n.dot(r.d));
    bool into = n.dot(nl) > 0;
    double nc = 1, nt = 1.5, nnt = into ? nc / nt : nt / nc, ddn = r.d.dot(nl), cos2t;
    if ((cos2t = 1 - nnt * nnt * (1 - ddn * ddn)) < 0)
        return obj.e + f.mult(radiance(reflRay, depth, Xi));
    Vec tdir = (r.d * nnt - n * ((into ? 1 : -1) * (ddn * nnt + sqrt(cos2t)))).norm();
    double a = nt - nc, b = nt + nc, R0 = a * a / (b * b), c = 1 - (into ? -ddn : tdir.dot(n));
    double Re = R0 + (1 - R0) * c * c * c * c * c, Tr = 1 - Re, P = .25 + .5 * Re, RP = Re / P, TP = Tr / (1 - P);
    return obj.e + f.mult(depth > 2 ? (erand48(Xi) < P ?
                                       radiance(reflRay, depth, Xi) * RP : radiance(Ray(x, tdir), depth, Xi) * TP) :
                          radiance(reflRay, depth, Xi) * Re + radiance(Ray(x, tdir), depth, Xi) * Tr);
}

int main(int argc, char *argv[]) {
	time_t startTime,curTime;
	time(&startTime);
    int w = 1024, h = 768;
	int samps = argc == 2 ? atoi(argv[1]) / 4 : 16;     //采样数
    Ray cam(Vec(50, 52, 295.6), Vec(0, -0.042612, -1).norm());     // cam pos, dir
    Vec cx = Vec(w * .5135 / h), cy = (cx % cam.d).norm() * .5135, r, *c = new Vec[w * h];
#pragma omp parallel for schedule(dynamic, 1) private(r)       // OpenMP
    for (int y = 0; y < h; y++) {
	    time(&curTime);
	    int diff=curTime-startTime;
	    float rate=1.0 * y / (h - 1);
        printf("\r渲染中(%d 采样数) 进度%5.2f%% 剩余时间%d秒", samps * 4, 100*rate,(int)(diff/rate*(1-rate)));
        for (unsigned short x = 0, Xi[3] = {0, 0, static_cast<unsigned short>(y * y * y)}; x < w; x++)
            for (int sy = 0, i = (h - y - 1) * w + x; sy < 2; sy++)
                for (int sx = 0; sx < 2; sx++, r = Vec()) {
                    for (int s = 0; s < samps; s++) {
                        double r1 = 2 * erand48(Xi), dx = r1 < 1 ? sqrt(r1) - 1 : 1 - sqrt(2 - r1);
                        double r2 = 2 * erand48(Xi), dy = r2 < 1 ? sqrt(r2) - 1 : 1 - sqrt(2 - r2);
                        Vec d = cx * (((sx + .5 + dx) / 2 + x) / w - .5) +
                                cy * (((sy + .5 + dy) / 2 + y) / h - .5) + cam.d;
                        r = r + radiance(Ray(cam.o + d * 140, d.norm()), 0, Xi) * (1. / samps);
                    }
                    c[i] = c[i] + Vec(clamp(r.x), clamp(r.y), clamp(r.z)) * .25;
                }
    }

    string filename = string("image@")+to_string(4*samps)+string("samples.png");
    cv::Mat image(h, w, CV_8UC3);
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            image.at<cv::Vec3b>(i, j)[0] = toInt(c[i*w+j].x);
            image.at<cv::Vec3b>(i, j)[1] = toInt(c[i*w+j].y);
            image.at<cv::Vec3b>(i, j)[2] = toInt(c[i*w+j].z);
        }
    }
	cv::Mat result;
//	cv::bilateralFilter(image,result,50,100,0);
	cv::medianBlur(image,result,3);
    cv::imwrite(filename, result);
}