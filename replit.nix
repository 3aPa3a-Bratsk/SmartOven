{pkgs}: {
  deps = [
    pkgs.rustc
    pkgs.pkg-config
    pkgs.openssl
    pkgs.libxcrypt
    pkgs.libiconv
    pkgs.cargo
    pkgs.wget
    pkgs.gcc
    pkgs.cmake
    pkgs.git
  ];
}
