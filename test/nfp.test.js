const assert = require('assert');
const calculateNFP = require('../').calculateNFP;

describe('No Fit Polygon Calculation', function() {
  // Allow generous time for complex calculations
  this.timeout(10000);

  it('should calculate NFP between two simple polygons', function() {
    // Create a square polygon A
    const A = [
      { x: 0, y: 0 },
      { x: 100, y: 0 },
      { x: 100, y: 100 },
      { x: 0, y: 100 }
    ];
    
    // Create a smaller square polygon B
    const B = [
      { x: 0, y: 0 },
      { x: 50, y: 0 },
      { x: 50, y: 50 },
      { x: 0, y: 50 }
    ];
    
    const result = calculateNFP({ A, B });
    
    assert.ok(Array.isArray(result), 'Result should be an array');
    assert.ok(result.length > 0, 'Result should contain at least one polygon');
    
    const firstPolygon = result[0];
    assert.ok(Array.isArray(firstPolygon), 'First result should be an array of points');
    assert.ok(firstPolygon.length > 0, 'First polygon should have points');
    
    const firstPoint = firstPolygon[0];
    assert.ok('x' in firstPoint, 'Point should have x coordinate');
    assert.ok('y' in firstPoint, 'Point should have y coordinate');
  });

  it('should calculate NFP with a polygon with a hole', function() {
    // Create a square polygon A with a hole
    const A = [
      { x: 0, y: 0 },
      { x: 100, y: 0 },
      { x: 100, y: 100 },
      { x: 0, y: 100 }
    ];
    
    // Add a square hole in the middle of A
    A.children = [
      [
        { x: 25, y: 25 },
        { x: 75, y: 25 },
        { x: 75, y: 75 },
        { x: 25, y: 75 }
      ]
    ];
    
    // Create a smaller square polygon B
    const B = [
      { x: 0, y: 0 },
      { x: 50, y: 0 },
      { x: 50, y: 50 },
      { x: 0, y: 50 }
    ];
    
    const result = calculateNFP({ A, B });
    
    assert.ok(Array.isArray(result), 'Result should be an array');
    assert.ok(result.length > 0, 'Result should contain at least one polygon');
    
    // Check that the result likely includes a hole effect
    // (The NFP of a square with a hole against a square should have some inner shape)
    let foundChildren = false;
    for (const polygon of result) {
      if (polygon.children && polygon.children.length > 0) {
        foundChildren = true;
        break;
      }
    }
    assert.ok(foundChildren || result.length > 1, 'Result should have either holes or multiple polygons due to the hole in A');
  });

  it('should calculate NFP with both polygons having holes', function() {
    // Create a square polygon A with a hole
    const A = [
      { x: 0, y: 0 },
      { x: 100, y: 0 },
      { x: 100, y: 100 },
      { x: 0, y: 100 }
    ];
    
    // Add a square hole in the middle of A
    A.children = [
      [
        { x: 40, y: 40 },
        { x: 60, y: 40 },
        { x: 60, y: 60 },
        { x: 40, y: 60 }
      ]
    ];
    
    // Create a square polygon B with a small hole
    const B = [
      { x: 0, y: 0 },
      { x: 50, y: 0 },
      { x: 50, y: 50 },
      { x: 0, y: 50 }
    ];
    
    // Add a small hole in B
    B.children = [
      [
        { x: 20, y: 20 },
        { x: 30, y: 20 },
        { x: 30, y: 30 },
        { x: 20, y: 30 }
      ]
    ];
    
    const result = calculateNFP({ A, B });
    
    assert.ok(Array.isArray(result), 'Result should be an array');
    assert.ok(result.length > 0, 'Result should contain at least one polygon');
  });

  it('should calculate NFP with multiple holes', function() {
    // Create a square polygon A with multiple holes
    const A = [
      { x: 0, y: 0 },
      { x: 100, y: 0 },
      { x: 100, y: 100 },
      { x: 0, y: 100 }
    ];
    
    // Add multiple holes in A
    A.children = [
      [
        { x: 20, y: 20 },
        { x: 40, y: 20 },
        { x: 40, y: 40 },
        { x: 20, y: 40 }
      ],
      [
        { x: 60, y: 60 },
        { x: 80, y: 60 },
        { x: 80, y: 80 },
        { x: 60, y: 80 }
      ]
    ];
    
    // Create a simple polygon B
    const B = [
      { x: 0, y: 0 },
      { x: 30, y: 0 },
      { x: 30, y: 30 },
      { x: 0, y: 30 }
    ];
    
    const result = calculateNFP({ A, B });
    
    assert.ok(Array.isArray(result), 'Result should be an array');
    assert.ok(result.length > 0, 'Result should contain at least one polygon');
  });

  it('should handle non-rectangular polygons', function() {
    // Create a triangle polygon A
    const A = [
      { x: 0, y: 0 },
      { x: 100, y: 0 },
      { x: 50, y: 100 }
    ];
    
    // Create a pentagon polygon B
    const B = [
      { x: 0, y: 0 },
      { x: 50, y: -20 },
      { x: 100, y: 0 },
      { x: 80, y: 50 },
      { x: 20, y: 50 }
    ];
    
    const result = calculateNFP({ A, B });
    
    assert.ok(Array.isArray(result), 'Result should be an array');
    assert.ok(result.length > 0, 'Result should contain at least one polygon');
  });

  it('should handle very small polygons', function() {
    // Create a very small square polygon A
    const A = [
      { x: 0, y: 0 },
      { x: 1, y: 0 },
      { x: 1, y: 1 },
      { x: 0, y: 1 }
    ];
    
    // Create another very small square polygon B
    const B = [
      { x: 0, y: 0 },
      { x: 0.5, y: 0 },
      { x: 0.5, y: 0.5 },
      { x: 0, y: 0.5 }
    ];
    
    const result = calculateNFP({ A, B });
    
    assert.ok(Array.isArray(result), 'Result should be an array');
    // Very small polygons might produce empty results in some edge cases
    if (result.length > 0) {
      const firstPolygon = result[0];
      assert.ok(Array.isArray(firstPolygon), 'First result should be an array of points');
    }
  });

  it('should handle very large polygons', function() {
    // Create a large square polygon A
    const A = [
      { x: 0, y: 0 },
      { x: 10000, y: 0 },
      { x: 10000, y: 10000 },
      { x: 0, y: 10000 }
    ];
    
    // Create another large square polygon B
    const B = [
      { x: 0, y: 0 },
      { x: 5000, y: 0 },
      { x: 5000, y: 5000 },
      { x: 0, y: 5000 }
    ];
    
    const result = calculateNFP({ A, B });
    
    assert.ok(Array.isArray(result), 'Result should be an array');
    assert.ok(result.length > 0, 'Result should contain at least one polygon');
  });

  it('should handle concave polygons', function() {
    // Create a concave polygon A
    const A = [
      { x: 0, y: 0 },
      { x: 100, y: 0 },
      { x: 100, y: 100 },
      { x: 50, y: 50 }, // This creates a concavity
      { x: 0, y: 100 }
    ];
    
    // Create a simple polygon B
    const B = [
      { x: 0, y: 0 },
      { x: 30, y: 0 },
      { x: 30, y: 30 },
      { x: 0, y: 30 }
    ];
    
    const result = calculateNFP({ A, B });
    
    assert.ok(Array.isArray(result), 'Result should be an array');
    assert.ok(result.length > 0, 'Result should contain at least one polygon');
  });
});
