use rust_minkowski::minkowski::{Point, NFPInput, calculate_nfp};

fn main() {
    println!("Calculating No-Fit Polygon (NFP) for complex polygons with holes");

    // Define polygon A - a complex concave shape
    let a_points = vec![
        (0.0, 0.0),
        (100.0, 0.0),
        (100.0, 100.0),
        (70.0, 100.0),
        (70.0, 30.0),   // This creates a concavity
        (30.0, 30.0),
        (30.0, 100.0),
        (0.0, 100.0),
    ];
    
    // Define multiple holes in polygon A
    let a_holes = Some(vec![
        // First hole - circular approximation
        vec![
            (20.0, 60.0), (15.0, 65.0), (10.0, 60.0), (15.0, 55.0)
        ],
        // Second hole - square
        vec![
            (80.0, 80.0), (90.0, 80.0), (90.0, 90.0), (80.0, 90.0)
        ]
    ]);
    
    // Define polygon B - a simple L shape
    let b_points = vec![
        (0.0, 0.0),
        (50.0, 0.0),
        (50.0, 20.0),
        (20.0, 20.0),
        (20.0, 50.0),
        (0.0, 50.0),
    ];
    
    // B has one small hole
    let b_holes = Some(vec![
        vec![
            (10.0, 10.0),
            (15.0, 10.0),
            (15.0, 15.0),
            (10.0, 15.0),
        ]
    ]);
    
    // Calculate NFP
    let input = NFPInput {
        a: a_points.clone(),
        b: b_points.clone(),
        a_holes: a_holes.clone(),
        b_holes: b_holes.clone(),
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
            
            // Print first few points of each hole
            if !hole.is_empty() {
                let hole_points_to_show = std::cmp::min(hole.len(), 3);
                println!("    First {} points:", hole_points_to_show);
                
                for p in 0..hole_points_to_show {
                    println!("    ({:.2}, {:.2})", hole[p].x, hole[p].y);
                }
            }
        }
    }
    
    println!("\nComplex example completed successfully!");
}
