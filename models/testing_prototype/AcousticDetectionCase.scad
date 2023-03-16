//build = false;
build = true;

$fn = build?200:30;

	

base_h = 0.8;
logo_h = 0.2;
box_h = 25;
esp_h = 10;
mics_h = 12;
mics_cap_h = 0.8;
arms_h = 6;
cover_h = 1.2;

module usb_hole(){
	h = esp_h + 3;
	off = 30.5;
    
	translate([0, off, h]) cube([10, 2, 6], center=true);
}

module cable_hole(){
	l = 171.4;
	d = 4;
	off = 26.8;
	off_z = 2.7;
    translate([0, off, off_z]) rotate([0, 90, 0]) cylinder(h=l, d=d, center = true);
}


difference(){
    linear_extrude(base_h) import("2D.dxf", layer="base");
    translate([0,0,-0.02]) linear_extrude(logo_h) mirror([1,0,0]) import("2D.dxf", layer="logo");
    translate([0,0,-0.05]) linear_extrude(base_h+0.1) import("2D.dxf", layer="mics_holes");
}

//cable_hole();

translate([0,0,base_h]){
    difference()
    {
        linear_extrude(box_h) import("2D.dxf", layer="box");
        usb_hole();
		cable_hole();
    }
    
    linear_extrude(esp_h) import("2D.dxf", layer="esp");
    
    difference(){
        linear_extrude(mics_h) import("2D.dxf", layer="mics_box");
        translate([0,0,-0.05]) linear_extrude(mics_h-mics_cap_h+0.05) import("2D.dxf", layer="mics_holes");
        cable_hole();
    }
    
    difference(){
        linear_extrude(arms_h) import("2D.dxf", layer="arms");
        cable_hole();
    }
}

if(build) translate([55, -15, 0]) linear_extrude(cover_h) import("2D.dxf", layer="cover");
else translate([0, 0, base_h+box_h+2]) linear_extrude(cover_h) import("2D.dxf", layer="cover");