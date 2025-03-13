use std::ffi::{c_void, c_double, c_int};
use std::slice;
use libc::malloc;
use crate::minkowski::{Point, NFPInput, NFPResult, calculate_nfp};

#[repr(C)]
pub struct CPoint {
    pub x: c_double,
    pub y: c_double,
}

#[repr(C)]
pub struct CPolygonHole {
    pub points: *mut CPoint,
    pub num_points: c_int,
}

#[repr(C)]
pub struct CPolygon {
    pub points: *mut CPoint,
    pub num_points: c_int,
    pub holes: *mut CPolygonHole,
    pub num_holes: c_int,
}

#[repr(C)]
pub struct CNFPResult {
    pub polygons: *mut CPolygon,
    pub num_polygons: c_int,
}

/// Export our Rust wrapper function to be callable from C/C++
///
/// This provides a way for other non-Rust applications to use our Rust wrapper
/// instead of directly calling the C++ implementation
#[no_mangle]
pub extern "C" fn rust_calculate_nfp(
    a_points: *const CPoint,
    a_length: c_int,
    a_holes: *const *const CPoint,
    a_hole_lengths: *const c_int,
    a_num_holes: c_int,
    b_points: *const CPoint,
    b_length: c_int,
    b_holes: *const *const CPoint,
    b_hole_lengths: *const c_int,
    b_num_holes: c_int,
) -> *mut CNFPResult {
    // Convert A points to Rust format
    let a_slice = unsafe { slice::from_raw_parts(a_points, a_length as usize) };
    let a_vec: Vec<(f64, f64)> = a_slice
        .iter()
        .map(|p| (p.x, p.y))
        .collect();

    // Convert B points to Rust format
    let b_slice = unsafe { slice::from_raw_parts(b_points, b_length as usize) };
    let b_vec: Vec<(f64, f64)> = b_slice
        .iter()
        .map(|p| (p.x, p.y))
        .collect();

    // Convert A holes if any
    let mut a_holes_vec = None;
    if a_num_holes > 0 && !a_holes.is_null() && !a_hole_lengths.is_null() {
        let hole_lengths = unsafe { slice::from_raw_parts(a_hole_lengths, a_num_holes as usize) };
        let mut holes = Vec::with_capacity(a_num_holes as usize);

        for i in 0..a_num_holes as usize {
            let hole_points = unsafe { slice::from_raw_parts(*a_holes.add(i), hole_lengths[i] as usize) };
            holes.push(hole_points.iter().map(|p| (p.x, p.y)).collect());
        }

        a_holes_vec = Some(holes);
    }

    // Convert B holes if any
    let mut b_holes_vec = None;
    if b_num_holes > 0 && !b_holes.is_null() && !b_hole_lengths.is_null() {
        let hole_lengths = unsafe { slice::from_raw_parts(b_hole_lengths, b_num_holes as usize) };
        let mut holes = Vec::with_capacity(b_num_holes as usize);

        for i in 0..b_num_holes as usize {
            let hole_points = unsafe { slice::from_raw_parts(*b_holes.add(i), hole_lengths[i] as usize) };
            holes.push(hole_points.iter().map(|p| (p.x, p.y)).collect());
        }

        b_holes_vec = Some(holes);
    }

    // Create input for NFP calculation
    let input = NFPInput {
        a: a_vec,
        b: b_vec,
        a_holes: a_holes_vec,
        b_holes: b_holes_vec,
    };

    // Calculate NFP using our wrapper over the C++ implementation
    let result = calculate_nfp(input);

    // Convert results back to C format
    let result_ptr = unsafe {
        let num_polygons = result.polygons.len();
        let ptr = malloc(std::mem::size_of::<CNFPResult>()) as *mut CNFPResult;
        let polygons_ptr = malloc(std::mem::size_of::<CPolygon>() * num_polygons) as *mut CPolygon;
        
        (*ptr).polygons = polygons_ptr;
        (*ptr).num_polygons = num_polygons as c_int;

        // Process each polygon
        for i in 0..num_polygons {
            let polygon = &result.polygons[i];
            let num_points = polygon.len();
            let points_ptr = malloc(std::mem::size_of::<CPoint>() * num_points) as *mut CPoint;
            
            // Copy points
            for (j, point) in polygon.iter().enumerate() {
                (*points_ptr.add(j)).x = point.x;
                (*points_ptr.add(j)).y = point.y;
            }
            
            // Process holes
            let holes = &result.holes[i];
            let num_holes = holes.len();
            let mut holes_ptr = std::ptr::null_mut();
            
            if num_holes > 0 {
                holes_ptr = malloc(std::mem::size_of::<CPolygonHole>() * num_holes) as *mut CPolygonHole;
                
                for (h, hole) in holes.iter().enumerate() {
                    let num_hole_points = hole.len();
                    let hole_points_ptr = malloc(std::mem::size_of::<CPoint>() * num_hole_points) as *mut CPoint;
                    
                    // Copy hole points
                    for (p, point) in hole.iter().enumerate() {
                        (*hole_points_ptr.add(p)).x = point.x;
                        (*hole_points_ptr.add(p)).y = point.y;
                    }
                    
                    (*holes_ptr.add(h)).points = hole_points_ptr;
                    (*holes_ptr.add(h)).num_points = num_hole_points as c_int;
                }
            }
            
            (*polygons_ptr.add(i)).points = points_ptr;
            (*polygons_ptr.add(i)).num_points = num_points as c_int;
            (*polygons_ptr.add(i)).holes = holes_ptr;
            (*polygons_ptr.add(i)).num_holes = num_holes as c_int;
        }
        
        ptr
    };

    result_ptr
}

#[no_mangle]
pub extern "C" fn rust_free_nfp_result(result: *mut CNFPResult) {
    unsafe {
        if result.is_null() {
            return;
        }
        
        let result_ref = &*result;
        
        for i in 0..result_ref.num_polygons as usize {
            let polygon = &*result_ref.polygons.add(i);
            
            // Free each hole's points
            for h in 0..polygon.num_holes as usize {
                let hole = &*polygon.holes.add(h);
                libc::free(hole.points as *mut c_void);
            }
            
            // Free the holes array
            if polygon.num_holes > 0 {
                libc::free(polygon.holes as *mut c_void);
            }
            
            // Free the polygon's points
            libc::free(polygon.points as *mut c_void);
        }
        
        // Free the polygons array
        libc::free(result_ref.polygons as *mut c_void);
        
        // Free the result itself
        libc::free(result as *mut c_void);
    }
}
