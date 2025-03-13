# Rust Minkowski Examples

This directory contains examples of how to use the Rust Minkowski library for NFP (No-Fit Polygon) calculations.

## Running Examples

To run an example, use the following command from the project root:

```bash
cargo run --example basic_nfp
# or
cargo run --example complex_nfp
```

## Available Examples

### basic_nfp.rs

A basic example that demonstrates how to:
1. Define two polygons (with and without holes)
2. Calculate the NFP between them
3. Process the results

### complex_nfp.rs

A more advanced example showing:
1. Complex concave polygons with multiple holes
2. Processing more complex NFP results
3. Handling multiple polygons and holes in the result

These examples show the fundamental usage of the library for simple and complex polygon nesting scenarios.
