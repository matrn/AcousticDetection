//build = false;
build = true;

$fn = build?200:30;

include <threadlib/threadlib.scad>
UNC_THREAD_SCALE = 1.08;
thread_end_wall_thickness = 0.8;

base_h = 1.6;
logo_h = 0.2;
text_h = 0.2;   //0.4
box_h = 6.25;
arms_h = 4;

solder_pads_h = 1;   // and LEDs

cover_h = 1.2;
cover_pads_h = 4.2;

mic_pos = [100, 19];
mic_hole_r1 = 1;
mic_hole_r2 = 3.6;

cover_holes_h = 1;
cover_holes_r1 = 1;
cover_holes_r2 = 3.9/2;
cover_holes_pos = [
	[88, 19],
	[-88, 19],
	[33, 19],
	[-33, 19],
	[6.5, 21],
	[-6.5, 21],
	[17, -25.5],
	[-17, -25.5]
];


module mic_hole(second=false){
	mp = second?-1:1;
	translate([mp*mic_pos[0], mic_pos[1], base_h/2]) cylinder(base_h, mic_hole_r2, mic_hole_r1, center=true); 
}

module usb_hole(){
	// from bottom 5.5 mm -> from center 16,5 mm
	// from base 3.5
	h = 3.5;
	off = -16.5;
    
	translate([36/2+2/2+1-0.8, off, h]) cube([2, 12, 7], center=true);
}

module box(){
	difference(){
		linear_extrude(box_h) import("2D.dxf", layer="box");
		linear_extrude(box_h+0.1) import("2D.dxf", layer="padded_device");
	}
}

module rotated_tripod_thread(){
	difference(){
		// Most consumer cameras are fitted with 1/4-20 UNC threads (https://en.wikipedia.org/wiki/Tripod_(photography))
		// 20-UN-1/4 = UNC-1/4
		ss = UNC_THREAD_SCALE;
		rotate([90, 0, 0]) scale([ss, ss, ss]) tap("20-UN-1/4", turns=5);
		
		translate([0, 4/2-thread_end_wall_thickness, 0]) cube([10, 4, 10], center = true);
	}
	
}

module box_base(){
difference(){
	union(){	
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
			linear_extrude(box_h-0.2) import("2D.dxf", layer="mount");
		}
	}

	translate([0, -22.2, (base_h+box_h)/2]) rotated_tripod_thread();
	mic_hole();
	mic_hole(true);
}
}

module box_cover(){
difference(){
	union(){
		linear_extrude(cover_h) import("2D.dxf", layer="cover");
		translate([0,0,-cover_pads_h]) linear_extrude(cover_pads_h) import("2D.dxf", layer="cover_pads");
	}
	
	for(pos = cover_holes_pos){
		translate(pos) translate([0,0,cover_h-cover_holes_h/2]) cylinder(cover_holes_h, cover_holes_r1, cover_holes_r2, center=true);
	}
}
}

box_base();
//box_cover();

if(build) translate([0, 55, cover_h]) rotate([0,180,180]) box_cover();
else translate([0, 0, base_h+box_h+2]) box_cover();