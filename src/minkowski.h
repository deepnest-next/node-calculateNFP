#ifndef MINKOWSKI_H
#define MINKOWSKI_H

#ifdef __cplusplus
extern "C" {
#endif

struct PointXY {
    double x;
    double y;
};

struct PolygonHole {
    struct PointXY* points;
    int num_points;
};

struct PolygonData {
    struct PointXY* points;
    int num_points;
    struct PolygonHole* holes;
    int num_holes;
};

struct NFPResult {
    struct PolygonData* polygons;
    int num_polygons;
};

// Core function for NFP calculation
struct NFPResult* calculate_nfp_raw(
    const struct PointXY* a_points, int a_length,
    const struct PointXY** a_holes, const int* a_hole_lengths, int a_num_holes,
    const struct PointXY* b_points, int b_length,
    const struct PointXY** b_holes, const int* b_hole_lengths, int b_num_holes
);

// Function to free the NFP result
void free_nfp_result(struct NFPResult* result);

#ifdef __cplusplus
}
#endif

#endif // MINKOWSKI_H
