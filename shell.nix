{ pkgs ? import <nixpkgs> {} }:
pkgs.mkShell {
  packages = [ 
    (pkgs.esphome.overrideAttrs {
      patches = [ 
        ./esphome-hack-is-connected.diff

        # https://github.com/esphome/esphome/pull/7393
        ./esphome-esp32h2-fix.diff
      ];
      doCheck = false;
    })
  ];
}
