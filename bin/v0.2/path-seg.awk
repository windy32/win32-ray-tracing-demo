BEGIN {
    seg = 0
}
{
    if (NF == 6) {
        if ($3 != seg) {
            if (seg != 0) {
                printf("%d\t%.1f\t%.1f\t%.1f\n", seg, sum_build / count,
                    sum_preprocess / count, sum_ray_tracing / count);
            }

            sum_build = 0
            sum_preprocess = 0
            sum_ray_tracing = 0
            count = 0
        
            seg = $3
        }

        sum_build += $4
        sum_preprocess += $5
        sum_ray_tracing += $6
        count += 1
    
    }
    
}
END {
    printf("%d\t%.1f\t%.1f\t%.1f\n", seg, sum_build / count,
        sum_preprocess / count, sum_ray_tracing / count);
}
