@echo off
For %%i in (rgrid fgrid kdtree sah convex convex_s) Do (
:For %%i In (linear rgrid fgrid kdtree sah convex convex_s) Do (
    del path-seg-%%i.log
    For %%j In (150 300 450 600 900 1200 1500 2100 3000) Do (
        For /L %%k In (1, 1, 10) Do (
            PerformanceTest 2000 1.5708 150 %%j 1000 %%i >> path-seg-%%i.log
        )
    )
)
