### About ###

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

<font color='#0068b7'><b>Note: This project was initially intended to be the experiment platform for a triangle intersection acceleration algorithm for tunnel models that the author worked on. So be sure to check out the first revision with source code (rev. 6) from the repository when you want to review the experiment results.</b></font>

<font color='#0068b7'><b>Note: Do not debug the program! Debugging in the release mode can also significantly affect performance. Please use "start (without debugging)" (Ctrl + F5) in Visual Studio. Double clicking on the exe also works.</b></font>

### Screenshots ###

<img src='http://win32-ray-tracing-demo.googlecode.com/svn/trunk/web/screenshot-1.png'>

<font color='#0068b7'><b>Preset scene 1: checker ground and a ball (there's no light in the scene, however the ground emits light)</b></font>

<img src='http://win32-ray-tracing-demo.googlecode.com/svn/trunk/web/screenshot-2.png'>

<font color='#0068b7'><b>Preset scene 2: smallpt</b></font>

<img src='http://win32-ray-tracing-demo.googlecode.com/svn/trunk/web/screenshot-3.png'>

<font color='#0068b7'><b>Preset scene 3: smallpt (stl ball)</b></font>

<img src='http://win32-ray-tracing-demo.googlecode.com/svn/trunk/web/screenshot-4.png'>

<font color='#0068b7'><b>Preset scene 4: a short and wide tunnel</b></font>

<img src='http://win32-ray-tracing-demo.googlecode.com/svn/trunk/web/screenshot-5.png'>

<font color='#0068b7'><b>Preset scene 5: a long and narrow tunnel</b></font>

<h3>Information for Paper Reviewers</h3>

There is only one table in the paper about the performance of different algorithms, but actually a number of experiments with different parameters have been performed. They are not listed in the paper because there was no space.<br>
<br>
The table in the paper is like this:<br>
<br>
<table><thead><th> <b>Algorithm</b>              </th><th> <b>Preprocess (seconds)</b> </th><th> <b>Render (seconds)</b> </th></thead><tbody>
<tr><td> Linear                   </td><td> 0.000                  </td><td> 70.939             </td></tr>
<tr><td> Regular grid (400x400x5) </td><td> 0.055                  </td><td> 0.197              </td></tr>
<tr><td> Flat grid (400x400x400)  </td><td> 0.892                  </td><td> 0.276              </td></tr>
<tr><td> K-d tree                 </td><td> 0.701                  </td><td> 0.256              </td></tr>
<tr><td> K-d tree (SAH)           </td><td> 7.075                  </td><td> 0.139              </td></tr>
<tr><td> Convex                   </td><td> 0.672                  </td><td> 0.061              </td></tr></tbody></table>

<font color='#0068b7'><b>Table I: The table in the paper</b></font>

These times are obtained when the cross section of the tunnel is tessellated into 150 line segments. As mentioned in the table, the grid is cut into 400 pieces. It may be interesting to know how the size of the cells in the grid can affect the time, and here's the comparison data:<br>
<br>
<table><thead><th> <b>Algorithm</b>              </th><th> <b>Preprocess (seconds)</b> </th><th> <b>Render (seconds)</b> </th></thead><tbody>
<tr><td> Regular grid (50x50x1)   </td><td> 0.013                  </td><td> 1.637              </td></tr>
<tr><td> Regular grid (80x80x2)   </td><td> 0.012                  </td><td> 0.404              </td></tr>
<tr><td> Regular grid (100x100x2) </td><td> 0.018                  </td><td> 0.321              </td></tr>
<tr><td> Regular grid (200x200x3) </td><td> 0.023                  </td><td> 0.382              </td></tr>
<tr><td> Regular grid (400x400x5) </td><td> 0.055                  </td><td> 0.197              </td></tr>
<tr><td> Regular grid (600x600x7) </td><td> 0.108                  </td><td> 0.129              </td></tr>
<tr><td> Regular grid (800x800x9) </td><td> 0.176                  </td><td> 0.131              </td></tr>
<tr><td> Flat grid (50x50x50)     </td><td> 0.026                  </td><td> 0.292              </td></tr>
<tr><td> Flat grid (80x80x80)     </td><td> 0.045                  </td><td> 0.222              </td></tr>
<tr><td> Flat grid (100x100x100)  </td><td> 0.066                  </td><td> 0.238              </td></tr>
<tr><td> Flat grid (200x200x200)  </td><td> 0.180                  </td><td> 0.201              </td></tr>
<tr><td> Flat grid (400x400x400)  </td><td> 0.892                  </td><td> 0.276              </td></tr>
<tr><td> Flat grid (600x600x600)  </td><td> -                      </td><td> -                  </td></tr>
<tr><td> Flat grid (800x800x800)  </td><td> -                      </td><td> -                  </td></tr></tbody></table>

<font color='#0068b7'><b>Table II: Comparison between different grid settings</b></font>

Note that when the size of the flat grid is 600x600x600, the system failed to allocate so much memory (216,000,000 std::vector objects). Thus in the paper, 400 is used as the size of the grid, which is the largest value that supports both regular grid and flat grid.<br>
<br>
There're two parameters related to the construction of the k-d tree which are not discussed in the paper:<br>
<br>
<ul><li>The leaf node object threshold<br>
</li><li>The maximum tree depth</li></ul>

For example, if the number of geometries in a node is <= 8, or the tree depth is > 18, then the node is regarded as a leaf node. This is actually the setting used in the paper. A comparison between different settings is available in the table below:<br>
<br>
<table><thead><th> <b>Object threshold</b> </th><th> <b>Max depth</b> </th><th> <b>Preprocess (seconds)</b> </th><th> <b>Render (seconds)</b> </th><th> <b>Average leaf size</b> </th></thead><tbody>
<tr><td> 4  </td><td> 10  </td><td> 0.211  </td><td> 0.359  </td><td> 47  </td></tr>
<tr><td> 4  </td><td> 11  </td><td> 0.221  </td><td> 0.283  </td><td> 32  </td></tr>
<tr><td> 4  </td><td> 12  </td><td> 0.266  </td><td> 0.293  </td><td> 24  </td></tr>
<tr><td> 4  </td><td> 13  </td><td> 0.334  </td><td> 0.261  </td><td> 14  </td></tr>
<tr><td> 4  </td><td> 14  </td><td> 0.403  </td><td> 0.244  </td><td> 11  </td></tr>
<tr><td> 4  </td><td> 15  </td><td> 0.531  </td><td> 0.277  </td><td> 10  </td></tr>
<tr><td> 4  </td><td> 16  </td><td> 0.660  </td><td> 0.248  </td><td> 6   </td></tr>
<tr><td> 4  </td><td> 17  </td><td> 0.993  </td><td> 0.277  </td><td> 6   </td></tr>
<tr><td> 4  </td><td> 18  </td><td> 1.219  </td><td> 0.287  </td><td> 6   </td></tr>
<tr><td> 4  </td><td> 19  </td><td> 1.905  </td><td> 0.289  </td><td> 5   </td></tr>
<tr><td> 4  </td><td> 20  </td><td> 2.542  </td><td> 0.309  </td><td> 5   </td></tr></tbody></table>

<font color='#0068b7'><b>Table III: Comparison between different k-d tree settings (threshold = 4)</b></font>

<table><thead><th> <b>Object threshold</b> </th><th> <b>Max depth</b> </th><th> <b>Preprocess (seconds)</b> </th><th> <b>Render (seconds)</b> </th><th> <b>Average leaf size</b> </th></thead><tbody>
<tr><td> 4  </td><td> 10  </td><td> 1.794  </td><td> 0.640  </td><td> 161  </td></tr>
<tr><td> 4  </td><td> 11  </td><td> 1.711  </td><td> 0.328  </td><td> 112  </td></tr>
<tr><td> 4  </td><td> 12  </td><td> 2.290  </td><td> 0.211  </td><td> 76   </td></tr>
<tr><td> 4  </td><td> 13  </td><td> 2.872  </td><td> 0.178  </td><td> 52   </td></tr>
<tr><td> 4  </td><td> 14  </td><td> 2.748  </td><td> 0.139  </td><td> 36   </td></tr>
<tr><td> 4  </td><td> 15  </td><td> 3.337  </td><td> 0.123  </td><td> 25   </td></tr>
<tr><td> 4  </td><td> 16  </td><td> 3.988  </td><td> 0.116  </td><td> 18   </td></tr>
<tr><td> 4  </td><td> 17  </td><td> 5.207  </td><td> 0.109  </td><td> 13   </td></tr>
<tr><td> 4  </td><td> 18  </td><td> 6.628  </td><td> 0.123  </td><td> 9    </td></tr>
<tr><td> 4  </td><td> 19  </td><td> 7.577  </td><td> 0.110  </td><td> 7    </td></tr>
<tr><td> 4  </td><td> 20  </td><td> 9.882  </td><td> 0.101  </td><td> 6    </td></tr></tbody></table>

<font color='#0068b7'><b>Table IV: Comparison between different k-d tree (SAH) settings (threshold = 4)</b></font>

It is clear that the average leaf size of the k-d tree with SAH converges more slowly than the simple balanced k-d tree. This is easy to understand because the under SAH strategy more split planes are used to separate objects from empty space. Thus the termination depth should be large when SAH is enabled. A termination depth of 16 is enough for a standard balanced k-d tree. However when SAH is enabled, the depth should be 18 - 20.<br>
<br>
Now we increase the threshold value to 8. The results are listed below.<br>
<br>
<table><thead><th> <b>Object threshold</b> </th><th> <b>Max depth</b> </th><th> <b>Preprocess (seconds)</b> </th><th> <b>Render (seconds)</b> </th><th> <b>Average leaf size</b> </th></thead><tbody>
<tr><td> 8  </td><td> 10  </td><td> 0.203  </td><td> 0.345  </td><td> 47  </td></tr>
<tr><td> 8  </td><td> 11  </td><td> 0.227  </td><td> 0.290  </td><td> 32  </td></tr>
<tr><td> 8  </td><td> 12  </td><td> 0.284  </td><td> 0.307  </td><td> 24  </td></tr>
<tr><td> 8  </td><td> 13  </td><td> 0.322  </td><td> 0.256  </td><td> 14  </td></tr>
<tr><td> 8  </td><td> 14  </td><td> 0.381  </td><td> 0.238  </td><td> 12  </td></tr>
<tr><td> 8  </td><td> 15  </td><td> 0.482  </td><td> 0.263  </td><td> 11  </td></tr>
<tr><td> 8  </td><td> 16  </td><td> 0.637  </td><td> 0.253  </td><td> 8   </td></tr>
<tr><td> 8  </td><td> 17  </td><td> 0.635  </td><td> 0.249  </td><td> 8   </td></tr>
<tr><td> 8  </td><td> 18  </td><td> 0.701  </td><td> 0.256  </td><td> 8   </td></tr>
<tr><td> 8  </td><td> 19  </td><td> 0.828  </td><td> 0.248  </td><td> 7   </td></tr>
<tr><td> 8  </td><td> 20  </td><td> 0.825  </td><td> 0.247  </td><td> 7   </td></tr></tbody></table>

<font color='#0068b7'><b>Table V: Comparison between different k-d tree settings (threshold = 8)</b></font>

<table><thead><th> <b>Object threshold</b> </th><th> <b>Max depth</b> </th><th> <b>Preprocess (seconds)</b> </th><th> <b>Render (seconds)</b> </th><th> <b>Average leaf size</b> </th></thead><tbody>
<tr><td> 8  </td><td> 10  </td><td> 1.786  </td><td> 0.638  </td><td> 163  </td></tr>
<tr><td> 8  </td><td> 11  </td><td> 2.090  </td><td> 0.368  </td><td> 114  </td></tr>
<tr><td> 8  </td><td> 12  </td><td> 2.356  </td><td> 0.229  </td><td> 79   </td></tr>
<tr><td> 8  </td><td> 13  </td><td> 2.548  </td><td> 0.176  </td><td> 57   </td></tr>
<tr><td> 8  </td><td> 14  </td><td> 3.275  </td><td> 0.160  </td><td> 41   </td></tr>
<tr><td> 8  </td><td> 15  </td><td> 3.866  </td><td> 0.153  </td><td> 29   </td></tr>
<tr><td> 8  </td><td> 16  </td><td> 4.687  </td><td> 0.130  </td><td> 20   </td></tr>
<tr><td> 8  </td><td> 17  </td><td> 5.376  </td><td> 0.133  </td><td> 14   </td></tr>
<tr><td> 8  </td><td> 18  </td><td> 7.075  </td><td> 0.139  </td><td> 11   </td></tr>
<tr><td> 8  </td><td> 19  </td><td> 8.232  </td><td> 0.143  </td><td> 8    </td></tr>
<tr><td> 8  </td><td> 20  </td><td> 8.270  </td><td> 0.104  </td><td> 7    </td></tr></tbody></table>

<font color='#0068b7'><b>Table VI: Comparison between different k-d tree (SAH) settings (threshold = 8)</b></font>

As you can see, the best value is still about 16 and 18 - 20. The difference is that when the threshood is increased, unnecessary splittings are avoided. The construction time is greatly decreased with a high termination depth.<br>
<br>
Thus in the paper, parameters (8, 18) are used as a balanced choice.