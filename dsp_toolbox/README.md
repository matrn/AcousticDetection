# DSP toolbox

## ~~matplotplusplus install~~
- `vcpkg install`: [https://vcpkg.io/en/getting-started.html](https://vcpkg.io/en/getting-started.html)
- `vcpkg install matplotplusplus`
~~

## sciplot install
`sudo apt install gnuplot`
```
git clone https://github.com/sciplot/sciplot --recursive
cd sciplot/
mkdir build && cd build
cmake ..
sudo cmake --build . --target install
```