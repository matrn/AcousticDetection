//build = false;
build = true;

$fn = build?200:30;



base_h = 1.6;
logo_h = 0.2;
text_h = 0.4;
box_h = 6.25;
arms_h = 4;

solder_pads_h = 1;   // and LEDs

cover_h = 1.2;

module usb_hole(){
	// from bottom 5.5 mm -> from center 16,5 mm
	// from base 3.5
	h = 3.5;
	off = -16.5;
    
	translate([36/2+2/2+1, off, h]) cube([2, 12, 7], center=true);
}

module box(){
	difference(){
		linear_extrude(box_h) import("2D.dxf", layer="box");
		linear_extrude(box_h+0.1) import("2D.dxf", layer="padded_device");
	}
}



difference(){
    linear_extrude(base_h) import("2D.dxf", layer="box");
	translate([0,0,base_h-solder_pads_h]) linear_extrude(solder_pads_h) import("2D.dxf", layer="solder_pads");
	translate([0,0,-0.02]) linear_extrude(logo_h) mirror([1,0,0]) import("2D.dxf", layer="logo");
	translate([0,0,-0.02]) linear_extrude(text_h) mirror([1,0,0]) import("2D.dxf", layer="text");    
}

//usb_hole();

translate([0,0,base_h]){
    difference(){
		box();
        usb_hole();
    }    
	linear_extrude(box_h) import("2D.dxf", layer="mount");
}


if(build) translate([0, 55, 0]) rotate([0,0,180]) linear_extrude(cover_h) import("2D.dxf", layer="cover");
else translate([0, 0, base_h+box_h+2]) linear_extrude(cover_h) import("2D.dxf", layer="cover");