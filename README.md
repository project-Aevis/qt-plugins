# Aevis Qt Plugins

Qt 6 platform and widget theme integration plugins to ensure native look-and-feel for Qt applications across the Aevis Desktop Environment.

Part of the **[Project Aevis](https://github.com/project-Aevis)** suite.

## Features

- Integrates system colors, fonts, icons, and metrics into Qt 6 applications
- Native SVG rendering with `librsvg` and `libqtxdg`
- Seamless switching between light and dark visual themes

## Dependencies

### Debian / Ubuntu

```shell
sudo apt install cmake extra-cmake-modules qt6-base-dev qt6-tools-dev libqtxdg-dev librsvg2-dev
```

### Arch Linux

```shell
sudo pacman -S gcc extra-cmake-modules qt6-base qt6-tools libqtxdg librsvg
```

## Build and Install

```shell
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr ..
make
sudo make install
```

## License

This project is licensed under the GNU General Public License v3.0 (GPLv3).
