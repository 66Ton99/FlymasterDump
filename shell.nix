{ pkgs ? import <nixpkgs> {} }:

let
  qt5 = pkgs.libsForQt5;
in
pkgs.mkShell {
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
}
