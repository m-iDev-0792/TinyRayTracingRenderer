#include "renderer.h"

Renderer::Renderer(QObject *parent) : QObject(parent),resultImg(480,360,QImage::Format_RGB888)
{

}
void Renderer::renderScene(int sample,int width,int height){
  resultImg=QImage(width,height,QImage::Format_RGB888);
  time_t startTime,curTime;
  time(&startTime);
  int w = width, h = height;
  int samps =sample;     //采样数
  Ray cam(Vec(50, 52, 295.6), Vec(0, -0.042612, -1).norm());     // cam pos, dir
  Vec cx = Vec(w * .5135 / h), cy = (cx % cam.d).norm() * .5135, r, *c = new Vec[w * h];
  //  uchar* data=new uchar[640*480*4];//resultImg.bits();
  uchar* data=resultImg.bits();
#pragma omp parallel for schedule(dynamic, 1) private(r)       // OpenMP
  for (int y = 0; y < h; y++) {
      time(&curTime);
      int diff=curTime-startTime;
      float rate=1.0 * y / (h - 1);
      emit renderState(100*rate,(diff/rate*(1-rate)));
//      qDebug()<<QString("渲染中(%1 采样数) 进度%2 剩余时间%3秒\n").arg(samps * 4).arg(100*rate).arg((int)(diff/rate*(1-rate)))<<endl;
      for (unsigned short x = 0, Xi[3] = {0, 0, static_cast<unsigned short>(y * y * y)}; x < w; x++){
          for (int sy = 0, i = (h - y - 1) * w + x; sy < 2; sy++){
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
              //a new pixel rendering is complete here
              data[3*i]=toInt(c[i].x);data[3*i+1]=toInt(c[i].y);data[3*i+2]=toInt(c[i].z);
            }
        }
      //a new line rendering is complete here
      if((y+1)%8==0){

          emit newLineRendered(resultImg);
        }
    }
  qDebug()<<"finished!"<<endl;
}
Vec Renderer::radiance(Ray &r, int depth, unsigned short *Xi) {
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
Vec Renderer::radiance(Ray &&r, int depth, unsigned short *Xi) {
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
