#!/usr/bin/php
<?

/*
Based on code from ...

Barcode Render Class for PHP using the GD graphics library 
Copyright (C) 2001  Karim Mribti
								
   Version  0.0.7a  2001-04-01  
								
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.
																  
This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.
											   
You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
																		 
Copy of GNU Lesser General Public License at: http://www.gnu.org/copyleft/lesser.txt
													 
Source code home page: http://www.mribti.com/barcode/
Contact author at: barcode@mribti.com
*/

$options = array();
for ($i = 0; $i <= count($_SERVER["argv"]) - 1; $i++) 
 {	$optarray = split("=", $_SERVER["argv"][$i]);
	switch ($optarray[0])
	 {	case "barcode":
			$options["barcode"] = $optarray[1];
			break;
		case "tape_date":
			$options["tape_date"] = $optarray[1];
			break;
		case "position":
			$options["position"] = $optarray[1];
			break;
		case "session_number":
			$options["session_number"] = $optarray[1];
			break;
		case "notes":
			$options["notes"] = $optarray[1];
			break;
		case "tape_number":
			$options["tape_number"] = $optarray[1];
			break;
	 }
 }

$codefilename = "/tmp/cvk_bc_{$options["barcode"]}.png";
$outfilename = "/tmp/cvk_tp_{$options["barcode"]}.png";
$font = "/usr/local/kiosk/barcode_generator/FreeSansBold.ttf";
$notesfont = "/usr/local/kiosk/barcode_generator/FreeSerif.ttf";  
  define (__TRACE_ENABLED__, false);
  define (__DEBUG_ENABLED__, false);
  
  require("barcode.php");  
  require("c128bobject.php");
              			   
/*  if (!isset($style))  $style   = BCD_DEFAULT_STYLE;
  if (!isset($width))  $width   = BCD_DEFAULT_WIDTH;
  if (!isset($height)) $height  = BCD_DEFAULT_HEIGHT;
  if (!isset($xres))   $xres    = BCD_DEFAULT_XRES;
  if (!isset($font))   $font    = BCD_DEFAULT_FONT;
*/
	$style = BCD_DEFAULT_STYLE;
	$width = 500;
	$height = 80;
	$xres = 2;

$obj = new C128BObject($width, $height, $style, $options["barcode"]);
   
  if ($obj) {
      $obj->SetFont($font);   
      $obj->DrawObject($xres);
  	  $obj->FlushObject($codefilename);
  	  $obj->DestroyObject();
  	  unset($obj);  /* clean */
  }
#$tpIm = ImageCreateTrueColor(767, 208);
$tpIm = ImageCreateTrueColor(750, 208);

$white = imagecolorallocate($tpIm, 255, 255, 255);
$black = imagecolorallocate($tpIm, 0, 0, 0);

ImageFill($tpIm, 0, 0, $white);

#bool imageline ( resource image, int x1, int y1, int x2, int y2, int color )
ImageLine ($tpIm, 0, 104, 750, 104, $black);
$bcIm = ImageCreateFromPNG($codefilename);

#bool imagecopy ( resource dst_im, resource src_im, int dst_x, int dst_y, int src_x, int src_y, int src_w, int src_h )
#Copy a part of src_im onto dst_im starting at the x,y coordinates src_x, src_y with a width of src_w and a height of src_h. The portion defined will be 
#copied onto the x,y coordinates, dst_x and dst_y.

ImageCopy($tpIm, $bcIm, 0, 108, 0, 0, 500, 80); ImageCopy($tpIm, $bcIm, 0, 0, 0, 0, 500, 80); 

#array imagefttext ( resource image, float size, float angle, int x, int y, int col, string font_file, string text [, array extrainfo] )

ImageTTFtext ($tpIm, 18, 0, 10, 207, $black, $font, $options["barcode"]);

ImageTTFtext ($tpIm, 18, 0, 10, 96, $black, $font, $options["barcode"]);

#$notes = substr($options["notes"], 0, 11) . "\n" . substr($options["notes"], 11);
#array imagettfbbox ( float size, float angle, string fontfile, string text )
/*
0	lower left corner, X position
1	lower left corner, Y position
2	lower right corner, X position
3	lower right corner, Y position
4	upper right corner, X position
5	upper right corner, Y position
6	upper left corner, X position
7	upper left corner, Y position
*/


function fixbbox($bbox)
{
   $xcorr=0-$bbox[6]; //northwest X
   $ycorr=0-$bbox[7]; //northwest Y
   $tmp_bbox['left']=$bbox[6]+$xcorr;
   $tmp_bbox['top']=$bbox[7]+$ycorr;
   $tmp_bbox['width']=$bbox[2]+$xcorr;
   $tmp_bbox['height']=$bbox[3]+$ycorr;
  
   return $tmp_bbox;
}

$chopped = FALSE;
$notes = $options["notes"];
#print ($notes . "\n");
$newlinecount = 0;
for ($x = 0; $x <= strlen($notes) - 1; $x++)
 {	$box = fixbbox(ImageTTFBBox(16, 0, $notesfont, $notestack . $notes[$x]));
	#print ($notes[$x] . " " . $notestack . " " . $box["width"] . "\n");
	if ($box[width] > 240 && $box[height] >= 54)
	 {	$labelnotes = substr($notestack, 0, -1) . ">";
		$chopped = TRUE;
		break;
	 } elseif ($box[width] > 240)
	 {	$notestack .= "\n" . $notes[$x];
		$newlinecount++;
	 } else
	 {	$notestack .= $notes[$x];
	 }
 }
if ($chopped == FALSE)
 {	$labelnotes = $notestack;
 }

ImageTTFtext ($tpIm, 18, 0, 500, 29, $black, $font, "{$options["tape_date"]}  S:{$options["session_number"]} P:{$options["position"]} T:{$options["tape_number"]}");

ImageTTFText($tpIm, 16, 0, 500, 52, $black, $notesfont, $labelnotes);

ImageTTFtext ($tpIm, 18, 0, 500, 138, $black, $font, "{$options["tape_date"]}  S:{$options["session_number"]} P:{$options["position"]} T:{$options["tape_number"]}");

ImageTTFtext ($tpIm, 16, 0, 500, 160, $black, $notesfont, $labelnotes);

ImagePNG($tpIm, $outfilename);
ImageDestroy($tpIm);
ImageDestroy($bcIm);
?>
