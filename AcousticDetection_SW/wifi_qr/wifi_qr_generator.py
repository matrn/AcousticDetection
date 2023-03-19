import sys
sys.path.insert(0,'..')
from config_parser import parse_config

# pip install qrcode
import qrcode
import qrcode.image.svg


config = parse_config()
AP_SSID = config['wifi_ap_ssid']
AP_PASS = config['wifi_ap_pass']
print(f'SSID: {AP_SSID}, PASS: {AP_PASS}')

def build_wifi_qr_data(ssid: str, password: str, enc_method: str = 'WPA', hidden: str = 'false'):
	'''
		source: https://en.wikipedia.org/wiki/QR_code#Joining_a_Wi%E2%80%91Fi_network
		WIFI:S:<SSID>;T:<WEP|WPA|blank>;P:<PASSWORD>;H:<true|false|blank>;;
	'''
	return f'WIFI:S:{ssid};T:{enc_method};P:{password};H:{hidden};;'

def build_url_data(url: str):
	return f'urlto: {url}'


def qr_gen(filename: str, data: str):
	qr = qrcode.QRCode(
		box_size=150,
		border=1,
		#image_factory=qrcode.image.svg.SvgPathImage
	)
	qr.add_data(data)
	qr.make(fit=True)
	img = qr.make_image(fill_color="black", back_color="white")
	type(img)  # qrcode.image.pil.PilImage
	img.save(filename)

qr_gen("wifi_qr.png", build_wifi_qr_data(AP_SSID, AP_PASS))
qr_gen("url_qr.png", build_url_data("http://192.168.1.1/"))



# pip install svgwrite
import svgwrite
from svgwrite import cm, mm

QR_SIZE_SMALL = (40*mm,)*2
QR_SIZE = (45*mm,)*2
QR_SIZE_BIGGER = (60*mm,)*2
QR_SIZE_BIGGER_2 = (80*mm,)*2


dwg = svgwrite.Drawing('qr_pdf.svg',size=('210mm','297mm'))

dwg.add(dwg.image('wifi_qr.png', insert=(2*cm, 2*cm), size=QR_SIZE))
dwg.add(dwg.image('wifi_qr.png', insert=(8*cm, 2*cm), size=QR_SIZE))
dwg.add(dwg.image('wifi_qr.png', insert=(14*cm, 2*cm), size=QR_SIZE_SMALL))

dwg.add(dwg.image('wifi_qr.png', insert=(2*cm, 8*cm), size=QR_SIZE_BIGGER))
# dwg.add(dwg.image('wifi_qr.png', insert=(10*cm, 8*cm), size=QR_SIZE_BIGGER_2))

off = 14
dwg.add(dwg.image('wifi_qr.png', insert=(2*cm, (off+2)*cm), size=QR_SIZE))
dwg.add(dwg.image('wifi_qr.png', insert=(8*cm, (off+2)*cm), size=QR_SIZE))
dwg.add(dwg.image('wifi_qr.png', insert=(14*cm, (off+2)*cm), size=QR_SIZE_SMALL))

dwg.add(dwg.image('wifi_qr.png', insert=(2*cm, (off+8)*cm), size=QR_SIZE_BIGGER))
# dwg.add(dwg.image('wifi_qr.png', insert=(10*cm, (off+8)*cm), size=QR_SIZE_BIGGER_2))

# dwg.add(dwg.line((2*cm, 2*cm), (10*cm, 2*cm), stroke=svgwrite.rgb(10, 10, 16, '%')))
dwg.save()