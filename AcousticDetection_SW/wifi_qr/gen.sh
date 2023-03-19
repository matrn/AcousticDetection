#!/bin/bash

echo "> Generating QR code & SVG template"
python3 wifi_qr_generator.py
echo "> Done!"
echo ""

# sudo apt install wkhtmltopdf
#wkhtmltopdf --enable-local-file-access qr_pdf.html qr.pdf

# sudo apt-get install librsvg2-bin
echo "> Generating PDF"
rsvg-convert -f pdf -o qr.pdf qr_pdf.svg
echo "> Done!"