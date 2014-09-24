@echo off
For %%i In (linear rgrid fgrid kdtree sah convex convex_s) Do (
    del seg-%%i.log
    For %%j In (30 45 60 75 90 120 150 200 250 300) Do (
        For /L %%k In (1, 1, 10) Do (
            PerformanceTest 2000 1.5708 %%j %%j 1000 %%i >> seg-%%i.log
        )
    )
)
