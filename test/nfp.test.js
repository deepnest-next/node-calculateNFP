const assert = require('assert');
const calculateNFP = require('../').calculateNFP;

// Simple test to verify the module loads and functions correctly
describe('No Fit Polygon Calculation', function() {
  // Allow generous time for the computation
  this.timeout(5000);

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
    
    // Call the native function
    const result = calculateNFP({ A, B });
    
    // Assertions to verify the result
    assert.ok(Array.isArray(result), 'Result should be an array');
    assert.ok(result.length > 0, 'Result should contain at least one polygon');
    
    // Check the first result polygon
    const firstPolygon = result[0];
    assert.ok(Array.isArray(firstPolygon), 'First result should be an array of points');
    assert.ok(firstPolygon.length > 0, 'First polygon should have points');
    
    // Check that the first point has x and y properties
    const firstPoint = firstPolygon[0];
    assert.ok('x' in firstPoint, 'Point should have x coordinate');
    assert.ok('y' in firstPoint, 'Point should have y coordinate');
  });

  it('should calculate NFP with polygon holes', function() {
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
    
    // Call the native function
    const result = calculateNFP({ A, B });
    
    // Assertions to verify the result
    assert.ok(Array.isArray(result), 'Result should be an array');
    assert.ok(result.length > 0, 'Result should contain at least one polygon');
    
    // Check for holes in the result
    const firstPolygon = result[0];
    assert.ok('children' in firstPolygon, 'Result should have children (holes)');
    assert.ok(Array.isArray(firstPolygon.children), 'Children should be an array');
  });

  it('should handle edge cases gracefully', function() {
    // Test with minimal valid inputs
    const A = [
      { x: 0, y: 0 },
      { x: 10, y: 0 },
      { x: 10, y: 10 },
      { x: 0, y: 10 }
    ];
    
    const B = [
      { x: 0, y: 0 },
      { x: 5, y: 0 },
      { x: 5, y: 5 }
    ];
    
    // A triangle and a square should still calculate correctly
    const result = calculateNFP({ A, B });
    assert.ok(Array.isArray(result), 'Should return array even with triangle');
    
    // Verify the result has expected structure even with unusual input
    if (result.length > 0) {
      const firstPolygon = result[0];
      assert.ok(Array.isArray(firstPolygon), 'First result should be an array');
    }
  });

  // Optional: Test with very large polygons or complex holes if performance is a concern
});
