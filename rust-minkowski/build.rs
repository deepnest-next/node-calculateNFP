extern crate cbindgen;

use std::env;
use std::path::PathBuf;

fn main() {
    let crate_dir = env::var("CARGO_MANIFEST_DIR").unwrap();
    let out_dir = PathBuf::from(env::var("OUT_DIR").unwrap());
    let header_path = out_dir.join("rust_minkowski.h");

    cbindgen::Builder::new()
        .with_crate(crate_dir)
        .generate()
        .expect("Unable to generate bindings")
        .write_to_file(header_path);

    // Tell cargo to tell rustc to link the system libraries
    println!("cargo:rustc-link-search=native={}/target/release", crate_dir);
    println!("cargo:rustc-link-lib=static=rust_minkowski");
}
