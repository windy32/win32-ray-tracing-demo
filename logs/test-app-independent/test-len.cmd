@echo off
For %%i In (linear rgrid fgrid kdtree sah convex convex_s) Do (
    del len-%%i.log
    For %%j In (100 200 300 400 600 800 1000 1500 2000 3000 4000) Do (
        For /L %%k In (1, 1, 10) Do (
            PerformanceTest %%j 1.5708 150 150 1000 %%i >> len-%%i.log
        )
    )
)
