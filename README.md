# TinyRayTracingRender


小型光线追踪渲染器
，渲染经典Cornell Box场景

Build需要CMake+OpenCV+OpenMP(可选)

使用:

●光线追踪

●蒙特卡洛方法

●2x抗锯齿

●均值滤波去噪，改善图像质量

●OpenMP并行计算加速（性能还是呵呵...）

ManyBalls@64samples

![](img/64manyballs.png)

各个采样数效果比较

![](img/compare.PNG)

256采样数结果

![](img/256spp.png)
TODO.

●添加界面

Reference:

[zhihu](https://zhuanlan.zhihu.com/p/29418992)

[CSDN](https://blog.csdn.net/qq_33274326/article/details/60779919)

[CSDN](https://blog.csdn.net/sunacmer/article/details/7451861)

[kevinbeason](http://www.kevinbeason.com/smallpt/)