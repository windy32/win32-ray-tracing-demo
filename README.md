# win32-ray-tracing-demo

This is a demo program for ray tracing written in C++, inspired by:

  * The [JavaScript ray tracing tutorial](http://www.cnblogs.com/miloyip/archive/2010/03/29/1698953.html) by Milo Yip (in Chinese)
  * The [ray tracing blog series](http://www.flipcode.com/archives/Raytracing_Topics_Techniques-Part_1_Introduction.shtml) by Jacco Bikker (in English)
  * The [smallpt project](http://www.kevinbeason.com/smallpt/) by Kevin Beason

The program has a simple graphics user interface based on win32 API. However, it is not difficult to be ported to other operating systems or frameworks (e.g., wxWidgets, Qt). Most of the source code is platform independent, except for MainWindow.cpp.

The files are organized in a Visual Studio 2012 solution, and the executable binary requires the Visual C++ 2012 redistributable library, which can downloaded [here](http://www.microsoft.com/en-us/download/details.aspx?id=30679).

This demo supports some basic geometris, including:

  * Infinite plane
  * Sphere
  * Triangle

It also supports the "Tunnel" geometry, which is made up of a number of triangle facets.

Materials supported by the program include:

  * SolidColorMaterial (can be reflective, refractive, or diffusive, with only one color)
  * RandomColorMaterial (for debugging)
  * CheckMaterial (white and black checkerboard)
  * RadianceCheckerMaterial (like checker material, but the white blocks can emit light)
  * PhongMaterial (a material used to render plastic balls)

The demo also supports some spacial subdivision schemes, including:

  * The regular grid
  * The k-d tree

However, in the first release, spacial subdivision can only be used for tunnel acceleration, which would be fixed in later versions.

**Note: This project was initially intended to be the experiment platform for a triangle intersection acceleration algorithm for tunnel models that the author worked on. So be sure to check out the first revision with source code (rev. 6) from the repository when you want to review the experiment results.**

**Note: Do not debug the program! Debugging in the release mode can also significantly affect performance. Please use "start (without debugging)" (Ctrl + F5) in Visual Studio. Double clicking on the exe also works.**

##### Screenshots #####

<img src='https://github.com/windy32/win32-ray-tracing-demo/blob/master/web/screenshot-1.png' />

**Preset scene 1: checker ground and a ball (there's no light in the scene, however the ground emits light)**

<img src='https://github.com/windy32/win32-ray-tracing-demo/blob/master/web/screenshot-2.png' />

**Preset scene 2: smallpt**

<img src='https://github.com/windy32/win32-ray-tracing-demo/blob/master/web/screenshot-3.png' />

**Preset scene 3: smallpt (stl ball)**

<img src='https://github.com/windy32/win32-ray-tracing-demo/blob/master/web/screenshot-4.png' />

**Preset scene 4: a short and wide tunnel**

<img src='https://github.com/windy32/win32-ray-tracing-demo/blob/master/web/screenshot-5.png' />

**Preset scene 5: a long and narrow tunnel**

##### Information for Paper Reviewers #####

There is only one table in the paper about the performance of different algorithms, but actually a number of experiments with different parameters have been performed. They are not listed in the paper because there was no space.

The table in the paper is like this:

| Algorithm                | Preprocess (seconds)   | Render (seconds)   |
| ------------------------ | ---------------------- | ------------------ |
| Linear                   | 0.000                  | 70.939             |
| Regular grid (400x400x5) | 0.055                  | 0.197              |
| Flat grid (400x400x400)  | 0.892                  | 0.276              |
| K-d tree                 | 0.701                  | 0.256              |
| K-d tree (SAH)           | 7.075                  | 0.139              |
| Convex                   | 0.672                  | 0.061              |

**Table I: The table in the paper**

These times are obtained when the cross section of the tunnel is tessellated into 150 line segments. As mentioned in the table, the grid is cut into 400 pieces. It may be interesting to know how the size of the cells in the grid can affect the time, and here's the comparison data:

| Algorithm                | Preprocess (seconds) | Render (seconds) |
| ------------------------ | -------------------- | ---------------- |
| Regular grid (50x50x1)   | 0.013                | 1.637            |
| Regular grid (80x80x2)   | 0.012                | 0.404            |
| Regular grid (100x100x2) | 0.018                | 0.321            |
| Regular grid (200x200x3) | 0.023                | 0.382            |
| Regular grid (400x400x5) | 0.055                | 0.197            |
| Regular grid (600x600x7) | 0.108                | 0.129            |
| Regular grid (800x800x9) | 0.176                | 0.131            |
| Flat grid (50x50x50)     | 0.026                | 0.292            |
| Flat grid (80x80x80)     | 0.045                | 0.222            |
| Flat grid (100x100x100)  | 0.066                | 0.238            |
| Flat grid (200x200x200)  | 0.180                | 0.201            |
| Flat grid (400x400x400)  | 0.892                | 0.276            |
| Flat grid (600x600x600)  | -                    | -                |
| Flat grid (800x800x800)  | -                    | -                |

**Table II: Comparison between different grid settings**

Note that when the size of the flat grid is 600x600x600, the system failed to allocate so much memory (216,000,000 std::vector objects). Thus in the paper, 400 is used as the size of the grid, which is the largest value that supports both regular grid and flat grid.

There're two parameters related to the construction of the k-d tree which are not discussed in the paper:

 * The leaf node object threshold
 * The maximum tree depth

For example, if the number of geometries in a node is <= 8, or the tree depth is > 18, then the node is regarded as a leaf node. This is actually the setting used in the paper. A comparison between different settings is available in the table below:

| Object threshold | Max depth | Preprocess (seconds) | Render (seconds) | Average leaf size |
| ---------------- | --------- | -------------------- | ---------------- | ----------------- |
| 4  | 10  | 0.211  | 0.359  | 47  |
| 4  | 11  | 0.221  | 0.283  | 32  |
| 4  | 12  | 0.266  | 0.293  | 24  |
| 4  | 13  | 0.334  | 0.261  | 14  |
| 4  | 14  | 0.403  | 0.244  | 11  |
| 4  | 15  | 0.531  | 0.277  | 10  |
| 4  | 16  | 0.660  | 0.248  | 6   |
| 4  | 17  | 0.993  | 0.277  | 6   |
| 4  | 18  | 1.219  | 0.287  | 6   |
| 4  | 19  | 1.905  | 0.289  | 5   |
| 4  | 20  | 2.542  | 0.309  | 5   |

**Table III: Comparison between different k-d tree settings (threshold = 4)**

| Object threshold | Max depth | Preprocess (seconds) | Render (seconds) | Average leaf size |
| ---------------- | --------- | -------------------- | ---------------- | ----------------- |
| 4  | 10  | 1.794  | 0.640  | 161  |
| 4  | 11  | 1.711  | 0.328  | 112  |
| 4  | 12  | 2.290  | 0.211  | 76   |
| 4  | 13  | 2.872  | 0.178  | 52   |
| 4  | 14  | 2.748  | 0.139  | 36   |
| 4  | 15  | 3.337  | 0.123  | 25   |
| 4  | 16  | 3.988  | 0.116  | 18   |
| 4  | 17  | 5.207  | 0.109  | 13   |
| 4  | 18  | 6.628  | 0.123  | 9    |
| 4  | 19  | 7.577  | 0.110  | 7    |
| 4  | 20  | 9.882  | 0.101  | 6    |

**Table IV: Comparison between different k-d tree (SAH) settings (threshold = 4)**

It is clear that the average leaf size of the k-d tree with SAH converges more slowly than the simple balanced k-d tree. This is easy to understand because the under SAH strategy more split planes are used to separate objects from empty space. Thus the termination depth should be large when SAH is enabled. A termination depth of 16 is enough for a standard balanced k-d tree. However when SAH is enabled, the depth should be 18 - 20.

Now we increase the threshold value to 8. The results are listed below.

| Object threshold | Max depth | Preprocess (seconds) | Render (seconds) | Average leaf size |
| ---------------- | --------- | -------------------- | ---------------- | ----------------- |
| 8  | 10  | 0.203  | 0.345  | 47  |
| 8  | 11  | 0.227  | 0.290  | 32  |
| 8  | 12  | 0.284  | 0.307  | 24  |
| 8  | 13  | 0.322  | 0.256  | 14  |
| 8  | 14  | 0.381  | 0.238  | 12  |
| 8  | 15  | 0.482  | 0.263  | 11  |
| 8  | 16  | 0.637  | 0.253  | 8   |
| 8  | 17  | 0.635  | 0.249  | 8   |
| 8  | 18  | 0.701  | 0.256  | 8   |
| 8  | 19  | 0.828  | 0.248  | 7   |
| 8  | 20  | 0.825  | 0.247  | 7   |

**Table V: Comparison between different k-d tree settings (threshold = 8)**

| Object threshold | Max depth | Preprocess (seconds) | Render (seconds) | Average leaf size |
| ---------------- | --------- | -------------------- | ---------------- | ----------------- |
| 8  | 10  | 1.786  | 0.638  | 163  |
| 8  | 11  | 2.090  | 0.368  | 114  |
| 8  | 12  | 2.356  | 0.229  | 79   |
| 8  | 13  | 2.548  | 0.176  | 57   |
| 8  | 14  | 3.275  | 0.160  | 41   |
| 8  | 15  | 3.866  | 0.153  | 29   |
| 8  | 16  | 4.687  | 0.130  | 20   |
| 8  | 17  | 5.376  | 0.133  | 14   |
| 8  | 18  | 7.075  | 0.139  | 11   |
| 8  | 19  | 8.232  | 0.143  | 8    |
| 8  | 20  | 8.270  | 0.104  | 7    |

**Table VI: Comparison between different k-d tree (SAH) settings (threshold = 8)**

As you can see, the best value is still about 16 and 18 - 20. The difference is that when the threshood is increased, unnecessary splittings are avoided. The construction time is greatly decreased with a high termination depth.

Thus in the paper, parameters (8, 18) are used as a balanced choice.
