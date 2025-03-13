use rust_minkowski::minkowski::{Point, NFPInput, calculate_nfp};

pub fn main() {
    println!("Calculating No-Fit Polygon (NFP) for two simple polygons");

    // Define polygon A - a square
    let a_points = vec![
        (0.0, 0.0),
        (100.0, 0.0),
        (100.0, 100.0),
        (0.0, 100.0),
    ];
    
    // Define polygon B - a smaller square
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
        a: a_points.clone(),
        b: b_points.clone(),
        a_holes: a_holes.clone(),
        b_holes: None,
    };
    
    // Calculate NFP
    let result = calculate_nfp(input);
    
    // Print results
    println!("NFP calculation completed!");
    println!("Number of polygons in result: {}", result.polygons.len());
    
    for (i, poly) in result.polygons.iter().enumerate() {
        println!("Polygon {} has {} points", i, poly.len());
        
        // Print first few points of each polygon
        let points_to_show = std::cmp::min(poly.len(), 5);
        println!("First {} points:", points_to_show);
        
        for j in 0..points_to_show {
            println!("  ({:.2}, {:.2})", poly[j].x, poly[j].y);
        }
        
        // Print hole information
        let holes = &result.holes[i];
        println!("  Number of holes: {}", holes.len());
        
        for (h, hole) in holes.iter().enumerate() {
            println!("  Hole {} has {} points", h, hole.len());
        }
    }
    
    println!("\nExample completed successfully!");
}
