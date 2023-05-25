# ESP32 Web page source code
Script `bundle_all.sh` is automatically called by PlatformIO during FS build.

## Scripts
- python script `bundle.py` takes all files from `src/` dir, if file js Javascript it runs minification and compresses all to the gzip
- bash script `bundle_all.sh` runs `PurgeCSS` to clean unused components from Bootstrap template and then `bundle.py`