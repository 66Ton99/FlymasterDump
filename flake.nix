{
  description = "FlymasterDump - Dump flights from Flymaster GPS SD Vario into IGC files";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };
        qt5 = pkgs.libsForQt5;
      in
      {
        packages.default = pkgs.stdenv.mkDerivation {
          pname = "FlymasterDump";
          version = "1.0.0";
          src = ./.;

          nativeBuildInputs = [
            pkgs.cmake
            qt5.wrapQtAppsHook
          ];

          buildInputs = [
            pkgs.boost
            pkgs.gtest
            qt5.qtbase
          ] ++ pkgs.lib.optionals pkgs.stdenv.isDarwin [
            pkgs.apple-sdk
          ];

          cmakeFlags = [
            "-DCMAKE_BUILD_TYPE=Release"
          ];

          doCheck = true;
          checkPhase = ''
            ctest --output-on-failure
          '';
        };

        devShells.default = pkgs.mkShell {
          nativeBuildInputs = [
            pkgs.cmake
            qt5.wrapQtAppsHook
          ];

          buildInputs = [
            pkgs.boost
            pkgs.gtest
            qt5.qtbase
          ] ++ pkgs.lib.optionals pkgs.stdenv.isDarwin [
            pkgs.apple-sdk
          ];
        };
      }
    );
}
