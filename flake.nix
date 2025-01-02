{
  description = "Crafting Interpreters Challenges, and lox interpreter";

  inputs = {
    # Pointing to the current stable release of nixpkgs. You can
    # customize this to point to an older version or unstable if you
    # like everything shining.
    #
    # E.g.
    #
    # nixpkgs.url = "github:NixOS/nixpkgs/unstable";
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

    utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, ... }@inputs: inputs.utils.lib.eachSystem [
    # Add the system/architecture you would like to support here. Note that not
    # all packages in the official nixpkgs support all platforms.
    "x86_64-linux" "i686-linux" "aarch64-linux" "x86_64-darwin"
  ] (system: let
    pkgs = import nixpkgs {
      inherit system;

      # Add overlays here if you need to override the nixpkgs
      # official packages.
      overlays = [];
      #[
      #  (self: super: {
      #    stdenv = super.llvmPackages_19.libcxxStdenv;
      # })
      #];
      
      # Uncomment this if you need unfree software (e.g. cuda) for
      # your project.
      #
      # config.allowUnfree = true;
    };
  in {
    devShells.default = pkgs.mkShell.override { stdenv = pkgs.llvmPackages_18.stdenv; } rec {
      # Update the name to something that suites your project.
      name = "loxxy-clang-18";


      packages = with pkgs; [
        # Development Tools
        cmake
        cmakeCurses
        ninja
        llvmPackages_18.clang-tools
        llvmPackages_18.bintools
        # Development time dependencies
        gtest
      ];

      buildInputs = with pkgs; [
        
      ];

      # Setting up the environment variables you need during
      # development.
      shellHook = let
        icon = "f121";
      in ''
        export PS1="$(echo -e '\u${icon}') {\[$(tput sgr0)\]\[\033[38;5;228m\]\w\[$(tput sgr0)\]\[\033[38;5;15m\]} (${name}) \\$ \[$(tput sgr0)\]"
      '';
    };
    devShells.clang-19 = pkgs.mkShell.override { stdenv = pkgs.llvmPackages_19.stdenv; } rec {
      # Update the name to something that suites your project.
      name = "loxxy-clang-19";


      packages = with pkgs; [
        # Development Tools
        cmake
        cmakeCurses
        ninja
        llvmPackages_19.clang-tools
        llvmPackages_19.bintools
        # Development time dependencies
        gtest
      ];

      buildInputs = with pkgs; [
        
      ];

      # Setting up the environment variables you need during
      # development.
      shellHook = let
        icon = "f121";
      in ''
        export PS1="$(echo -e '\u${icon}') {\[$(tput sgr0)\]\[\033[38;5;228m\]\w\[$(tput sgr0)\]\[\033[38;5;15m\]} (${name}) \\$ \[$(tput sgr0)\]"
      '';
    };

    devShells.gcc = pkgs.mkShell.override { stdenv = pkgs.gcc14Stdenv; } rec {
      # Update the name to something that suites your project.
      name = "loxxy-gcc";


      packages = with pkgs; [
        # Development Tools
        cmake
        cmakeCurses
        ninja
        llvmPackages_19.clang-tools 
        llvmPackages_19.bintools
        # llvmPackages_18.libcxxClang
        # Development time dependencies
        gtest

        # Build time and Run time dependencies
        abseil-cpp
      ];

      buildInputs = with pkgs; [
        abseil-cpp
      ];

      # Setting up the environment variables you need during
      # development.
      shellHook = let
        icon = "f121";
      in ''
        export PS1="$(echo -e '\u${icon}') {\[$(tput sgr0)\]\[\033[38;5;228m\]\w\[$(tput sgr0)\]\[\033[38;5;15m\]} (${name}) \\$ \[$(tput sgr0)\]"
      '';
    };
    

    packages.default = pkgs.callPackage ./default.nix {};
  });
}
