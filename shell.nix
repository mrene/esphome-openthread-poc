{ pkgs ? import <nixpkgs> {} }:
pkgs.mkShell {
  packages = [ 
    (pkgs.esphome.overrideAttrs {
      patches = [ ./esphome-hack-is-connected.diff ];
    })
  ];
}
