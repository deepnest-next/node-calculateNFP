use std::ffi::{c_double, c_int};
use std::slice;
use std::ptr;

/// A point with x and y coordinates
#[derive(Debug, Clone, Copy)]
pub struct Point {
    pub x: f64,
    pub y: f64,
}

/// A polygon represented as a list of points
#[derive(Debug, Clone)]
pub struct Polygon {
    pub points: Vec<Point>,
    pub holes: Vec<Vec<Point>>,
}

/// Input data for NFP calculation
#[derive(Debug)]
pub struct NFPInput {
    pub a: Vec<(f64, f64)>,
    pub b: Vec<(f64, f64)>,
    pub a_holes: Option<Vec<Vec<(f64, f64)>>>,
    pub b_holes: Option<Vec<Vec<(f64, f64)>>>,
}

/// Output from NFP calculation
#[derive(Debug)]
pub struct NFPResult {
    pub polygons: Vec<Vec<Point>>,
    pub holes: Vec<Vec<Vec<Point>>>,
}

// FFI structures that match the C++ definitions
#[repr(C)]
struct CPointXY {
    x: c_double,
    y: c_double,
}

#[repr(C)]
struct CPolygonHole {
    points: *mut CPointXY,
    num_points: c_int,
}

#[repr(C)]
struct CPolygonData {
    points: *mut CPointXY,
    num_points: c_int,
    holes: *mut CPolygonHole,
    num_holes: c_int,
}

#[repr(C)]
struct CNFPResult {
    polygons: *mut CPolygonData,
    num_polygons: c_int,
}

// FFI functions from our C++ library
extern "C" {
    fn calculate_nfp_raw(
        a_points: *const CPointXY, a_length: c_int,
        a_holes: *const *const CPointXY, a_hole_lengths: *const c_int, a_num_holes: c_int,
        b_points: *const CPointXY, b_length: c_int,
        b_holes: *const *const CPointXY, b_hole_lengths: *const c_int, b_num_holes: c_int,
    ) -> *mut CNFPResult;
    
    fn free_nfp_result(result: *mut CNFPResult);
}

/// Calculate the No-Fit Polygon (NFP) for two polygons
/// 
/// The NFP represents all positions where the reference point of polygon B 
/// can be placed such that A and B do not overlap but touch externally
pub fn calculate_nfp(input: NFPInput) -> NFPResult {
    // Convert A points to C format
    let a_points: Vec<CPointXY> = input.a.iter()
        .map(|&(x, y)| CPointXY { x, y })
        .collect();
    
    // Convert B points to C format
    let b_points: Vec<CPointXY> = input.b.iter()
        .map(|&(x, y)| CPointXY { x, y })
        .collect();
    
    // Prepare A holes
    let mut a_hole_points: Vec<Vec<CPointXY>> = Vec::new();
    let mut a_holes_ptrs: Vec<*const CPointXY> = Vec::new();
    let mut a_hole_lengths: Vec<c_int> = Vec::new();
    
    if let Some(holes) = &input.a_holes {
        for hole in holes {
            let hole_points: Vec<CPointXY> = hole.iter()
                .map(|&(x, y)| CPointXY { x, y })
                .collect();
            
            a_hole_lengths.push(hole_points.len() as c_int);
            a_holes_ptrs.push(hole_points.as_ptr());
            a_hole_points.push(hole_points);
        }
    }
    
    // Prepare B holes
    let mut b_hole_points: Vec<Vec<CPointXY>> = Vec::new();
    let mut b_holes_ptrs: Vec<*const CPointXY> = Vec::new();
    let mut b_hole_lengths: Vec<c_int> = Vec::new();
    
    if let Some(holes) = &input.b_holes {
        for hole in holes {
            let hole_points: Vec<CPointXY> = hole.iter()
                .map(|&(x, y)| CPointXY { x, y })
                .collect();
            
            b_hole_lengths.push(hole_points.len() as c_int);
            b_holes_ptrs.push(hole_points.as_ptr());
            b_hole_points.push(hole_points);
        }
    }
    
    // Call the C++ function
    let result_ptr = unsafe {
        calculate_nfp_raw(
            a_points.as_ptr(), a_points.len() as c_int,
            if a_holes_ptrs.is_empty() { ptr::null() } else { a_holes_ptrs.as_ptr() },
            if a_hole_lengths.is_empty() { ptr::null() } else { a_hole_lengths.as_ptr() },
            a_hole_lengths.len() as c_int,
            b_points.as_ptr(), b_points.len() as c_int,
            if b_holes_ptrs.is_empty() { ptr::null() } else { b_holes_ptrs.as_ptr() },
            if b_hole_lengths.is_empty() { ptr::null() } else { b_hole_lengths.as_ptr() },
            b_hole_lengths.len() as c_int,
        )
    };
    
    // Convert C result to Rust format
    let mut polygons = Vec::new();
    let mut holes = Vec::new();
    
    unsafe {
        if !result_ptr.is_null() {
            let c_result = &*result_ptr;
            
            // Process each polygon
            for i in 0..c_result.num_polygons as usize {
                let c_polygon = &*c_result.polygons.add(i);
                
                // Convert polygon points
                let points_slice = slice::from_raw_parts(c_polygon.points, c_polygon.num_points as usize);
                let polygon_points = points_slice.iter()
                    .map(|p| Point { x: p.x, y: p.y })
                    .collect();
                
                polygons.push(polygon_points);
                
                // Convert polygon holes
                let mut polygon_holes = Vec::new();
                
                for h in 0..c_polygon.num_holes as usize {
                    let c_hole = &*c_polygon.holes.add(h);
                    let hole_points_slice = slice::from_raw_parts(c_hole.points, c_hole.num_points as usize);
                    let hole_points = hole_points_slice.iter()
                        .map(|p| Point { x: p.x, y: p.y })
                        .collect();
                    
                    polygon_holes.push(hole_points);
                }
                
                holes.push(polygon_holes);
            }
            
            // Free the C result
            free_nfp_result(result_ptr);
        }
    }
    
    NFPResult { polygons, holes }
}

// Convenience function to create a polygon from a vector of points
pub fn create_polygon(points: Vec<(f64, f64)>, holes: Option<Vec<Vec<(f64, f64)>>>) -> Polygon {
    let points = points.into_iter()
        .map(|(x, y)| Point { x, y })
        .collect();
    
    let holes = match holes {
        Some(holes) => holes.into_iter()
            .map(|hole| hole.into_iter()
                 .map(|(x, y)| Point { x, y })
                 .collect())
            .collect(),
        None => Vec::new(),
    };
    
    Polygon { points, holes }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_nfp_calculation() {
        // Define polygon A
        let a_points = vec![
            (0.0, 0.0),
            (100.0, 0.0),
            (100.0, 100.0),
            (0.0, 100.0),
        ];
        
        // Define polygon B
        let b_points = vec![
            (0.0, 0.0),
            (50.0, 0.0),
            (50.0, 50.0),
            (0.0, 50.0),
        ];
        
        // Define a hole in polygon A
        let a_holes = Some(vec![
            vec![
                (25.0, 25.0),
                (75.0, 25.0),
                (75.0, 75.0),
                (25.0, 75.0),
            ]
        ]);
        
        let input = NFPInput {
            a: a_points,
            b: b_points,
            a_holes,
            b_holes: None,
        };
        
        let result = calculate_nfp(input);
        
        // Basic validation of result
        assert!(!result.polygons.is_empty(), "NFP calculation should return at least one polygon");
        
        // Check that the first polygon has points
        assert!(!result.polygons[0].is_empty(), "First polygon should have points");
    }
}
