use std::env;
use std::path::Path;

fn main() {
    let crate_dir = env::var("CARGO_MANIFEST_DIR").unwrap();
    let root_dir = Path::new(&crate_dir).parent().unwrap();
    let src_dir = root_dir.join("src");
    
    // Tell Cargo to re-run this script if the source files change
    println!("cargo:rerun-if-changed={}", src_dir.join("minkowski.cc").display());
    println!("cargo:rerun-if-changed={}", src_dir.join("minkowski.h").display());
    
    // Compile C++ code directly
    let mut build = cc::Build::new();
    
    // Configure the build based on platform
    build.cpp(true)
         .file(src_dir.join("minkowski.cc"))
         .include(src_dir.clone())
         // Include the Boost polygon headers that are already in the repo
         .include(src_dir.join("polygon/include"));
    
    println!("Using Boost headers from: {}", src_dir.join("polygon/include").display());
    
    // Platform specific settings
    if cfg!(target_os = "windows") {
        build.flag("/std:c++17").flag("/EHsc");
        // Add Windows-specific defines
        build.define("NOMINMAX", None);
    } else {
        build.flag("-std=c++17")
             .flag("-fPIC");
    }
    
    // Define USE_RUST instead of USE_NODE_API to avoid including Node.js headers
    build.define("USE_RUST", None);
    
    // Perform the build
    build.compile("minkowski");
    
    println!("cargo:rustc-link-lib=static=minkowski");
    
    // Link against the C++ standard library
    if cfg!(target_os = "linux") {
        println!("cargo:rustc-link-lib=dylib=stdc++");
    } else if cfg!(target_os = "macos") {
        println!("cargo:rustc-link-lib=dylib=c++");
    } else if cfg!(target_os = "windows") {
        // On Windows, the C++ standard library is linked automatically
    }
}
